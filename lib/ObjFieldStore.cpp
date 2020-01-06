//=============================================================================
//
// FILE:
//    ObjFieldStore.cpp
//
// DESCRIPTION:
//    Visits all functions in a module and annotate the store instructions.
//    Results: report all store instructions to a field of an aggregated data
//             structure.
//    This is an analysis pass, so it should implement the print method, which
//    should print out all the instructions in the module that stores to a field
//    of an aggreagted structure.
//
//
// USAGE:
//    - New pass manager ONLY
//      Pass Name: obj-field-store
//      # Define your pass pipeline via the '-passes' flag
//      opt -load-pass-plugin=libObjFieldStore.dylib -passes="obj-field-store" -disable-output <input-llvm-file>
//=============================================================================

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <assert.h>

using namespace llvm;

// New pass manager interface
struct ObjFieldStore : public PassInfoMixin<ObjFieldStore> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
    bool runOnModule(Module &M);
};

PreservedAnalyses ObjFieldStore::run(Module &M, ModuleAnalysisManager &) {
    bool changed = runOnModule(M);
    assert(!changed);
    return PreservedAnalyses::all();
}

bool ObjFieldStore::runOnModule(Module &M) {
    errs() << "Module id is " << M.getModuleIdentifier() << "\n";
    // This is an analysis pass. Nothing is changed.
    // Maybe we can pass on a data structure to the transformation pass
    return false;
}

// Registering the pass with the new pass manager
PassPluginLibraryInfo getObjFieldStorePluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "ObjFieldStore", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                           if (Name == "obj-field-store") {
                               MPM.addPass(ObjFieldStore());
                               return true;
                           }
                           return false;
                       }
                );
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getObjFieldStorePluginInfo();
}
