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
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/CommandLine.h"

#include <assert.h>

#include "ObjFieldStore.hpp"

using namespace llvm;

#define DEBUG_TYPE "object-field-store"

static cl::opt<bool> AnalysisOnly ("analysis-only",
    cl::desc("Filter the relevant stores, but do not insert instrumentation"));

ObjFieldStore::ObjFieldStore(bool print) : print_to_errs{print} {}

PreservedAnalyses ObjFieldStore::run(Module &M, ModuleAnalysisManager &) {
    bool changed = runOnModule(M);
    if (!changed) {
        return PreservedAnalyses::all();
    } else {
        return PreservedAnalyses::none();
    }
}

bool ObjFieldStore::runOnModule(Module &M) {
    for (auto& F : M) {
        analyzeFunction(F);
    }

    if (print_to_errs || AnalysisOnly) {
        print(errs(), &M);
        return false;
    }

    return injectInstrumentation(M);
}

bool ObjFieldStore::analyzeFunction(Function &F) {
    for (auto& bb : F) {
        for (auto& inst : bb) {
            Instruction *InstPtr = &inst;
            if (isa<StoreInst>(InstPtr)) {
                StoreInst *store = dyn_cast<StoreInst>(InstPtr);
                auto pointer_op = store->getPointerOperand();
                if (isa<GetElementPtrInst>(pointer_op)
                 || isa<GEPOperator>(pointer_op)) {
                    all_stores.push_back(store);
                }
            }
        }
    }
    return false;
}

bool ObjFieldStore::injectInstrumentation(Module &M) {
    bool changed = false;
    // Step 1 create the function to inject
    // 1.1 create function prototype and insert it into the module
    auto &CTX = M.getContext();
    PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
    FunctionType *PrintfTy = FunctionType::get(
        IntegerType::getInt32Ty(CTX), PrintfArgTy, /*IsVarArgs*/true);
    FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
    // 1.2 set attributes
    Function *PrintF = dyn_cast<Function>(Printf.getCallee());
    PrintF->setDoesNotThrow();
    PrintF->addParamAttr(0, Attribute::NoCapture);
    PrintF->addParamAttr(0, Attribute::ReadOnly);

    // 1.3 create a global constant for the format string
    Constant *PrintfFormatStr = ConstantDataArray::getString(
        CTX, "Seen a relevant store instruction! %s\n");

    Constant *PrintfFormatStrVar =
        M.getOrInsertGlobal("PrintfFormatStr", PrintfFormatStr->getType());

    dyn_cast<GlobalVariable>(PrintfFormatStrVar)->setInitializer(PrintfFormatStr);

    // Step 2 Inject the function before each store
    for (auto inst : all_stores) {
        IRBuilder<> Builder(inst);

        auto tempName = Builder.CreateGlobalStringPtr("AAAAA");

        Value *FormatStrPtr = Builder.CreatePointerCast(
            PrintfFormatStrVar, PrintfArgTy, "formatStr");

        Builder.CreateCall(
            Printf, {FormatStrPtr, tempName});

        changed = true;
    }

    return changed;
}

void ObjFieldStore::print(raw_ostream &O, const Module *M) const {
    O << "Module id is " << M->getModuleIdentifier() << "\n";
    O << "Store instructions are \n";
    for (const auto* inst : all_stores) {
        O << *inst << "\n";

        if (isa<GetElementPtrInst>(inst->getPointerOperand())) {
            auto pointer_op = dyn_cast<GetElementPtrInst>(inst->getPointerOperand());
            O << "\t" << "destn type: "
                   << *(pointer_op->getSourceElementType()) << "\t";
            int i = 0;
            for (auto op = pointer_op->idx_begin();
                      op != pointer_op->idx_end(); ++op) {
                // Iterate through all operands
                if (isa<ConstantInt>(&(**op))) {
                    auto constantIntOp = dyn_cast<ConstantInt>(&(**op));
                    O << "field " << i << " idx " << *(constantIntOp->getType()) << " ";
                    O << constantIntOp->getValue();
                } else {
                    O << "field " << i << " is not constant";
                }
                O << "\t";
                i++;
            }
            O << "\n";
        } else if (isa<GEPOperator>(inst->getPointerOperand())) {
            auto pointer_op = dyn_cast<GEPOperator>(inst->getPointerOperand());
            O << "\t" << "destn type: "
                   << *(pointer_op->getSourceElementType()) << "\t";
            int i = 0;
            for (auto op = pointer_op->idx_begin();
                      op != pointer_op->idx_end(); ++op) {
                // Iterate through all operands
                if (isa<ConstantInt>(&(**op))) {
                    auto constantIntOp = dyn_cast<ConstantInt>(&(**op));
                    O << "field " << i << " idx " << *(constantIntOp->getType()) << " ";
                    O << constantIntOp->getValue();
                } else {
                    O << "field " << i << " is not constant";
                }
                O << "\t";
                i++;
            }
            O << "\n";
        }
    }
}

bool LegacyObjFieldStore::runOnModule(Module &M) {
    return Impl.runOnModule(M);
}

// Registering the pass with the new pass manager
PassPluginLibraryInfo getObjFieldStorePluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "ObjFieldStore", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                           if (Name == "obj-field-store") {
                               MPM.addPass(ObjFieldStore(false));
                               return true;
                           } else if (Name == "print<obj-field-store>") {
                               MPM.addPass(ObjFieldStore(true));
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

// Registering the pass with the old pass manager
char LegacyObjFieldStore::ID = 0;
static RegisterPass<LegacyObjFieldStore> X ("legacy-obj-field-store",
    "legacy-obj-FS", true, true);
