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

//#include "llvm/IR/Instructions.h"
#include <vector>

// New pass manager interface
struct ObjFieldStore : public llvm::PassInfoMixin<ObjFieldStore> {
    ObjFieldStore(bool print);

    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
    bool runOnModule(llvm::Module &M);

    virtual void print(llvm::raw_ostream &O, const llvm::Module *M) const;

    const std::vector<llvm::StoreInst *>& getStores() const {
        return all_stores;
    }

private:
    std::vector<llvm::StoreInst *> all_stores;
    bool print_to_errs;

    bool analyzeFunction(llvm::Function &F);
    bool injectInstrumentation(llvm::Module &M);
};

// Legacy pass manager interface
struct LegacyObjFieldStore : public llvm::ModulePass {
    static char ID;
    LegacyObjFieldStore() : ModulePass(ID), Impl(false){}
    bool runOnModule(llvm::Module &M) override;

    const std::vector<llvm::StoreInst *>& getStores() const {
        return Impl.getStores();
    }

private:
    ObjFieldStore Impl;
};
#endif
