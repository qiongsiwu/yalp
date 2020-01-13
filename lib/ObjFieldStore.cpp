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
#include "llvm/IR/TypeFinder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/CommandLine.h"

#include <assert.h>
#include <map>
#include <vector>

#include "ObjFieldStore.hpp"

using namespace llvm;

#define DEBUG_TYPE "object-field-store"

// Main llvm pass

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

// Helper functions
Constant *createGlobalStringConstant(
    LLVMContext &CTX, Module &M, StringRef VarName, StringRef Value) {
    Constant *Str = ConstantDataArray::getString(CTX, Value);
    Constant *StrVar = M.getOrInsertGlobal(VarName, Str->getType());
    dyn_cast<GlobalVariable>(StrVar)->setInitializer(Str);
    return StrVar;
}

static const std::string IntegerFormat = "| %d ";
static const std::string PointerFormat = "| %p ";

const std::string &getPrintFmtForType(Type *t) {
    // Extend to other types later
    if (isa<PointerType>(t)) {
        return PointerFormat;
    } else {
        return IntegerFormat;
    }
}

Constant *getStructName(std::map<Type *, Constant *> NameDict, Type *t,
                        unsigned &arrCounter, LLVMContext &CTX, Module &M) {
  auto it = NameDict.find(t);

  if (it == NameDict.end()) {
      std::string arr_id;
      raw_string_ostream arrid_so(arr_id);
      arrid_so << "arr_" << arrCounter;
      arrCounter++;

      std::string arr_str;
      raw_string_ostream rso(arr_str);
      t->print(rso);

      Constant *ArrConst =
          createGlobalStringConstant(CTX, M, arrid_so.str(), rso.str());

      return ArrConst;
  } else {
      return it->second;
  }
}

// end of helpers

template <typename T>
bool createInstrumentationInstructions(
    T *pointer_op, std::map<Type *, Constant *> StructTypeNames,
    std::map<int, Constant *>PrintFormatStrs,
    StoreInst *StrInst, Module &M, FunctionCallee &Printf,
    PointerType *PrintfArgTy, unsigned &uCounter) {
    IRBuilder<> Builder(StrInst);
    auto& CTX = M.getContext();

    Constant *SourceTypeName =
        getStructName(StructTypeNames, pointer_op->getSourceElementType(),
                      uCounter, CTX, M);

    Constant *ResultTypeName =
        getStructName(StructTypeNames, pointer_op->getResultElementType(),
                      uCounter, CTX, M);

    std::string FormatString = "[ValueProf] | %s | %s | %p ";
    int numIdxes = 0;
    std::vector<Value *> args{SourceTypeName, ResultTypeName, pointer_op};
    for (auto op = pointer_op->idx_begin();
              op != pointer_op->idx_end(); ++op) {
        FormatString.append("| %d ");
        args.push_back(*op);
        numIdxes++;
    }

    // For the value operand. This need to be changed to deal with more types!
    FormatString.append(
        getPrintFmtForType(StrInst->getValueOperand()->getType()));
    FormatString.append("\n");

    Constant *PrintFormatStrPtr;
    auto fstrIter = PrintFormatStrs.find(numIdxes);
    if (fstrIter == PrintFormatStrs.end()) {
        std::string var_name;
        raw_string_ostream rso(var_name);
        rso << "PrintFormatStr_" << numIdxes;
        Constant *NewStringConst =
            createGlobalStringConstant(CTX, M, rso.str(), FormatString);
        PrintFormatStrs.insert(
            std::pair<int, Constant *>(numIdxes, NewStringConst));
        PrintFormatStrPtr = NewStringConst;
    } else {
        PrintFormatStrPtr = fstrIter->second;
    }

    Value *FormatStrPtr =
        Builder.CreatePointerCast(PrintFormatStrPtr, PrintfArgTy, "formatStr");

    args.insert(args.begin(), FormatStrPtr);
    args.push_back(StrInst->getValueOperand());

    Builder.CreateCall(Printf, args);

    return true;
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

    // Step 2 Inject the function before each store
    std::map<Type *, Constant *> StructTypeNames;
    TypeFinder StructTypes;
    StructTypes.run(M, true);

    std::map<int, Constant *> PrintFormatStrs;

    // Set up a table with the names of the structs
    unsigned int sCounter = 1;
    for (auto *STy : StructTypes) {
        std::string type_str;
        raw_string_ostream rso(type_str);
        STy->print(rso);

        std::string type_id;
        raw_string_ostream tyidso(type_id);
        tyidso << "struct_" << sCounter;

        Constant *StructName = createGlobalStringConstant(
            CTX, M, tyidso.str(), rso.str());

        StructTypeNames.insert(
            std::pair<Type *, Constant *>(STy, StructName));
        sCounter++;
    }

    unsigned int uCounter = 1;
    for (auto inst : all_stores) {
        IRBuilder<> Builder(inst);

        auto pointer_op = dyn_cast<GetElementPtrInst>(inst->getPointerOperand());

        if (pointer_op) {
            changed = createInstrumentationInstructions<GetElementPtrInst>(
                pointer_op, StructTypeNames, PrintFormatStrs,
                inst, M, Printf, PrintfArgTy,
                uCounter);
        } else {
          // the pointer operand must be a GEP operator
          auto GEPOp = dyn_cast<GEPOperator>(inst->getPointerOperand());
          assert(GEPOp);

          changed = createInstrumentationInstructions<GEPOperator>(
              GEPOp, StructTypeNames, PrintFormatStrs,
              inst, M, Printf, PrintfArgTy, uCounter);
        }
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
