// //===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
// //
// //                     The LLVM Compiler Infrastructure
// //
// // This file is distributed under the University of Illinois Open Source
// // License. See LICENSE.TXT for details.
// //
// //===----------------------------------------------------------------------===//
// //
// // This file implements two versions of the LLVM "Hello World" pass described
// // in docs/WritingAnLLVMPass.html
// //
// //===----------------------------------------------------------------------===//

// #include <llvm/Support/CommandLine.h>
// #include <llvm/IRReader/IRReader.h>
// #include <llvm/IR/LLVMContext.h>
// #include <llvm/Support/SourceMgr.h>
// #include <llvm/IR/LegacyPassManager.h>
// #include <llvm/Support/ToolOutputFile.h>

// #include <llvm/Transforms/Scalar.h>
// #include <llvm/Transforms/Utils.h>

// #include <llvm/IR/Function.h>
// #include <llvm/Pass.h>
// #include <llvm/Support/raw_ostream.h>

// #include <llvm/Bitcode/BitcodeReader.h>
// #include <llvm/Bitcode/BitcodeWriter.h>

// #include <llvm/IR/IntrinsicInst.h>
// #include <map>
// #include <set>
// using namespace llvm;
// using namespace std;

// static ManagedStatic<LLVMContext> GlobalContext;
// static LLVMContext &getGlobalContext() { return *GlobalContext; }

// // key为左侧标识符，value可以多个可能引用的函数
// std::multimap<string, string> phiMap;

// //  key为行号，value为可能调用的函数。最后按照行号排列
// std::multimap<int, string> resultMap;

// // value为行号： key为 phi 的左侧标识符 或者还不能确定调用谁的函数名称
// std::map<string , int> lineMap; 

// // std::set<string> resultSet;
// /* In LLVM 5.0, when  -O0 passed to clang , the functions generated with clang will
//  * have optnone attribute which would lead to some transform passes disabled, like mem2reg.
//  */
// // 每次处理一个函数
// struct EnableFunctionOptPass: public FunctionPass {
//     static char ID;
//     EnableFunctionOptPass():FunctionPass(ID){}
//     // 以函数为单位进行处理
//     bool runOnFunction(Function & F) override{
//       if(F.hasFnAttribute(Attribute::OptimizeNone))
//       {
//           F.removeFnAttr(Attribute::OptimizeNone);
//       }
//       return true;
//     }
// };

// char EnableFunctionOptPass::ID=0;
// // 使用ID地址来识别Pass
	
// ///!TODO TO BE COMPLETED BY YOU FOR ASSIGNMENT 2
// ///Updated 11/10/2017 by fargo: make all functions
// ///processed by mem2reg before this pass.
// // 将整个程序当做一个单元，可以随意引用函数主体，无法预知子类行为，不能优化
// struct FuncPtrPass : public ModulePass {
//   static char ID; // Pass identification, replacement for typeid
//   FuncPtrPass() : ModulePass(ID) {}
  
//   bool runOnModule(Module &M) override {
//     // M.dump(); // 转储
//     M.print(llvm::errs(), nullptr);
//     errs()<<"------------------------------\n";
//     for(Function& func : M){
//       for(BasicBlock& bb : func){
//         for(Instruction& i : bb){
//           // Instruction 可以获得 Value, Function, ReturnInst
//           if (PHINode *phiNode = dyn_cast<PHINode>(&i)){
//             for(int i = 0; i < phiNode->getNumOperands(); ++i){
//               dealPhi(phiNode->getName(),phiNode->getIncomingValue(i)->getName());
//               // if(phiNode->getIncomingValue(i)->getName()!=""){
//               //   // phi的可能值有可能还是标识符，需要再次转换
//               //   // 若在phimap里面没有，就不需要再次转换
//               //   string phiValue = phiNode->getIncomingValue(i)->getName();
//               //   if(phiMap.count(phiValue)==0){
//               //     phiMap.insert(std::make_pair(phiNode->getName(), phiValue));
//               //   }else{
//               //     std::multimap<string,string>::iterator phiIter;
//               //     phiIter = phiMap.find(phiValue);
//               //     for(int k = 0;k < phiMap.count(phiValue);k++,phiIter++){
//               //       phiMap.insert(std::make_pair(phiNode->getName(), phiIter->second));
//               //     }
//               //   }
//               // }
//             }
//           }
//           if (CallInst *inst = dyn_cast<CallInst>(&i)){
//             Function* called = inst->getCalledFunction();
//             // 间接调用返回null
//             if (called==NULL) {
//                 Value* v=inst->getCalledValue();
//                 Value* sv = v->stripPointerCasts();
//                 StringRef fname = sv->getName();
//                 if(fname == ""){
//                   lineMap.insert(std::make_pair(func.getName(),inst->getDebugLoc().getLine()));
//                   continue;
//                 }

