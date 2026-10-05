#include "StrengthReduction.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

// Checks whether V is a constant integer with exactly one bit set
// (4, 8, 16, ... in binary that's 100, 1000, 10000, ...), which makes
// it a power of two. If so, writes the position of that bit into
// ShiftAmount - that position IS the amount you'd shift by, since
// 2^ShiftAmount == the value. We find this ourselves by walking every
// bit rather than calling APInt::isPowerOf2()/logBase2().
//
// Limited to values that fit in 64 bits, which covers every ordinary
// integer type (i8/i16/i32/i64); wider types are skipped.
static bool isPowerOfTwoConstant(Value *V, unsigned &ShiftAmount) {
  auto *Const = dyn_cast<ConstantInt>(V);
  if (!Const)
    return false;

  if (Const->getValue().getBitWidth() > 64)
    return false; // outside the range we handle; skip rather than guess

  uint64_t RawValue = Const->getValue().getZExtValue();
  if (RawValue == 0)
    return false; // zero has no bits set - not a power of two

  unsigned SetBitCount = 0;
  unsigned PositionOfSetBit = 0;
  for (unsigned BitPosition = 0; BitPosition < 64; ++BitPosition) {
    if (RawValue & (1ULL << BitPosition)) {
      ++SetBitCount;
      PositionOfSetBit = BitPosition;
    }
  }

  if (SetBitCount != 1)
    return false; // more than one bit set - not a power of two

  ShiftAmount = PositionOfSetBit;
  return true;
}

bool runStrengthReduction(Function &F, unsigned &NumReduced) {
  bool Changed = false;

  for (BasicBlock &BB : F) {
    for (auto It = BB.begin(), End = BB.end(); It != End;) {
      Instruction *I = &*It++;

      auto *BinOp = dyn_cast<BinaryOperator>(I);
      if (!BinOp || !BinOp->getType()->isIntegerTy())
        continue;

      unsigned Shift = 0;

      if (BinOp->getOpcode() == Instruction::Mul) {
        // Multiplication is commutative: the power-of-two constant
        // could be on either side ("x * 8" or "8 * x").
        Value *LHS = BinOp->getOperand(0);
        Value *RHS = BinOp->getOperand(1);

        Value *Variable = nullptr;
        if (isPowerOfTwoConstant(RHS, Shift))
          Variable = LHS;
        else if (isPowerOfTwoConstant(LHS, Shift))
          Variable = RHS;

        if (Variable) {
          Constant *ShiftConst = ConstantInt::get(Variable->getType(), Shift);
          auto *NewInst = BinaryOperator::CreateShl(Variable, ShiftConst);
          NewInst->insertBefore(BinOp->getIterator());
          BinOp->replaceAllUsesWith(NewInst);
          BinOp->eraseFromParent();
          ++NumReduced;
          Changed = true;
        }
      } else if (BinOp->getOpcode() == Instruction::UDiv) {
        // Division is not commutative: only the right-hand side (the
        // divisor) can be our power-of-two constant ("x / 4").
        Value *Dividend = BinOp->getOperand(0);
        Value *Divisor = BinOp->getOperand(1);

        if (isPowerOfTwoConstant(Divisor, Shift)) {
          Constant *ShiftConst =
              ConstantInt::get(Dividend->getType(), Shift);
          auto *NewInst = BinaryOperator::CreateLShr(Dividend, ShiftConst);
          NewInst->insertBefore(BinOp->getIterator());
          BinOp->replaceAllUsesWith(NewInst);
          BinOp->eraseFromParent();
          ++NumReduced;
          Changed = true;
        }
      }
    }
  }

  return Changed;
}
