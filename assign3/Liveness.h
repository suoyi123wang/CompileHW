//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/InstIterator.h>

#include <vector>
#include <map>
#include <set>

#include "Dataflow.h"
using namespace llvm;


// struct LivenessInfo {
//    std::set<Instruction *> LiveVars;             /// Set of variables which are live
//    LivenessInfo() : LiveVars() {}
//    LivenessInfo(const LivenessInfo & info) : LiveVars(info.LiveVars) {}
  
//    bool operator == (const LivenessInfo & info) const {
//        return LiveVars == info.LiveVars;
//    }
// };
struct FuncPtrInfo {
   std::map<Value *, std::set<Value *>>  ptrMapSet;
   std::map<Value *, std::set<Value *>>  ptrMapSetField;

   FuncPtrInfo() : ptrMapSet(),ptrMapSetField() {}
   FuncPtrInfo(const FuncPtrInfo & info) : ptrMapSet(info.ptrMapSet),ptrMapSetField(info.ptrMapSetField) {}

   bool operator==(const FuncPtrInfo &info) const
    {
        return (ptrMapSet == info.ptrMapSet) && (ptrMapSetField == info.ptrMapSetField);
    }

    bool operator!=(const FuncPtrInfo &info) const
    {
        return (ptrMapSet != info.ptrMapSet) || (ptrMapSetField != info.ptrMapSetField);
    }

    FuncPtrInfo &operator=(const FuncPtrInfo &info)
    {
        ptrMapSet = info.ptrMapSet;
        ptrMapSetField = info.ptrMapSetField;
        return *this;
    }
  
//    bool operator == (const FuncPtrInfo & info) const {
//        return ptrMapSet == info.ptrMapSet;
//    }
};
class FuncPtrVisitor : public DataflowVisitor<struct FuncPtrInfo> {
public:
   std::map<CallInst *, std::set<Function *>> call_func_result;
   std::set<Function *> fn_worklist;

   FuncPtrVisitor(): call_func_result(),fn_worklist(){}

   // 处理不同指令时，都是先从result的Map中获取该指令的入口值，然后经过计算，得到该指指令的出口值，最后把出口值再赋值回result