//                 generateResult(inst->getDebugLoc().getLine(),fname);
//             }
//             if (called && called->getName()!="llvm.dbg.value"){
//               resultMap.insert(std::make_pair(inst->getDebugLoc().getLine(),called->getName()));

//               for(unsigned i = 0;i<inst->getNumArgOperands();i++){
//                 auto arg = inst->getArgOperand(i);
//                 // 调用的函数参数里面是指针函数
//                 if(PointerType *pointerParam = dyn_cast<PointerType>(arg->getType())){
//                   // 首先获取对应的主调函数
//                   int line;
//                   map<string,int>::iterator lineIter=lineMap.find(called->getName());
//                   if(lineIter != lineMap.end()){
//                     line = lineIter->second;
//                   }else{
//                     errs()<<i<<"::::error1111"<< arg->getName() <<"\n";
//                   }
//                   errs()<< called->getName()<<arg->getName()<<"\n";
//                   // for(int i = 0; i < phiMap.count(arg->getName());++i){
//                   //   std::multimap<string,string>::iterator phiIter;
//                   //   phiIter = phiMap.find(arg->getName());
//                   //   for(int k = 0;k < phiMap.count(arg->getName());k++,phiIter++){
//                   //     resultMap.insert(std::make_pair(line,phiIter->second));
//                   //   }
//                   // }
//                   // if lineMap.find(arg->getName())
//                   // lineMap.insert(std::make_pair(arg->getName(),line));
//                   generateResult(line, arg->getName());
//                 }
//               }
//             }

//           }
//         }
//       }
//     }
//     // 如果模块被转换修改它将返回true，否则应该返回false。
//     // errs() << "最终的结果:\n";
//     multimap <int,string>::iterator resultIter;
//               for ( resultIter = resultMap.begin( );resultIter != resultMap.end( ); resultIter++ ){
//                 errs()<<resultIter->first<<":::"<<resultIter->second<<"\n";
//               }
//     return false;
//   }

//   // 自己添加
//   void dealPhi(string phiName, string phiValue){
//     if(phiValue!=""){
//       // phi的可能值有可能还是标识符，需要再次转换
//       // 若在phimap里面没有，就不需要再次转换
//       // string phiValue = phiNode->getIncomingValue(i)->getName();
//       if(phiMap.count(phiValue)==0){
//         phiMap.insert(std::make_pair(phiName, phiValue));
//       }else{
//         std::multimap<string,string>::iterator phiIter;
//         phiIter = phiMap.find(phiValue);
//         for(int k = 0;k < phiMap.count(phiValue);k++,phiIter++){
//           dealPhi(phiName,phiIter->second);
//           // phiMap.insert(std::make_pair(phiName, phiIter->second));
//         }
//       }
//     }
//   }


//   void generateResult(int line , string fname){
//     std::multimap<string,string>::iterator phiIter;
//     phiIter = phiMap.find(fname);
//     for(int k = 0;k < phiMap.count(fname);k++,phiIter++){
//       resultMap.insert(std::make_pair(line,phiIter->second));
//     }
//   }

// };
// // 

// char FuncPtrPass::ID = 0;


// static RegisterPass<FuncPtrPass> X("funcptrpass", "Print function call instruction");

