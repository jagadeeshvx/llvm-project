#ifndef CUSTOMOPT_CONSTANT_FOLD_H
#define CUSTOMOPT_CONSTANT_FOLD_H

#include "llvm/IR/Function.h"

// ---------------------------------------------------------------------
// Constant Folding / Constant Propagation (written from scratch)
//
// Example:   %x = add i32 2, 3          becomes simply the number 5.
//
// We do NOT call LLVM's own ConstantFoldInstruction() helper. Instead,
// we recognize a small set of arithmetic/bitwise opcodes ourselves and
// compute the answer ourselves, using APInt only as a plain number
// container (LLVM's arbitrary-bit-width integer type - we need it
// instead of a normal C++ int because an LLVM integer can be any width,
// e.g. i7 or i256, not just 32 or 64 bits).
// ---------------------------------------------------------------------

// Scans every instruction in F. Whenever an instruction is a binary
// operator (add, sub, mul, ...) whose two operands are both constant
// integers, computes the result ourselves and replaces the instruction
// with it. NumFolded is incremented once per instruction folded.
// Returns true if anything in F was changed.
bool runConstantFolding(llvm::Function &F, unsigned &NumFolded);

#endif // CUSTOMOPT_CONSTANT_FOLD_H