   void dealPhiNode(PHINode* phiNode, DataflowResult<FuncPtrInfo>::Type *result){
       FuncPtrInfo ptrInfo = (*result)[phiNode].first;
       (*result).erase(phiNode);
       for (Value * incoming : phiNode->incoming_values()){
           if(isa<Function>(incoming)){
               ptrInfo.ptrMapSet[phiNode].insert(incoming);
           }else{
               ptrInfo.ptrMapSet[phiNode].insert(ptrInfo.ptrMapSet[phiNode].begin(),
                                                 ptrInfo.ptrMapSet[phiNode].end());
           }
       }
       (*result)[phiNode].second = ptrInfo;
   }
   void dealCallInst(CallInst* callInst, DataflowResult<FuncPtrInfo>::Type *result){
       FuncPtrInfo dfval = (*result)[callInst].first;
    //    std::map<CallInst *, std::set<Function *>> callsMapSet;
    //    Function * callFuncs = callInst->getCalledFunction();
        std::set<Function *> callees;
        //callee被调用者，caller调用者
        Value *value = callInst->getCalledValue();
        if (auto *func = dyn_cast<Function>(value))
        {
            callees.insert(func);
        }
        else
        {
            std::set<Value *> value_worklist;
            if (dfval.ptrMapSet.count(value))
            {
                value_worklist.insert(dfval.ptrMapSet[value].begin(), dfval.ptrMapSet[value].end());
            }

            while (!value_worklist.empty())
            {
                Value *v = *(value_worklist.begin());
                value_worklist.erase(value_worklist.begin());
                if (auto *func = dyn_cast<Function>(v))
                {
                    callees.insert(func);
                }
                else
                {
                    value_worklist.insert(dfval.ptrMapSet[v].begin(), dfval.ptrMapSet[v].end());
                }
                //前向访问找到所有的func
            }
        }

        call_func_result[callInst].clear();
        call_func_result[callInst].insert(callees.begin(), callees.end());

        /// Return the function called, or null if this is an
        /// indirect function invocation.
        if (callInst->getCalledFunction() && callInst->getCalledFunction()->isDeclaration())
        {
            (*result)[callInst].second = (*result)[callInst].first;
            return;
        }

        for (auto calleei = callees.begin(), calleee = callees.end(); calleei != calleee; calleei++)
        {
            Function *callee = *calleei;
            // 声明不算
            if (callee->isDeclaration())
            {
                continue;
            }
            std::map<Value *, Argument *> ValueToArg_map;

            for (int argi = 0, arge = callInst->getNumArgOperands(); argi < arge; argi++)
            {
                Value *caller_arg = callInst->getArgOperand(argi);
                if (caller_arg->getType()->isPointerTy())
                {
                    // only consider pointer
                    Argument *callee_arg = callee->arg_begin() + argi;
                    ValueToArg_map.insert(std::make_pair(caller_arg, callee_arg));
                }
            }

            if (ValueToArg_map.empty())
            {
                FuncPtrInfo tmpdfval = (*result)[callInst].second;
                merge(&tmpdfval, (*result)[callInst].first);
                continue;
            }

            // replace LiveVars_map
            FuncPtrInfo tmpdfval = (*result)[callInst].first;
            FuncPtrInfo &callee_dfval_in = (*result)[&*inst_begin(callee)].first;
            FuncPtrInfo old_callee_dfval_in = callee_dfval_in;
            for (auto bi = tmpdfval.ptrMapSet.begin(), be = tmpdfval.ptrMapSet.end(); bi != be; bi++)
            {
                for (auto argi = ValueToArg_map.begin(), arge = ValueToArg_map.end(); argi != arge; argi++)
                {
                    if (bi->second.count(argi->first) && !isa<Function>(argi->first))
                    {
                        // 函数
                        bi->second.erase(argi->first);
                        bi->second.insert(argi->second);
                    }
                }
            }

            // replace LiveVars_feild_map
            for (auto bi = tmpdfval.ptrMapSetField.begin(), be = tmpdfval.ptrMapSetField.end(); bi != be; bi++)
            {
                for (auto argi = ValueToArg_map.begin(), arge = ValueToArg_map.end(); argi != arge; argi++)
                {
                    if (bi->second.count(argi->first) && !isa<Function>(argi->first))
                    {
                        bi->second.erase(argi->first);
                        bi->second.insert(argi->second);
                    }
                }
            }

            for (auto argi = ValueToArg_map.begin(), arge = ValueToArg_map.end(); argi != arge; argi++)
            {
                if (tmpdfval.ptrMapSet.count(argi->first))
                {
                    std::set<Value *> values = tmpdfval.ptrMapSet[argi->first];
                    tmpdfval.ptrMapSet.erase(argi->first);
                    tmpdfval.ptrMapSet[argi->second].insert(values.begin(), values.end());
                }

                if (tmpdfval.ptrMapSetField.count(argi->first))
                {
                    std::set<Value *> values = tmpdfval.ptrMapSetField[argi->first];
                    tmpdfval.ptrMapSetField.erase(argi->first);
                    tmpdfval.ptrMapSetField[argi->second].insert(values.begin(), values.end());
                }
            }

            for (auto argi = ValueToArg_map.begin(), arge = ValueToArg_map.end(); argi != arge; argi++)
            {
                if (isa<Function>(argi->first))
                {
                    tmpdfval.ptrMapSet[argi->second].insert(argi->first);
                }
            }

            merge(&callee_dfval_in, tmpdfval);
            if (old_callee_dfval_in == callee_dfval_in){        
            }else{
                fn_worklist.insert(callee);
            }
        }
       



   }
   void dealRetInst(ReturnInst *returnInst, DataflowResult<FuncPtrInfo>::Type *result)
    {
        FuncPtrInfo dfval = (*result)[returnInst].first;

        Function *callee = returnInst->getFunction();
        //前向找到哪个函数调用了return的函数
        for (auto funci = call_func_result.begin(), funce = call_func_result.end(); funci != funce; funci++)
        {
            if (funci->second.count(callee))
            { // funci call callee
                CallInst *callInst = funci->first;
                Function *caller = callInst->getFunction();

                std::map<Value *, Argument *> ValueToArg_map;
                for (unsigned argi = 0, arge = callInst->getNumArgOperands(); argi < arge; argi++)
                {
                    Value *caller_arg = callInst->getArgOperand(argi);
                    if (!caller_arg->getType()->isPointerTy())
                        continue;
                    Argument *callee_arg = callee->arg_begin() + argi;
                    ValueToArg_map.insert(std::make_pair(caller_arg, callee_arg));
                }

                FuncPtrInfo tmpdfval = (*result)[returnInst].first;
                FuncPtrInfo &caller_dfval_out = (*result)[callInst].second;
                FuncPtrInfo old_caller_dfval_out = caller_dfval_out;

                if (returnInst->getReturnValue() &&
                    returnInst->getReturnValue()->getType()->isPointerTy())
                {
                    std::set<Value *> values = tmpdfval.ptrMapSet[returnInst->getReturnValue()];
                    tmpdfval.ptrMapSet.erase(returnInst->getReturnValue());
                    tmpdfval.ptrMapSet[callInst].insert(values.begin(), values.end());
                }
                // // replace LiveVars_map
                for (auto bi = tmpdfval.ptrMapSet.begin(), be = tmpdfval.ptrMapSet.end(); bi != be; bi++)
                {
                    for (auto argi = ValueToArg_map.begin(), arge = ValueToArg_map.end(); argi != arge; argi++)
                    {
                        if (bi->second.count(argi->second))
                        {
                            bi->second.erase(argi->second);
                            bi->second.insert(argi->first);
                        }
                    }
                }

                // replace LiveVars_feild_map
                for (auto bi = tmpdfval.ptrMapSetField.begin(), be = tmpdfval.ptrMapSetField.end(); bi != be; bi++)
                {
                    for (auto argi = ValueToArg_map.begin(), arge = ValueToArg_map.end(); argi != arge; argi++)
                    {
                        if (bi->second.count(argi->second))
                        {
                            bi->second.erase(argi->second);
                            bi->second.insert(argi->first);
                        }
                    }
                }
                for (auto argi = ValueToArg_map.begin(), arge = ValueToArg_map.end(); argi != arge; argi++)
                {
                    if (tmpdfval.ptrMapSet.count(argi->second))
                    {
                        std::set<Value *> values = tmpdfval.ptrMapSet[argi->second];
                        tmpdfval.ptrMapSet.erase(argi->second);
                        tmpdfval.ptrMapSet[argi->first].insert(values.begin(), values.end());
                    }
                    if (tmpdfval.ptrMapSetField.count(argi->second))
                    {
                        std::set<Value *> values = tmpdfval.ptrMapSetField[argi->second];
                        tmpdfval.ptrMapSetField.erase(argi->second);
                        tmpdfval.ptrMapSetField[argi->first].insert(values.begin(), values.end());
                    }
                }

                merge(&caller_dfval_out, tmpdfval);
                if (caller_dfval_out == old_caller_dfval_out)
                {
                    
                }else{
                    fn_worklist.insert(caller);
                }
            }
        }
        (*result)[returnInst].second = dfval;
    }
   void dealStoreInst(StoreInst *storeInst, DataflowResult<FuncPtrInfo>::Type *result){
        FuncPtrInfo dfval = (*result)[storeInst].first;

        std::set<Value *> values;
        if (dfval.ptrMapSet[storeInst->getValueOperand()].empty())
        {
            values.insert(storeInst->getValueOperand());
        }
        else
        {
            std::set<Value *> &tmp = dfval.ptrMapSet[storeInst->getValueOperand()];
            values.insert(tmp.begin(), tmp.end());
        }

        if (auto *getElementPtrInst = dyn_cast<GetElementPtrInst>(storeInst->getPointerOperand()))
        {
            Value *pointerOperand = getElementPtrInst->getPointerOperand();
            if (dfval.ptrMapSet[pointerOperand].empty())
            {
                dfval.ptrMapSetField[pointerOperand].clear();
                dfval.ptrMapSetField[pointerOperand].insert(values.begin(), values.end());
            }
            else
            {
                std::set<Value *> &tmp = dfval.ptrMapSet[pointerOperand];
                for (auto tmpi = tmp.begin(), tmpe = tmp.end(); tmpi != tmpe; tmpi++)
                {
                    Value *v = *tmpi;
                    dfval.ptrMapSetField[v].clear();
                    dfval.ptrMapSetField[v].insert(values.begin(), values.end());
                }
            }
        }
        else
        {
            //ptr
            dfval.ptrMapSet[storeInst->getPointerOperand()].clear();
            dfval.ptrMapSet[storeInst->getPointerOperand()].insert(values.begin(), values.end());
        }

        (*result)[storeInst].second = dfval;
    }
   void dealLoadInst(LoadInst *loadInst, DataflowResult<FuncPtrInfo>::Type *result)
    {
        FuncPtrInfo dfval = (*result)[loadInst].first;

        dfval.ptrMapSet[loadInst].clear();
        if (auto *getElementPtrInst = dyn_cast<GetElementPtrInst>(loadInst->getPointerOperand()))
        {
            Value *pointerOperand = getElementPtrInst->getPointerOperand();
            if (dfval.ptrMapSet[pointerOperand].empty())
            {
                std::set<Value *> &tmp = dfval.ptrMapSetField[pointerOperand];
                dfval.ptrMapSet[loadInst].insert(tmp.begin(), tmp.end());
            }
            else
            {
                std::set<Value *> &values = dfval.ptrMapSet[pointerOperand];
                for (auto valuei = values.begin(), valuee = values.end(); valuei != valuee; valuei++)
                {
                    Value *v = *valuei;
                    std::set<Value *> &tmp = dfval.ptrMapSetField[v];
                    dfval.ptrMapSet[loadInst].insert(tmp.begin(), tmp.end());
                }
            }
        }
        else
        {
            // ptr
            std::set<Value *> &tmp = dfval.ptrMapSet[loadInst->getPointerOperand()];
            dfval.ptrMapSet[loadInst].insert(tmp.begin(), tmp.end());
        }
        (*result)[loadInst].second = dfval;
    }
   void dealGetElementPtrInst(GetElementPtrInst *getElementPtrInst, DataflowResult<FuncPtrInfo>::Type *result){
        FuncPtrInfo dfval = (*result)[getElementPtrInst].first;
        dfval.ptrMapSet[getElementPtrInst].clear();
        Value *pointerOperand = getElementPtrInst->getPointerOperand();
        if (dfval.ptrMapSet[pointerOperand].empty())
        {
            dfval.ptrMapSet[getElementPtrInst].insert(pointerOperand);
        }
        else
        {
            dfval.ptrMapSet[getElementPtrInst].insert(dfval.ptrMapSet[pointerOperand].begin(), dfval.ptrMapSet[pointerOperand].end());
        }

        (*result)[getElementPtrInst].second = dfval;
    }
   void dealBitCastInst(BitCastInst *bitCastInst, DataflowResult<FuncPtrInfo>::Type *result)
    {
        FuncPtrInfo dfval = (*result)[bitCastInst].first;
        (*result)[bitCastInst].second = dfval;
    }
   void dealMemCpyInst(MemCpyInst *memCpyInst, DataflowResult<FuncPtrInfo>::Type *result)
    {
        FuncPtrInfo dfval = (*result)[memCpyInst].first;

        auto *b1 = dyn_cast<BitCastInst>(memCpyInst->getArgOperand(0));
        auto *b2 = dyn_cast<BitCastInst>(memCpyInst->getArgOperand(1));
        if (b1 && b2)
        {
            Value *dest = b1->getOperand(0);
            Value *src = b2->getOperand(0);
            dfval.ptrMapSet[dest].clear();
            dfval.ptrMapSet[dest].insert(dfval.ptrMapSet[src].begin(), dfval.ptrMapSet[src].end());

            dfval.ptrMapSetField[dest].clear();
            dfval.ptrMapSetField[dest].insert(dfval.ptrMapSetField[src].begin(), dfval.ptrMapSetField[src].end());
        }
        (*result)[memCpyInst].second = dfval;
    }
   void printCallFuncResult()
    {
        // 每次找到行数最小的，输出对应的结果，然后从call_func_result中删除
        while (!call_func_result.empty())
        {
            int line = call_func_result.begin()->first->getDebugLoc().getLine();
            auto p = call_func_result.begin();
            for (auto ii = call_func_result.begin(), ie = call_func_result.end(); ii != ie; ii++)
            {
                if (ii->first->getDebugLoc().getLine() < line)
                {
                    line = ii->first->getDebugLoc().getLine();
                    p = ii;
                }
            }
            errs() << line << " : ";
            for (auto fi = p->second.begin(), fe = p->second.end(); fi != fe; fi++)
            {
                if (fi != p->second.begin())
                {
                    errs() << ", ";
                }
                errs() << (*fi)->getName();
            }
            errs() << "\n";
            call_func_result.erase(p);
        }
    }
   void merge(FuncPtrInfo * dest, const FuncPtrInfo & src) override {
    //    for (std::map<Value *, std::set<Value *>>::const_iterator ii = src.ptrMapSet.begin(), 
    //         ie = src.ptrMapSet.end(); ii != ie; ++ii) {
    //        // ii->first:key
    //        dest->ptrMapSet[ii->first].insert(ii->second.begin(),(*ii).second.end());   
    //    }
        for (auto ii = src.ptrMapSet.begin(), ie = src.ptrMapSet.end(); ii != ie; ii++)
        {
            dest->ptrMapSet[ii->first].insert(ii->second.begin(), ii->second.end());
        }
        for (auto ii = src.ptrMapSetField.begin(), ie = src.ptrMapSetField.end(); ii != ie; ii++)
        {
            dest->ptrMapSetField[ii->first].insert(ii->second.begin(), ii->second.end());
        }
   }
   void compDFVal(Instruction *inst, DataflowResult<FuncPtrInfo>::Type *result) override{
        // dfval是当前指令inst的入口值
        // isa表示指令以llvm.dbg开头
        if (isa<DbgInfoIntrinsic>(inst)) return;

        // 如果是IntrinsicInst实例,返回true  IntrinsicInst 检查对内部函数调用的包装类
        if (isa<IntrinsicInst>(inst)){
            if (MemCpyInst *memCpyInst = dyn_cast<MemCpyInst>(inst)){
                dealMemCpyInst(memCpyInst, result);
            }else{
                (*result)[inst].second = (*result)[inst].first;
                return;
            }
        }else if (PHINode *phiNode = dyn_cast<PHINode>(inst)){
            dealPhiNode(phiNode,result);
        }else if (CallInst *callInst = dyn_cast<CallInst>(inst)){
            dealCallInst(callInst,result);
        }else if (ReturnInst *retInst = dyn_cast<ReturnInst>(inst)){
            dealRetInst(retInst,result);
        }else if (StoreInst *storeInst = dyn_cast<StoreInst>(inst)){
            dealStoreInst(storeInst,result);
        }else if (LoadInst *loadInst = dyn_cast<LoadInst>(inst)){
            dealLoadInst(loadInst,result);
        }else if (GetElementPtrInst *getElementPtrInst = dyn_cast<GetElementPtrInst>(inst)){
            dealGetElementPtrInst(getElementPtrInst,result);
        }else if (BitCastInst *bitCastInst = dyn_cast<BitCastInst>(inst)){
            dealBitCastInst(bitCastInst,result);
        }else{
            (*result)[inst].second = (*result)[inst].first;
        }
        return;
    //     dfval->ptrMapSet.erase(inst);
    //     for(User::op_iterator oi = inst->op_begin(), oe = inst->op_end();
    //         oi != oe; ++oi) {
    //        Value * val = *oi;
    //     //    if (isa<Instruction>(val)) 
    //         //    dfval->ptrMapSet.insert(cast<Instruction>(val));
    //    }
   }
};