// // 解析bc文件 -> 遍历module -> 拿到函数列表 -> 遍历函数迭代器 -> 输出
// static cl::opt<std::string>
// InputFilename(cl::Positional,
//               cl::desc("<filename>.bc"),
//               cl::init(""));


// int main(int argc, char **argv) {
//    LLVMContext &Context = getGlobalContext();
//    SMDiagnostic Err;
//    // Parse the command line to read the Inputfilename
//    cl::ParseCommandLineOptions(argc, argv,
//                               "FuncPtrPass \n My first LLVM too which does not do much.\n");


//    // Load the input module
//    std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
//    if (!M) {
//       Err.print(argv[0], errs());
//       return 1;
//    }

//    llvm::legacy::PassManager Passes;
   	
//    ///Remove functions' optnone attribute in LLVM5.0
//    Passes.add(new EnableFunctionOptPass());
//    ///Transform it to SSA
//    Passes.add(llvm::createPromoteMemoryToRegisterPass());

//    /// Your pass to print Function and Call Instructions
//    Passes.add(new FuncPtrPass());
//    Passes.run(*M.get());
// }



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

// #include <llvm/Support/CommandLine.h>
// #include <llvm/IRReader/IRReader.h>
// #include <llvm/IR/LLVMContext.h>
// #include <llvm/Support/SourceMgr.h>
// #include <llvm/IR/LegacyPassManager.h>
// #include <llvm/Support/ToolOutputFile.h>

// #include <llvm/Transforms/Scalar.h>
// #include <llvm/Transforms/Utils.h>

// #include <llvm/IR/Function.h>
// #include <llvm/Pass.h>
// #include <llvm/Support/raw_ostream.h>

// #include <llvm/Bitcode/BitcodeReader.h>
// #include <llvm/Bitcode/BitcodeWriter.h>

// #include <llvm/IR/IntrinsicInst.h>
// #include <map>
// #include <set>
// using namespace llvm;
// using namespace std;

// static ManagedStatic<LLVMContext> GlobalContext;
// static LLVMContext &getGlobalContext() { return *GlobalContext; }

// // key为左侧标识符，value可以多个可能引用的函数
// std::multimap<string, string> phiMap;

// //  key为行号，value为可能调用的函数。最后按照行号排列
// std::multimap<int, string> resultMap;

// // value为行号： key为 phi 的左侧标识符 或者还不能确定调用谁的函数名称
// std::map<string , int> lineMap; 

// // std::set<string> resultSet;
// /* In LLVM 5.0, when  -O0 passed to clang , the functions generated with clang will
//  * have optnone attribute which would lead to some transform passes disabled, like mem2reg.
//  */
// // 每次处理一个函数
// struct EnableFunctionOptPass: public FunctionPass {
//     static char ID;
//     EnableFunctionOptPass():FunctionPass(ID){}
//     // 以函数为单位进行处理
//     bool runOnFunction(Function & F) override{
//       if(F.hasFnAttribute(Attribute::OptimizeNone))
//       {
//           F.removeFnAttr(Attribute::OptimizeNone);
//       }
//       return true;
//     }
// };

// char EnableFunctionOptPass::ID=0;
// // 使用ID地址来识别Pass
	
// ///!TODO TO BE COMPLETED BY YOU FOR ASSIGNMENT 2
// ///Updated 11/10/2017 by fargo: make all functions
// ///processed by mem2reg before this pass.
// // 将整个程序当做一个单元，可以随意引用函数主体，无法预知子类行为，不能优化
// struct FuncPtrPass : public ModulePass {
//   static char ID; // Pass identification, replacement for typeid
//   FuncPtrPass() : ModulePass(ID) {}
  
//   bool runOnModule(Module &M) override {
//     // M.dump(); // 转储
//     M.print(llvm::errs(), nullptr);
//     errs()<<"------------------------------\n";
//     for(Function& func : M){
//       for(BasicBlock& bb : func){
//         for(Instruction& i : bb){
//           // Instruction 可以获得 Value, Function, ReturnInst
//           if (PHINode *phiNode = dyn_cast<PHINode>(&i)){
//             for(int i = 0; i < phiNode->getNumOperands(); ++i){
//               dealPhi(phiNode->getName(),phiNode->getIncomingValue(i)->getName());
//             }
//           }
//           if (CallInst *inst = dyn_cast<CallInst>(&i)){
//             Function* called = inst->getCalledFunction();
//             // 间接调用返回null
//             if (called==NULL) {
              
