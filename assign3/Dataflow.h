/************************************************************************
 *
 * @file Dataflow.h
 *
 * General dataflow framework
 *
 ***********************************************************************/

#ifndef _DATAFLOW_H_
#define _DATAFLOW_H_

#include <llvm/Support/raw_ostream.h>
#include <map>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Function.h>

using namespace llvm;

///
/// Dummy class to provide a typedef for the detailed result set
/// For each basicblock, we compute its input dataflow val and its output dataflow val
///
template<class T>
struct DataflowResult {
    // first T:inValue   second T:outValue
    typedef typename std::map<Instruction *, std::pair<T, T> > Type;
};

///Base dataflow visitor class, defines the dataflow function

template <class T>
class DataflowVisitor {
public:
    virtual ~DataflowVisitor() { }

    /// Dataflow Function invoked for each basic block 
    /// 
    /// @block the Basic Block
    /// @dfval the input dataflow value
    /// @isforward true to compute dfval forward, otherwise backward
    virtual void compDFVal(BasicBlock *block, typename DataflowResult<T>::Type *result, bool isforward) {
        if (isforward) {// forward
           for (BasicBlock::iterator ii=block->begin(), ie=block->end(); ii!=ie; ++ii) {
                Instruction * inst = &*ii;
                compDFVal(inst, result);
                // ?为什么放在这里
                if (Instruction *next_inst = inst->getNextNode())
                {
                    (*result)[next_inst].first = (*result)[inst].second;
                }
           }
        } else { //backward
           for (BasicBlock::reverse_iterator ii=block->rbegin(), ie=block->rend();ii != ie; ++ii) {
                Instruction * inst = &*ii;
                compDFVal(inst, result);
           }
        }
    }

    ///
    /// Dataflow Function invoked for each instruction
    ///
    /// @inst the Instruction
    /// @dfval the input dataflow value
    /// @return true if dfval changed
    virtual void compDFVal(Instruction *inst, typename DataflowResult<T>::Type *result ) = 0;

    ///
    /// Merge of two dfvals, dest will be ther merged result
    /// @return true if dest changed
    ///
    virtual void merge( T *dest, const T &src ) = 0;
};



/// 
/// Compute a forward iterated fixedpoint dataflow function, using a user-supplied
/// visitor function. Note that the caller must ensure that the function is
/// in fact a monotone function, as otherwise the fixedpoint may not terminate.
/// 单调函数
/// @param fn The function
/// @param visitor A function to compute dataflow vals
/// @param result The results of the dataflow 
/// @initval the Initial dataflow value
template<class T>
void compForwardDataflow(Function *fn,
                         DataflowVisitor<T> *visitor,
                         typename DataflowResult<T>::Type *result,
                         const T & initval) {
    
    // std::set<BasicBlock *> basicBlockSet;

    // for(BasicBlock& basicBlock : *fn){
    //     for(Instruction& instruction : basicBlock){
    //         result->insert(std::make_pair(&instruction, std::make_pair(initval, initval)));
    //     }
    //     basicBlockSet.insert(&basicBlock);
    // }

    // while(basicBlockSet.size() > 0){
    //     BasicBlock* bb = *basicBlockSet.begin();
    //     basicBlockSet.erase(basicBlockSet.begin());

    //     Instruction *fInst = dyn_cast<Instruction>(bb->begin());
    //     Instruction *lInst = dyn_cast<Instruction>(--bb->end());

    //     // all outValue
    //     T value = (*result)[fInst].first;
    //     // pred_begin(): 前节点
    //     // 当前节点的inVal是前一节点的outVal
    //     for (auto preB = pred_begin(bb), preE = pred_end(bb); preB != preE; preB++) {
    //         BasicBlock *pred = *preB;
    //         // 前block的最后一个指令
    //         Instruction *plInst = dyn_cast<Instruction>(--pred->end());
    //         // 前节点的outValue和当前节点的inValue merge,value改变
    //         visitor->merge(&value, (*result)[lInst].second);
    //     }
    //     //更新当前节点的inval
    //     (*result)[fInst].first = value;
    //     T outValue = (*result)[lInst].second;

    //     visitor->compDFVal(bb, result, true);

    //     // If ingoing value changed, propagate it along the CFG
    //     if (outValue == (*result)[lInst].second) continue;
    //     // succ_begin后继节点
    //     for (succ_iterator si = succ_begin(bb), se = succ_begin(bb); si != se; si++) {
    //         basicBlockSet.insert(*si);
    //     }
    // }

    errs()<<"Function：————————————————————————————————\n";
    fn->print(llvm::errs(), nullptr);
    errs()<<"Function end————————————————————————————————\n";
    std::set<BasicBlock *> bb_worklist;
    for (Function::iterator bi = fn->begin(), be = fn->end(); bi != be; bi++)
    {
        BasicBlock *bb = dyn_cast<BasicBlock>(bi);
        for (auto ii = bb->begin(), ie = bb->end(); ii != ie; ii++)
        {
            auto i = dyn_cast<Instruction>(ii);
            result->insert(std::make_pair(i, std::make_pair(initval, initval)));
        }
        bb_worklist.insert(bb);
    }
    // LivenessInfo initval;
    while (!bb_worklist.empty())
    { // 遍历每个BasicBlock
        BasicBlock *bb = *bb_worklist.begin();
        bb_worklist.erase(bb_worklist.begin());

        Instruction *bb_first_inst = dyn_cast<Instruction>(bb->begin());
        Instruction *bb_last_inst = dyn_cast<Instruction>(--bb->end());
        T bbinval = (*result)[bb_first_inst].first; // std::make_pair(initval, initval)[0]

        for (auto pi = pred_begin(bb), pe = pred_end(bb); pi != pe; pi++)
        {
            BasicBlock *pred = *pi;
            Instruction *pred_last_inst = dyn_cast<Instruction>(--pred->end());
            visitor->merge(&bbinval, (*result)[pred_last_inst].second); //std::make_pair(initval, initval)[1]
        }

        (*result)[bb_first_inst].first = bbinval;
        T bb_outval = (*result)[bb_last_inst].second;
        visitor->compDFVal(bb, result, true);// 把每个块的入口和出口写好
        if(bb_outval==(*result)[bb_last_inst].second){
            continue;
        }else{
            for(auto bi=succ_begin(bb),be=succ_end(bb);bi!=be;bi++){
                bb_worklist.insert(*bi);
            }
        }

    }    
    return;
}