// inline修饰符 表示为内联函数
// inline raw_ostream &operator<<(raw_ostream &out, const LivenessInfo &info) {
//     errs()<<"在raw_ostream\n";
//     for (std::set<Instruction *>::iterator ii=info.LiveVars.begin(), ie=info.LiveVars.end();
//          ii != ie; ++ ii) {
//        const Instruction * inst = *ii;
//        out << inst->getName();
//        out << " ";
//     }
//     return out;
// }

	
// class LivenessVisitor : public DataflowVisitor<struct LivenessInfo> {
// public:
//    LivenessVisitor() {}
//    void merge(LivenessInfo * dest, const LivenessInfo & src) override {
//        for (std::set<Instruction *>::const_iterator ii = src.LiveVars.begin(), 
//             ie = src.LiveVars.end(); ii != ie; ++ii) {
//            dest->LiveVars.insert(*ii);
//        }
//    }

//    void compDFVal(Instruction *inst, LivenessInfo * dfval) override{
//         if (isa<DbgInfoIntrinsic>(inst)) return;
//         dfval->LiveVars.erase(inst);
//         for(User::op_iterator oi = inst->op_begin(), oe = inst->op_end();
//             oi != oe; ++oi) {
//            Value * val = *oi;
//            if (isa<Instruction>(val)) 
//                dfval->LiveVars.insert(cast<Instruction>(val));
//        }
//    }
// };