//                 Value* v=inst->getCalledValue();
//                 Value* sv = v->stripPointerCasts();
//                 // 判断callValue的类型
                

//                 StringRef fname = sv->getName();
//                 lineMap.insert(std::make_pair(func.getName(),inst->getDebugLoc().getLine()));
//                 if(fname == ""){
//                   continue;
//                 }
//                 generateResult(inst->getDebugLoc().getLine(),fname);
//             }else if (!called->isIntrinsic()){
//               // 间接调用
//               phiMap.insert(std::make_pair(func.getName(),called->getName()));
//               lineMap.insert(std::make_pair(func.getName(),inst->getDebugLoc().getLine()));
//               resultMap.insert(std::make_pair(inst->getDebugLoc().getLine(),called->getName()));

//               for(unsigned i = 0;i<inst->getNumArgOperands();i++){
//                 auto arg = inst->getArgOperand(i);
//                 // 调用的函数参数里面是指针函数
//                 if(PointerType *pointerParam = dyn_cast<PointerType>(arg->getType())){
//                   // 首先获取对应的主调函数
//                   dealPointerFunction(called->getName(),arg->getName());
//                 }
//               }
//             }
          
          
//           }

//           // if (ReturnInst * ret = dyn_cast<ReturnInst>(&i)) {
//           //   Value * retvalue = ret->getReturnValue();
//           //   if (CallInst * retcall = dyn_cast<CallInst>(retvalue)) {
//           //     Value * value = retcall->getOperand(argindex);
//           //     chargeValue(value);
//           //   }
//           // }
        
        
//         }
//       }
//     }
//     // 如果模块被转换修改它将返回true，否则应该返回false。
//     // errs() << "最终的结果:\n";
//     multimap <int,string>::iterator resultIter;
//               for ( resultIter = resultMap.begin( );resultIter != resultMap.end( ); resultIter++ ){
//                 errs()<<resultIter->first<<":::"<<resultIter->second<<"\n";
//               }
//     return false;
//   }

//   // 自己添加
//   void dealPhi(string phiName, string phiValue){
//     if(phiValue!=""){
//       // phi的可能值有可能还是标识符，需要再次转换
//       // 若在phimap里面没有，就不需要再次转换
//       // string phiValue = phiNode->getIncomingValue(i)->getName();
//       if(phiMap.count(phiValue)==0){
//         phiMap.insert(std::make_pair(phiName, phiValue));
//       }else{
//         std::multimap<string,string>::iterator phiIter;
//         phiIter = phiMap.find(phiValue);
//         for(int k = 0;k < phiMap.count(phiValue);k++,phiIter++){
//           dealPhi(phiName,phiIter->second);
//           // phiMap.insert(std::make_pair(phiName, phiIter->second));
//         }
//       }
//     }
//   }
//   void generateResult(int line , string fname){
//     std::multimap<string,string>::iterator phiIter;
//     phiIter = phiMap.find(fname);
//     for(int k = 0;k < phiMap.count(fname);k++,phiIter++){
//       resultMap.insert(std::make_pair(line,phiIter->second));
//     }
//   }

//   void dealPointerFunction(string calledName, string value){
//     while(phiMap.count(calledName)!=0){
//       std::multimap<string,string>::iterator phiIter;
//       phiIter = phiMap.find(calledName);
//       for(int k = 0;k < phiMap.count(calledName);k++,phiIter++){
//         calledName = phiIter->second;
//       }
//     }

//     int line;
//     map<string,int>::iterator lineIter=lineMap.find(calledName);
//     if(lineIter != lineMap.end()){
//       line = lineIter->second;
//     }else{
//       errs()<<"::::error1111\n";
//     }
//     generateResult(line, value);
//   }



