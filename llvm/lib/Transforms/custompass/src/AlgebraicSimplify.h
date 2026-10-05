#ifndef CUSTOMOPT_ALGEBRAIC_SIMPLIFY_H
#define CUSTOMOPT_ALGEBRAIC_SIMPLIFY_H

#include "llvm/IR/Function.h"

// ---------------------------------------------------------------------
// Algebraic identity simplification (written from scratch)
//
//   x - x -> 0     x ^ x -> 0     x / x -> 1     x % x -> 0
//   x & x -> x     x | x -> x
//   x + 0 -> x     x - 0 -> x     x * 1 -> x     x / 1 -> x
//   x * 0 -> 0     x & 0 -> 0     x & -1 -> x    x | 0, x ^ 0 -> x
//   x << 0, x >> 0 -> x
//
// (x / x -> 1 is valid because x == 0 would already be undefined.)
// NumSimplified is incremented once per instruction replaced.
// Returns true if anything in F was changed.
// ---------------------------------------------------------------------
bool runAlgebraicSimplify(llvm::Function &F, unsigned &NumSimplified);

#endif // CUSTOMOPT_ALGEBRAIC_SIMPLIFY_H