// class Liveness : public FunctionPass {
// public:

//    static char ID;
//    Liveness() : FunctionPass(ID) {} 
   
//    // 处理每一个函数，在LLVMAssignment的runOnFunction之后
//    bool runOnFunction(Function &F) override {
//     //    F.dump();
//        F.print(llvm::errs(), nullptr);
//        errs()<<"----------------------------------------------------\n";
//        errs()<<"----------------------------------------------------\n";
//        LivenessVisitor visitor;
//        DataflowResult<LivenessInfo>::Type result;
//        LivenessInfo initval;
//        compBackwardDataflow(&F, &visitor, &result, initval);
//        printDataflowResult<LivenessInfo>(errs(), result);
//        return false;
//    }
// };
class FuncPtrPass : public ModulePass {
public:
    DataflowResult<FuncPtrInfo>::Type result;
    std::set<Function *> funcSet;
    static char ID; 
    FuncPtrPass() : ModulePass(ID) {}
    bool runOnModule(Module &M) override {
        for (Function &F : M)
        {
            // 函数名称以llvm.开头
            if (!F.isIntrinsic()){
                funcSet.insert(&F);
            }
        }
        FuncPtrVisitor visitor;
        while (funcSet.size()>0)
        { //遍历每个Function
            FuncPtrInfo initval;

            Function *func = *(funcSet.begin());
            funcSet.erase(funcSet.begin());

            compForwardDataflow(func, &visitor, &result, initval);
            
            funcSet.insert(visitor.fn_worklist.begin(),visitor.fn_worklist.end());//？
            visitor.fn_worklist.clear();
        }
        visitor.printCallFuncResult();
        return false;
    }
};
char FuncPtrPass::ID = 0;