//   // void getPHINode(PHINode * phi) {
//   //   for (Value * incoming : phi->incoming_values())
//   //     chargeValue(incoming);
//   // }

//   // void getArgument(Argument * arg) {
//   //   unsigned argindex = arg->getArgNo();
//   //   Function * parentfunc = arg->getParent();
//   //   for (User * users : parentfunc->users()) {
//   //     if (CallInst * callinst = dyn_cast<CallInst>(users)) {
//   //       Value * callvalue = callinst->getOperand(argindex);
//   //       if (callinst->getCalledFunction() != parentfunc) {
//   //         Function * callfunc = callinst->getCalledFunction();
//   //         for (Function::iterator ft = callfunc->begin(), fe = callfunc->end(); ft != fe; ++ft) {
//   //           for (BasicBlock::iterator bt = ft->begin(), be = ft->end(); bt != be; ++bt) {
//   //             Instruction * instruction = dyn_cast<Instruction>(bt);
//   //             if (ReturnInst * ret = dyn_cast<ReturnInst>(instruction)) {
//   //               Value * retvalue = ret->getReturnValue();
//   //               if (CallInst * retcall = dyn_cast<CallInst>(retvalue)) {
//   //                 Value * value = retcall->getOperand(argindex);
//   //                 chargeValue(value);
//   //               }
//   //             }
//   //           }
//   //         }
//   //       }
//   //       else
//   //         chargeValue(callvalue);
//   //     }
//   //     else if (PHINode * phi = dyn_cast<PHINode>(users)) {
//   //       for (User * phiuser : phi->users()) {
//   //         if (CallInst * phicall = dyn_cast<CallInst>(phiuser)) {
//   //           Value * value = phicall->getOperand(argindex);
//   //           if (Function * func = dyn_cast<Function>(value))
//   //             if (find(mFuncName.begin(), mFuncName.end(), func->getName()) == mFuncName.end())
//   //               mFuncName.push_back(func->getName());
//   //         }
//   //       }
//   //     }
//   //   }
//   // }
  
//   // void chargeValue(Value * value) {
//   //   if (Function * func = dyn_cast<Function>(value)) {
//   //     if (find(mFuncName.begin(), mFuncName.end(), func->getName()) == mFuncName.end())
//   //       mFuncName.push_back(func->getName());
//   //   } 
//   //   else if (PHINode * phi = dyn_cast<PHINode>(value))
//   //     getPHINode(phi);
//   //   else if (Argument * arg = dyn_cast<Argument>(value))
//   //     getArgument(arg);
//   // }

// };
// // 

// char FuncPtrPass::ID = 0;


// static RegisterPass<FuncPtrPass> X("funcptrpass", "Print function call instruction");

// // 解析bc文件 -> 遍历module -> 拿到函数列表 -> 遍历函数迭代器 -> 输出
// static cl::opt<std::string>
// InputFilename(cl::Positional,
//               cl::desc("<filename>.bc"),
//               cl::init(""));


// int main(int argc, char **argv) {
//    LLVMContext &Context = getGlobalContext();
//    SMDiagnostic Err;
//    // Parse the command line to read the Inputfilename
//    cl::ParseCommandLineOptions(argc, argv,
//                               "FuncPtrPass \n My first LLVM too which does not do much.\n");


//    // Load the input module
//    std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
//    if (!M) {
//       Err.print(argv[0], errs());
//       return 1;
//    }

//    llvm::legacy::PassManager Passes;
   	
//    ///Remove functions' optnone attribute in LLVM5.0
//    Passes.add(new EnableFunctionOptPass());
//    ///Transform it to SSA
//    Passes.add(llvm::createPromoteMemoryToRegisterPass());

//    /// Your pass to print Function and Call Instructions
//    Passes.add(new FuncPtrPass());
//    Passes.run(*M.get());
// }







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

#include <llvm/Support/CommandLine.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/ToolOutputFile.h>

#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>

// add
#include <llvm/IR/Use.h>
#include <llvm/Pass.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DebugLoc.h>



