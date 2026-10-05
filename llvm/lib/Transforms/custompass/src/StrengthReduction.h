#ifndef CUSTOMOPT_STRENGTH_REDUCTION_H
#define CUSTOMOPT_STRENGTH_REDUCTION_H

#include "llvm/IR/Function.h"

// ---------------------------------------------------------------------
// Strength Reduction (written from scratch)
//
//   x * 8   ==  x << 3      (8 is 2^3)
//   x / 4   ==  x >> 2      (4 is 2^2, unsigned division only)
//
// We do NOT call LLVM's APInt::isPowerOf2()/logBase2() helpers.
// Instead we scan the constant's bits ourselves to decide whether
// exactly one bit is set, and which position it's in.
//
// Note: only *unsigned* division is handled. Signed division by a
// power of two is not simply a shift when the value can be negative
// (rounding works differently), so that case is left out.
// ---------------------------------------------------------------------

// Scans every instruction in F for multiply/unsigned-divide by a power
// of two, and rewrites it as a shift. NumReduced is incremented once
// per instruction rewritten. Returns true if anything in F was changed.
bool runStrengthReduction(llvm::Function &F, unsigned &NumReduced);

#endif // CUSTOMOPT_STRENGTH_REDUCTION_H
