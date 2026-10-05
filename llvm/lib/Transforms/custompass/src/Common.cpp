#include "Common.h"

#include "llvm/IR/Instructions.h"

using namespace llvm;

bool hasSideEffects(const Instruction *I) {
  // Calling a function: the function might print something, modify a
  // global, loop forever, crash, etc. We can never assume it's safe to
  // skip or duplicate a call.
  if (isa<CallInst>(I) || isa<InvokeInst>(I) || isa<CallBrInst>(I))
    return true;

  // Writing to memory changes what every later load from that address
  // will see.
  if (isa<StoreInst>(I))
    return true;

  // Fences and atomic read-modify-write operations coordinate with
  // other threads; never treat them as removable or duplicable.
  if (isa<FenceInst>(I) || isa<AtomicCmpXchgInst>(I) ||
      isa<AtomicRMWInst>(I))
    return true;

  // Terminators (ret, br, switch, ...) control which code runs next.
  // Every basic block must end with exactly one, so these are never
  // "dead" even if nothing uses their (nonexistent) result.
  if (I->isTerminator())
    return true;

  return false;
}

bool readsMemory(const Instruction *I) { return isa<LoadInst>(I); }
