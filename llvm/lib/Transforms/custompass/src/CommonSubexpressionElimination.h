#ifndef CUSTOMOPT_CSE_H
#define CUSTOMOPT_CSE_H

#include "llvm/IR/Function.h"

// ---------------------------------------------------------------------
// Common Subexpression Elimination (written from scratch)
//
// Example:
//   %a = mul i32 %x, %y
//   ...
//   %b = mul i32 %x, %y     <- identical to %a: reuse %a instead
//
// We do NOT call LLVM's Instruction::isIdenticalTo(). Instead we wrote
// our own comparison: two instructions are "the same computation" if
// they have the same opcode, the same result type, and the same
// operands in the same order.
//
// This version only looks within a single basic block (a straight-line
// run of instructions with no branches). An instruction earlier in the
// same block always runs before one later in that block, so reusing
// its result is always safe. Finding duplicates ACROSS blocks would
// need a dominator tree to stay safe - left out here to keep the logic
// easy to follow.
// ---------------------------------------------------------------------

// Scans each basic block of F for repeated, identical instructions and
// replaces the later copies with the first one. NumRemoved is
// incremented once per duplicate instruction removed.
// Returns true if anything in F was changed.
bool runCommonSubexpressionElimination(llvm::Function &F,
                                        unsigned &NumRemoved);

#endif // CUSTOMOPT_CSE_H
