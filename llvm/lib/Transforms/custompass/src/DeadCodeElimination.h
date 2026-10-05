#ifndef CUSTOMOPT_DEAD_CODE_ELIMINATION_H
#define CUSTOMOPT_DEAD_CODE_ELIMINATION_H

#include "llvm/IR/Function.h"

// ---------------------------------------------------------------------
// Dead Code Elimination (written from scratch)
//
// Removes instructions whose result nothing uses, as long as removing
// them is safe (see Common.h's hasSideEffects - we wrote our own
// definition of "safe to remove" instead of calling LLVM's
// isInstructionTriviallyDead()).
//
// Deleting one dead instruction can make one of ITS operands dead too,
// so we keep sweeping the function until a full pass finds nothing
// left to remove.
// ---------------------------------------------------------------------

// Repeatedly removes dead instructions from F until none remain.
// NumRemoved is incremented once per instruction deleted.
// Returns true if anything in F was changed.
bool runDeadCodeElimination(llvm::Function &F, unsigned &NumRemoved);

#endif // CUSTOMOPT_DEAD_CODE_ELIMINATION_H