using namespace llvm;
static ManagedStatic<LLVMContext> GlobalContext;
static LLVMContext &getGlobalContext() { return *GlobalContext; }
/* In LLVM 5.0, when  -O0 passed to clang , the functions generated with clang will
 * have optnone attribute which would lead to some transform passes disabled, like mem2reg.
 */
struct EnableFunctionOptPass: public FunctionPass {
    static char ID;
    EnableFunctionOptPass():FunctionPass(ID){}
    bool runOnFunction(Function & F) override{
        if(F.hasFnAttribute(Attribute::OptimizeNone))
        {
            F.removeFnAttr(Attribute::OptimizeNone);
        }
        return true;
    }
};

char EnableFunctionOptPass::ID=0;

	
///!TODO TO BE COMPLETED BY YOU FOR ASSIGNMENT 2
///Updated 11/10/2017 by fargo: make all functions
///processed by mem2reg before this pass.
struct FuncPtrPass : public ModulePass {
  static char ID; // Pass identification, replacement for typeid
  FuncPtrPass() : ModulePass(ID) {}

  // line, function names
  std::map<int, std::vector<StringRef>> mCall;

  // called function names
  std::vector<StringRef> mFuncName;

  
  bool runOnModule(Module &M) override {
    M.print(llvm::errs(), nullptr);
    errs()<<"------------------------------\n";
    // for functions
    for (Module::iterator it = M.begin(), ie = M.end(); it != ie; ++it) {
      // for basic_blocks
      for (Function::iterator ft = it->begin(), fe = it->end(); ft != fe; ++ft) {
        // for instructions
        for (BasicBlock::iterator bt = ft->begin(), be = ft->end(); bt != be; ++bt) {
          Instruction * instruction = dyn_cast<Instruction>(bt);
          // if instruction is a call instruction
          if (isa<CallInst>(bt)) {
            CallInst * callinst = dyn_cast<CallInst>(instruction);
            // get called function
            Function * callfunc = callinst->getCalledFunction();
            // get called line
            int line = callinst->getDebugLoc().getLine();                        
            mFuncName.clear();

            // indirection function invocation
            if (!callfunc) {
              Value * callvalue = callinst->getCalledValue();
              // match type
              if (CallInst * call = dyn_cast<CallInst>(callvalue))
                getCallInst(call);
              else if (PHINode * phi = dyn_cast<PHINode>(callvalue))
                getPHINode(phi);
              else if (Argument * arg = dyn_cast<Argument>(callvalue))
                getArgument(arg);
              mCall.insert(std::pair<int, std::vector<StringRef>>(line, mFuncName));
            }

            // if the function's name starts with "llvm."
            else if (callfunc->isIntrinsic())
              continue;

            // called function exist
            else {
              // duplicate checking & function name adding
              if (find(mFuncName.begin(), mFuncName.end(), callfunc->getName()) == mFuncName.end())
                mFuncName.push_back(callfunc->getName());
              // duplicate checking
              if (mCall.find(line) == mCall.end())
                mCall.insert(std::pair<int, std::vector<StringRef>>(line, mFuncName));
              else
                mCall.find(line)->second.push_back(callfunc->getName());
            }
          }
        }
      }
    }    
    resultPrint();
    return false;
  }

  void getCallInst(CallInst * callinst) {
    Function * callfunc = callinst->getCalledFunction();
    if (callfunc)
      getCallFunc(callfunc);  // 直接调用
    else {
      Value * callvalue = callinst->getCalledValue();// 间接调用
      // match PHI node
      if (PHINode * phi = dyn_cast<PHINode>(callvalue))
        for (Value * incoming : phi->incoming_values())
          if (Function * phifunc = dyn_cast<Function>(incoming))
            getCallFunc(phifunc);
    }
  }

  void getCallFunc(Function * callfunc) {
    for (Function::iterator ft = callfunc->begin(), fe = callfunc->end(); ft != fe; ++ft) {
      for (BasicBlock::iterator bt = ft->begin(), be = ft->end(); bt != be; ++bt) {
        Instruction * instruction = dyn_cast<Instruction>(bt);
        // match return value
        if (ReturnInst * ret = dyn_cast<ReturnInst>(instruction)) {
          Value * retvalue = ret->getReturnValue();
          if (CallInst * callinst = dyn_cast<CallInst>(retvalue))
            getCallInst(callinst);
          else if (PHINode * phi = dyn_cast<PHINode>(retvalue))
            getPHINode(phi);
          else if (Argument * arg = dyn_cast<Argument>(retvalue))
            getArgument(arg);
        }
      }
    }
  }

