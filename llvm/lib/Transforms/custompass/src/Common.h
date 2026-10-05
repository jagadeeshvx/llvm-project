#ifndef CUSTOMOPT_COMMON_H
#define CUSTOMOPT_COMMON_H

#include "llvm/IR/Instruction.h"

// ---------------------------------------------------------------------
// Shared helpers used by more than one optimization step.
//
// Both Dead Code Elimination and Common Subexpression Elimination need
// to answer the same question: "is it safe to skip running this
// instruction, or to treat it as interchangeable with an identical
// one?" That's only true if the instruction has no side effect. We
// define "side effect" ourselves here, by hand-checking the specific
// kinds of instructions that do something besides compute a value.
// ---------------------------------------------------------------------

// True if running I could change the program's behavior beyond just
// producing a value - e.g. calling a function, writing memory,
// branching. Such instructions must never be deleted just because
// their result is unused, and must never be treated as a duplicate of
// another "identical-looking" instruction.
bool hasSideEffects(const llvm::Instruction *I);

// True if I reads from memory (a load). Two loads from the same
// address can return different values if memory changed in between,
// so loads are never safe to treat as duplicates of each other.
bool readsMemory(const llvm::Instruction *I);

#endif // CUSTOMOPT_COMMON_H
