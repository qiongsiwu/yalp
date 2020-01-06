//=============================================================================
// Modeled after
// https://github.com/banach-space/llvm-tutor/blob/master/HelloWorld/HelloWorld.cpp
//
// FILE:
//    hello.cpp
//
// DESCRIPTION:
//    Visits all functions in a module, prints their names and the number of
//    arguments via stderr. Strictly speaking, this is an analysis pass (i.e.
//    the functions are not modified). However, in order to keep things simple
//    there's no 'print' method here, which every analysis pass should
//    implement. - Maybe we should try implemeting the print method?
//
// USAGE:
//    - New pass manager ONLY
//      # Define your pass pipeline via the '-passes' flag
//      opt -load-pass-plugin=libHelloWorld.dylib -passes="hello-world" -disable-output <input-llvm-file>
//=============================================================================

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {

// New pass manager implemtation
struct Trivial : PassInfoMixin<Trivial> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
        errs() << "Module Name is " << M.getModuleIdentifier() << "\n";
        return PreservedAnalyses::all();
    }
};

} // namespace

// New pass manager registration
llvm::PassPluginLibraryInfo getTrivialModulePluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "TrivialModulePass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                           if (Name == "trivial") {
                               MPM.addPass(Trivial());
                               return true;
                           }
                           return false;
                       }
                );
            }};
}

// This is the core interface for pass plugins - with this 'opt' will be able
// to recognize HelloWorld when added to the pass pipeline on the command line,
// i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getTrivialModulePluginInfo();
}
