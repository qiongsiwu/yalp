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
private:
    std::vector<llvm::StoreInst *> all_stores;
    bool print_to_errs;

    bool runOnFunction(llvm::Function &F);
};
#endif
