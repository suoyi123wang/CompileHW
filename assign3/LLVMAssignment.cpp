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

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>


#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

#include "Liveness.h"
// #include "FuncPtr.h"
using namespace llvm;
static ManagedStatic<LLVMContext> GlobalContext;
static LLVMContext &getGlobalContext() { return *GlobalContext; }

// struct EnableFunctionOptPass : public FunctionPass {
//     static char ID;
//     EnableFunctionOptPass() :FunctionPass(ID) {}
//     bool runOnFunction(Function & F) override {
//         // errs() <<"在FunctionPass:"<<F.getName()<<"\n";
//         if (F.hasFnAttribute(Attribute::OptimizeNone))
//         {
//             F.removeFnAttr(Attribute::OptimizeNone);
//         }
//         return true;
//     }
// };

// char EnableFunctionOptPass::ID = 0;

///!TODO TO BE COMPLETED BY YOU FOR ASSIGNMENT 3
// struct FuncPtrPass : public ModulePass {
//    DataflowResult<FuncPtrInfo>::Type result;
//    std::set<Function *> fn_worklist;

//     static char ID; // Pass identification, replacement for typeid
//     FuncPtrPass() : ModulePass(ID) {}
//     bool runOnModule(Module &M) override {
//        for (auto &F : M)
//         {
//             // 函数名称以llvm.开头
//             if (F.isIntrinsic())
//             {
//                 continue;
//             }
//             else
//             {
//                 errs() << F.getName() << "\n";
//                 fn_worklist.insert(&F);
//             }
//         }

//         FuncPtrVisitor visitor;
//         while (!fn_worklist.empty())
//         { //遍历每个Function
//             FuncPtrInfo initval;
//             Function *func = *(fn_worklist.begin());
//             fn_worklist.erase(fn_worklist.begin());
//             compForwardDataflow(func, &visitor, &result, initval);
//             fn_worklist.insert(visitor.fn_worklist.begin(),visitor.fn_worklist.end());//？
//             visitor.fn_worklist.clear();
//         }
//         visitor.printCallFuncResult();
//         return false;
//     }
// };

// char FuncPtrPass::ID = 0;
// static RegisterPass<FuncPtrPass> X("funcptrpass", "Print function call instruction");

// char Liveness::ID = 0;
// static RegisterPass<Liveness> Y("liveness", "Liveness Dataflow Analysis");

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
// #if LLVM_VERSION_MAJOR == 5
   // runOnFunction
//    Passes.add(new EnableFunctionOptPass());
// #endif
   ///Transform it to SSA
   Passes.add(llvm::createPromoteMemoryToRegisterPass());

   /// Your pass to print Function and Call Instructions
//    Passes.add(new Liveness());
   // runOnModule
   Passes.add(new FuncPtrPass());
   Passes.run(*M.get());
#ifndef NDEBUG    
   system("pause");
#endif
}
