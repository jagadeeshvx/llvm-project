//===- CustomOptPass.cpp - Driver that runs all four optimizations ------===//
//
// This file doesn't contain any optimization logic itself - it just
// calls, in order, the four optimization passes that live in their own
// files, each written from scratch (no calls to LLVM's built-in
// constant-folding, dead-code, or instruction-matching helpers):
//
//   ConstantFold.cpp                   - constant propagation/folding
//   StrengthReduction.cpp              - multiply/divide by power of 2
//   CommonSubexpressionElimination.cpp - CSE
//   DeadCodeElimination.cpp            - DCE
//
// Common.cpp holds one small helper (hasSideEffects/readsMemory) shared
// by the DCE and CSE steps, so that definition only has to be written
// once.
//
//===----------------------------------------------------------------------===//

#include "AlgebraicSimplify.h"
#include "CommonSubexpressionElimination.h"
#include "ConstantFold.h"
#include "DeadCodeElimination.h"
#include "StrengthReduction.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct CustomOptPass : public OptionalPassInfoMixin<CustomOptPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    unsigned NumFolded = 0;
    unsigned NumSimplified = 0;
    unsigned NumReduced = 0;
    unsigned NumCSE = 0;
    unsigned NumDCE = 0;

    bool ChangedAnything = false;

    // Iterate to a fixed point: each step can expose work for another
    // (e.g. a/a -> 1 lets constant folding fold the next instruction).
    // Capped so a bug can never make this loop forever.
    for (int Round = 0; Round < 16; ++Round) {
      bool Changed = false;
      Changed |= runAlgebraicSimplify(F, NumSimplified);
      Changed |= runConstantFolding(F, NumFolded);
      Changed |= runStrengthReduction(F, NumReduced);
      Changed |= runCommonSubexpressionElimination(F, NumCSE);
      Changed |= runDeadCodeElimination(F, NumDCE);
      if (!Changed)
        break;
      ChangedAnything = true;
    }

    if (ChangedAnything) {
      errs() << "[custom-opt] " << F.getName() << ": " << NumFolded
             << " folded, " << NumSimplified << " simplified, " << NumReduced << " strength-reduced, " << NumCSE
             << " CSE'd, " << NumDCE << " dead instructions removed\n";
    }

    return ChangedAnything ? PreservedAnalyses::none()
                            : PreservedAnalyses::all();
  }
};

} // end anonymous namespace

// ---------------------------------------------------------------------
// Boilerplate LLVM needs to find our pass. No need to change this even
// if you add more optimization steps above.
// ---------------------------------------------------------------------
PassPluginLibraryInfo getCustomOptPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CustomOptPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "custom-opt") {
                    FPM.addPass(CustomOptPass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCustomOptPassPluginInfo();
}
