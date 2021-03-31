// #include <llvm/IR/Function.h>
// #include <llvm/Pass.h>
// #include <llvm/Support/raw_ostream.h>
// #include <llvm/IR/IntrinsicInst.h>
// #include <set>
// #include <map>

// #include "Dataflow.h"
// using namespace llvm;

// struct FuncPtrInfo {
//    std::map<Value *, std::set<Value *>>  ptrMapSet;
//    FuncPtrInfo() : ptrMapSet() {}
//    FuncPtrInfo(const FuncPtrInfo & info) : ptrMapSet(info.ptrMapSet) {}
  
//    bool operator == (const FuncPtrInfo & info) const {
//        return ptrMapSet == info.ptrMapSet;
//    }
// };

// std::map<Function *, FuncPtrInfo> workFuncMap;

// class FuncPtrVisitor : public DataflowVisitor<struct FuncPtrInfo> {
// public:
//    FuncPtrVisitor() {}
//    void merge(FuncPtrInfo * dest, const FuncPtrInfo & src) override {
//     //    for (std::set<Instruction *>::const_iterator ii = src.ptrMapSet.begin(), 
//     //         ie = src.ptrMapSet.end(); ii != ie; ++ii) {
//     //        dest->ptrMapSet.insert(*ii);
//     //    }
//    }

//    void compDFVal(Instruction *inst, FuncPtrInfo * dfval) override{
//         if (isa<DbgInfoIntrinsic>(inst)) return;
//         dfval->ptrMapSet.erase(inst);
//         for(User::op_iterator oi = inst->op_begin(), oe = inst->op_end();
//             oi != oe; ++oi) {
//            Value * val = *oi;
//         //    if (isa<Instruction>(val)) 
//             //    dfval->ptrMapSet.insert(cast<Instruction>(val));
//        }
//    }
// };

// class FuncPtrPass : public ModulePass {
// public:
//     static char ID; 
//     FuncPtrPass() : ModulePass(ID) {}
//     bool runOnModule(Module &M) override {

//         // 已有函数Map
//         for (Function &F : M) {
//           FuncPtrInfo initval;
//           workFuncMap[&F]=initval;
//         }

//         // 处理所有函数
//         while (!workFuncMap.empty()){
//             Function* function = workFuncMap.begin()->first;//key
//             FuncPtrInfo funcPtrInfo = workFuncMap.begin()->second;//value
//             workFuncMap.erase(workFuncMap.begin());

//             FuncPtrVisitor visitor;
//             DataflowResult<FuncPtrInfo>::Type result;
//             compForwardDataflow(function, &visitor, &result, funcPtrInfo);
//         }

        
//         return false;
//     }
// };
// char FuncPtrPass::ID = 0;