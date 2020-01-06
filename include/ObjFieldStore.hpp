//==============================================================================
// FILE:
//    ObjFieldStore.hpp
//
// DESCRIPTION:
//    Declares the ObjFieldStore pass for the new pass manager
//==============================================================================

#ifndef YALP_OBJFIELDSTORE_HPP
#define YALP_OBJFIELDSTORE_HPP

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

// New pass manager interface
struct ObjFieldStore : public llvm::PassInfoMixin<ObjFieldStore> {
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
    bool runOnModule(llvm::Module &M);
};
#endif
