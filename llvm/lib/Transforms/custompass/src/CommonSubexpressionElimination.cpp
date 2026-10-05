#include "CommonSubexpressionElimination.h"

#include "Common.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

// Our own definition of "these two instructions compute the same
// thing": same opcode, same result type, and every operand matches in
// the same position. This is a hand-written replacement for
// Instruction::isIdenticalTo().
static bool computeSameValue(const Instruction *A, const Instruction *B) {
  if (A->getOpcode() != B->getOpcode())
    return false;
  if (A->getType() != B->getType())
    return false;
  if (A->getNumOperands() != B->getNumOperands())
    return false;

  for (unsigned i = 0; i < A->getNumOperands(); ++i) {
    // Comparing the operand pointers directly works because LLVM gives
    // identical values (the same constant, the same earlier
    // instruction's result, etc.) the same pointer - it does not
    // create a second copy of "5" or of "%x" elsewhere.
    if (A->getOperand(i) != B->getOperand(i))
      return false;
  }

  return true;
}

// Instructions we should never deduplicate, even if computeSameValue
// says they match:
//   - anything with a side effect (a call, a store, ...): running it
//     twice vs. once can change the program's behavior.
//   - anything that reads memory (a load): the value at that address
//     could have changed between the two reads.
//   - PHI nodes and allocas: special-purpose instructions that this
//     simple pass should just leave alone.
static bool isEligibleForCSE(const Instruction *I) {
  if (hasSideEffects(I) || readsMemory(I))
    return false;
  if (isa<PHINode>(I) || isa<AllocaInst>(I))
    return false;
  return true;
}

bool runCommonSubexpressionElimination(Function &F, unsigned &NumRemoved) {
  bool Changed = false;

  for (BasicBlock &BB : F) {
    // Every eligible instruction we've already walked past in this
    // block, so we can compare each new one against everything seen
    // so far.
    SmallVector<Instruction *, 16> SeenInstructions;

    for (auto It = BB.begin(), End = BB.end(); It != End;) {
      Instruction *I = &*It++;

      if (!isEligibleForCSE(I))
        continue;

      Instruction *Duplicate = nullptr;
      for (Instruction *Earlier : SeenInstructions) {
        if (computeSameValue(Earlier, I)) {
          Duplicate = Earlier;
          break;
        }
      }

      if (Duplicate) {
        I->replaceAllUsesWith(Duplicate);
        I->eraseFromParent();
        ++NumRemoved;
        Changed = true;
      } else {
        SeenInstructions.push_back(I);
      }
    }
  }

  return Changed;
}
