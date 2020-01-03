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

// Function implements what this pass does
void visitor(Function &F) {
    errs() << "(hello)FunctionName : " << F.getName() << "\n";
    errs() << "(hello)  number of arguments: " << F.arg_size() << "\n";
}

// New pass manager implemtation
struct Hello : PassInfoMixin<Hello> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
        visitor(F);
        return PreservedAnalyses::all();
    }
};

} // namespace

// New pass manager registration
llvm::PassPluginLibraryInfo getHelloPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "Hello", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                           if (Name == "hello") {
                               FPM.addPass(Hello());
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
    return getHelloPluginInfo();
}
