#include "AlgebraicSimplify.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

static bool isConstWithValue(Value *V, int64_t N) {
  auto *C = dyn_cast<ConstantInt>(V);
  return C && C->getValue() == N;
}

static bool isAllOnes(Value *V) {
  auto *C = dyn_cast<ConstantInt>(V);
  return C && C->getValue().isAllOnes();
}

// Returns the value BO can be replaced with, or nullptr if no identity
// applies.
static Value *simplify(BinaryOperator *BO) {
  Value *L = BO->getOperand(0), *R = BO->getOperand(1);
  Type *Ty = BO->getType();
  Constant *Zero = ConstantInt::get(Ty, 0);
  Constant *One = ConstantInt::get(Ty, 1);

  switch (BO->getOpcode()) {
  case Instruction::Add:
    if (isConstWithValue(R, 0)) return L;
    if (isConstWithValue(L, 0)) return R;
    return nullptr;
  case Instruction::Sub:
    if (L == R) return Zero;
    if (isConstWithValue(R, 0)) return L;
    return nullptr;
  case Instruction::Mul:
    if (isConstWithValue(R, 1)) return L;
    if (isConstWithValue(L, 1)) return R;
    if (isConstWithValue(R, 0) || isConstWithValue(L, 0)) return Zero;
    return nullptr;
  case Instruction::UDiv:
  case Instruction::SDiv:
    if (L == R) return One;
    if (isConstWithValue(R, 1)) return L;
    return nullptr;
  case Instruction::URem:
  case Instruction::SRem:
    if (L == R) return Zero;
    if (isConstWithValue(R, 1)) return Zero;
    return nullptr;
  case Instruction::And:
    if (L == R) return L;
    if (isConstWithValue(R, 0) || isConstWithValue(L, 0)) return Zero;
    if (isAllOnes(R)) return L;
    if (isAllOnes(L)) return R;
    return nullptr;
  case Instruction::Or:
    if (L == R) return L;
    if (isConstWithValue(R, 0)) return L;
    if (isConstWithValue(L, 0)) return R;
    return nullptr;
  case Instruction::Xor:
    if (L == R) return Zero;
    if (isConstWithValue(R, 0)) return L;
    if (isConstWithValue(L, 0)) return R;
    return nullptr;
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
    if (isConstWithValue(R, 0)) return L;
    return nullptr;
  default:
    return nullptr;
  }
}

bool runAlgebraicSimplify(Function &F, unsigned &NumSimplified) {
  bool Changed = false;
  for (BasicBlock &BB : F) {
    for (auto It = BB.begin(), End = BB.end(); It != End;) {
      Instruction *I = &*It++;
      auto *BO = dyn_cast<BinaryOperator>(I);
      if (!BO || !BO->getType()->isIntegerTy())
        continue;
      Value *New = simplify(BO);
      if (!New || New == BO)
        continue;
      BO->replaceAllUsesWith(New);
      BO->eraseFromParent();
      ++NumSimplified;
      Changed = true;
    }
  }
  return Changed;
}