  void getPHINode(PHINode * phi) {
    for (Value * incoming : phi->incoming_values())
      chargeValue(incoming);
  }

  void getArgument(Argument * arg) {
    
    unsigned argindex = arg->getArgNo();
    Function * parentfunc = arg->getParent();
    for (User * users : parentfunc->users()) {
      if (CallInst * callinst = dyn_cast<CallInst>(users)) {
        Value * callvalue = callinst->getOperand(argindex);
        if (callinst->getCalledFunction() != parentfunc) {
          Function * callfunc = callinst->getCalledFunction();
          for (Function::iterator ft = callfunc->begin(), fe = callfunc->end(); ft != fe; ++ft) {
            for (BasicBlock::iterator bt = ft->begin(), be = ft->end(); bt != be; ++bt) {
              Instruction * instruction = dyn_cast<Instruction>(bt);
              if (ReturnInst * ret = dyn_cast<ReturnInst>(instruction)) {
                Value * retvalue = ret->getReturnValue();
                if (CallInst * retcall = dyn_cast<CallInst>(retvalue)) {
                  Value * value = retcall->getOperand(argindex);
                  chargeValue(value);
                }
              }
            }
          }
        }
        else
          chargeValue(callvalue);
      }
      else if (PHINode * phi = dyn_cast<PHINode>(users)) {
        for (User * phiuser : phi->users()) {
          if (CallInst * phicall = dyn_cast<CallInst>(phiuser)) {
            Value * value = phicall->getOperand(argindex);
            if (Function * func = dyn_cast<Function>(value))
              if (find(mFuncName.begin(), mFuncName.end(), func->getName()) == mFuncName.end())
                mFuncName.push_back(func->getName());
          }
        }
      }
    }
  }
  
  void chargeValue(Value * value) {
    if (Function * func = dyn_cast<Function>(value)) {
      if (find(mFuncName.begin(), mFuncName.end(), func->getName()) == mFuncName.end())
        mFuncName.push_back(func->getName());
    } 
    else if (PHINode * phi = dyn_cast<PHINode>(value))
      getPHINode(phi);
    else if (Argument * arg = dyn_cast<Argument>(value))
      getArgument(arg);
  }
  
  void resultPrint() {
    for (std::map<int, std::vector<StringRef>>::iterator it = mCall.begin(), ie = mCall.end(); it != ie; ++it) {
      if (!it->second.empty()) {
        errs() << it->first << " : ";
        auto st = it->second.begin();
        auto se = it->second.end() - 1;
        for (; st != se; ++st)
          errs() << *st << ", ";
        errs() << *se << "\n";
      }
    }
  }
};


char FuncPtrPass::ID = 0;
static RegisterPass<FuncPtrPass> X("funcptrpass", "Print function call instruction");

static cl::opt<std::string>
InputFilename(cl::Positional,
              cl::desc("<filename>.bc"),
              cl::init(""));


int main(int argc, char **argv) {
   LLVMContext &Context = getGlobalContext();
   SMDiagnostic Err;
   // Parse the command line to read the Inputfilename
   cl::ParseCommandLineOptions(argc, argv,
                              "FuncPtrPass \n My first LLVM too which does not do much.\n");


   // Load the input module
   std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
   if (!M) {
      Err.print(argv[0], errs());
      return 1;
   }

   llvm::legacy::PassManager Passes;
   	
   ///Remove functions' optnone attribute in LLVM5.0
   Passes.add(new EnableFunctionOptPass());
   ///Transform it to SSA
   Passes.add(llvm::createPromoteMemoryToRegisterPass());

   /// Your pass to print Function and Call Instructions
   Passes.add(new FuncPtrPass());
   Passes.run(*M.get());
}

