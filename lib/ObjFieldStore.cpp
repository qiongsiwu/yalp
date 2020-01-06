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

bool runOnModule(Module &M) {
    errs() << "Module name is " << M.getName() << "\n";
    // This is an analysis pass. Nothing is changed.
    // Maybe we can pass on a data structure to the transformation pass
    return false;
}

PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    bool changed = runOnModule(M);
    assert(!changed);
    return PreservedAnalyses::all();
}
