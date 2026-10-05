#include "DeadCodeElimination.h"

#include "Common.h"

using namespace llvm;

// Our own definition of "dead": nothing uses this instruction's result
// (I->use_empty() just checks LLVM's bookkeeping list of who uses this
// value - it doesn't decide anything by itself), AND removing it
// wouldn't skip a side effect we defined in Common.cpp.
static bool isDead(const Instruction *I) {
  if (hasSideEffects(I))
    return false;
  return I->use_empty();
}

bool runDeadCodeElimination(Function &F, unsigned &NumRemoved) {
  bool Changed = false;

  // Keep sweeping the function until a full pass removes nothing. This
  // is needed because deleting one dead instruction can make an
  // instruction it used become dead too - e.g. if "%a = add %b, %c" is
  // deleted, %b or %c might now be unused as well.
  bool RemovedSomethingThisPass = true;
  while (RemovedSomethingThisPass) {
    RemovedSomethingThisPass = false;

    for (BasicBlock &BB : F) {
      for (auto It = BB.begin(), End = BB.end(); It != End;) {
        Instruction *I = &*It++;

        if (isDead(I)) {
          I->eraseFromParent();
          ++NumRemoved;
          Changed = true;
          RemovedSomethingThisPass = true;
        }
      }
    }
  }

  return Changed;
}