/// Compute a backward iterated fixedpoint dataflow function, using a user-supplied
/// visitor function. Note that the caller must ensure that the function is
/// in fact a monotone function, as otherwise the fixedpoint may not terminate.
/// 
/// @param fn The function
/// @param visitor A function to compute dataflow vals
/// @param result The results of the dataflow 
/// @initval The initial dataflow value
// template<class T>
// void compBackwardDataflow(Function *fn,
//     DataflowVisitor<T> *visitor,
//     typename DataflowResult<T>::Type *result,
//     const T &initval) {

//     std::set<BasicBlock *> worklist;

//     // Initialize the worklist with all exit blocks

//     for (Function::iterator bi = fn->begin(); bi != fn->end(); ++bi) {
//         BasicBlock * bb = &*bi;
//         result->insert(std::make_pair(bb, std::make_pair(initval, initval)));
//         worklist.insert(bb);
//     }

//     // Iteratively compute the dataflow result
//     while (!worklist.empty()) {
//         BasicBlock *bb = *worklist.begin();
//         worklist.erase(worklist.begin());// 删除元素

//         // Merge all incoming value
//         T bbexitval = (*result)[bb].second;
//         for (auto si = succ_begin(bb), se = succ_end(bb); si != se; si++) {
//             BasicBlock *succ = *si;
//             visitor->merge(&bbexitval, (*result)[succ].first);
//         }

//         (*result)[bb].second = bbexitval;
//         visitor->compDFVal(bb, &bbexitval, false);

//         // If outgoing value changed, propagate it along the CFG
//         if (bbexitval == (*result)[bb].first) continue;
//         (*result)[bb].first = bbexitval;

//         for (pred_iterator pi = pred_begin(bb), pe = pred_end(bb); pi != pe; pi++) {
//             worklist.insert(*pi);
//         }
//     }
// }

// template<class T>
// void printDataflowResult(raw_ostream &out,
//                          const typename DataflowResult<T>::Type &dfresult) {
//     for ( typename DataflowResult<T>::Type::const_iterator it = dfresult.begin();
//             it != dfresult.end(); ++it ) {
//         if (it->first == NULL) out << "*";
//         // else it->first->dump();
//         out << "\n\tin : "
//             << it->second.first 
//             << "\n\tout :  "
//             << it->second.second
//             << "\n";
//     }
// }
#endif /* !_DATAFLOW_H_ */

