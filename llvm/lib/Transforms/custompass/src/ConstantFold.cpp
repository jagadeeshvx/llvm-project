#include "ConstantFold.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

// Tries to compute the result of applying BO's opcode to the two
// constant operands A and B ourselves. Writes the answer to Result and
// returns true if we know how to handle this opcode (and, for
// division/remainder, the divisor isn't zero). Returns false if we
// don't recognize the opcode, or the computation is undefined - in
// which case we leave the instruction alone rather than guess.
static bool computeConstantResult(const BinaryOperator *BO, const APInt &A,
                                   const APInt &B, APInt &Result) {
  switch (BO->getOpcode()) {
  case Instruction::Add:
    Result = A + B;
    return true;
  case Instruction::Sub:
    Result = A - B;
    return true;
  case Instruction::Mul:
    Result = A * B;
    return true;
  case Instruction::And:
    Result = A & B;
    return true;
  case Instruction::Or:
    Result = A | B;
    return true;
  case Instruction::Xor:
    Result = A ^ B;
    return true;
  case Instruction::Shl:
    // Shifting by more bits than the type has is undefined; skip it.
    if (B.uge(A.getBitWidth()))
      return false;
    Result = A.shl(B);
    return true;
  case Instruction::LShr:
    if (B.uge(A.getBitWidth()))
      return false;
    Result = A.lshr(B);
    return true;
  case Instruction::AShr:
    if (B.uge(A.getBitWidth()))
      return false;
    Result = A.ashr(B);
    return true;
  case Instruction::UDiv:
    if (B.isZero()) // division by zero: undefined, don't fold
      return false;
    Result = A.udiv(B);
    return true;
  case Instruction::SDiv:
    if (B.isZero())
      return false;
    Result = A.sdiv(B);
    return true;
  case Instruction::URem:
    if (B.isZero())
      return false;
    Result = A.urem(B);
    return true;
  case Instruction::SRem:
    if (B.isZero())
      return false;
    Result = A.srem(B);
    return true;
  default:
    // An opcode we don't handle (float math, pointer math, etc.) -
    // leave it for a human, or a more complete pass, to deal with.
    return false;
  }
}

bool runConstantFolding(Function &F, unsigned &NumFolded) {
  bool Changed = false;

  for (BasicBlock &BB : F) {
    for (auto It = BB.begin(), End = BB.end(); It != End;) {
      Instruction *I = &*It++;

      auto *BO = dyn_cast<BinaryOperator>(I);
      if (!BO)
        continue;

      // Both operands must be plain integer constants for us to be
      // able to compute the answer right now, at compile time.
      auto *LHS = dyn_cast<ConstantInt>(BO->getOperand(0));
      auto *RHS = dyn_cast<ConstantInt>(BO->getOperand(1));
      if (!LHS || !RHS)
        continue;

      APInt Result;
      if (!computeConstantResult(BO, LHS->getValue(), RHS->getValue(),
                                  Result))
        continue;

      // Turn our computed APInt number back into an LLVM constant of
      // the same type, and swap it in for the instruction.
      Constant *NewConst = ConstantInt::get(BO->getType(), Result);
      BO->replaceAllUsesWith(NewConst);
      BO->eraseFromParent();
      ++NumFolded;
      Changed = true;
    }
  }

  return Changed;
}
