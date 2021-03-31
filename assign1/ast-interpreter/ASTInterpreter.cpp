//==--- tools/clang-check/ClangInterpreter.cpp - Clang Interpreter tool --------------===//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

using namespace std;
using namespace clang;


#include "Environment.h"

class InterpreterVisitor : 
   public EvaluatedExprVisitor<InterpreterVisitor> {
public:
   explicit InterpreterVisitor(const ASTContext &context, Environment * env)
   : EvaluatedExprVisitor(context), mEnv(env) {}
   virtual ~InterpreterVisitor() {}

   virtual void VisitBinaryOperator (BinaryOperator * bop) {
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(bop);
	      mEnv->binop(bop);
      }
   }

   // 变量声明
   virtual void VisitDeclRefExpr(DeclRefExpr * expr) {
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(expr);
	      mEnv->declref(expr);
      }
   }
   virtual void VisitCastExpr(CastExpr * expr) {
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(expr);
	      mEnv->cast(expr);
      }
   }
   virtual void VisitCallExpr(CallExpr * call) {
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(call);  // 继续查看子节点，没有就返回
	      mEnv->call(call);
         // 开始执行自定义函数
         if (FunctionDecl *callee = call->getDirectCallee()){
            auto funcName = callee->getName();
            if(!funcName.equals("PRINT")&&
               !funcName.equals("MALLOC")&&
               !funcName.equals("FREE")&&
               !funcName.equals("GET")){
               Visit(callee->getBody());
               mEnv->customFunction(call);
            }
         }
      }
   }
   virtual void VisitDeclStmt(DeclStmt * declstmt) {
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(declstmt);
         mEnv->decl(declstmt);
      }
   }
   //新加 访问整型值
   virtual void VisitIntegerLiteral(IntegerLiteral * integerLiteral){
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(integerLiteral);
         mEnv->integer(integerLiteral);
      }
   };
   // if
   virtual void VisitIfStmt(IfStmt * ifStmt) {
      if(mEnv->ifReturn()){
         return;
      }else{
         Expr *cond = ifStmt ->getCond();//条件
         if (mEnv->matchType(cond)){
            // 条件true
            Stmt *thenStmt = ifStmt->getThen();
            Visit(thenStmt);
         }else if(ifStmt->getElse()){
            Stmt *elseStmt = ifStmt->getElse();
            Visit(elseStmt);
         }
      }
      
   }
   // return
   virtual void VisitReturnStmt(ReturnStmt *returnStmt)
   {
      if(mEnv->ifReturn()){
         return;
      }else{
         Visit(returnStmt->getRetValue());
         mEnv->returnOp(returnStmt);
      }
   }
   // 处理 a = -10 a = +10 类似
   virtual void VisitUnaryOperator (UnaryOperator * unaryop) {
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(unaryop);
	      mEnv->unaryop(unaryop);
      }
   }
   virtual void CStyleCastExpr(CStyleCastExpr * cStyleCastExpr){
      if(mEnv->ifReturn()){
         return;
      }else{
         VisitStmt(cStyleCastExpr);
	      mEnv->doCStyleCastExpr(cStyleCastExpr);
      }
   }
   virtual void VisitWhileStmt(WhileStmt *whilestmt)
   {
      // clang/AST/stmt.h/ line 1050
      if(mEnv->ifReturn()){
         return;
      }  
      Expr *cond = whilestmt->getCond();
      while (mEnv->matchType(cond))
      {
         Visit(whilestmt->getBody());
         //mEnv->mStack.back().
      }
   }

   virtual void VisitForStmt(ForStmt *forstmt)
   {
      // clang/AST/stmt.h/ line 1179
      if(mEnv->ifReturn()){
         return;
      }  
      Stmt *init = forstmt->getInit();
      if (init)
      {
         Visit(init);
      }
      for (; mEnv->matchType(forstmt->getCond()); Visit(forstmt->getInc()))
      {
         Visit(forstmt->getBody());
      }
   }
private:
   Environment * mEnv;
};

class InterpreterConsumer : public ASTConsumer {
public:
   explicit InterpreterConsumer(const ASTContext& context) : mEnv(),
   	   mVisitor(context, &mEnv) {
   }
   virtual ~InterpreterConsumer() {}

   virtual void HandleTranslationUnit(clang::ASTContext &Context) {
      // TranslationUnitDecl是AST入口节点
	   TranslationUnitDecl * decl = Context.getTranslationUnitDecl();
	   mEnv.init(decl);

	   FunctionDecl * entry = mEnv.getEntry();// 获取主函数
	   mVisitor.VisitStmt(entry->getBody()); 
  }
private:
   Environment mEnv;
   InterpreterVisitor mVisitor;
};
// FrontendAction是一个接口，它允许用户指定的actions作为编译的一部分来执行。
class InterpreterClassAction : public ASTFrontendAction {
public: 
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new InterpreterConsumer(Compiler.getASTContext()));
  }
};

int main (int argc, char ** argv) {
   if (argc > 1) {
       clang::tooling::runToolOnCode(std::unique_ptr<clang::FrontendAction>(new InterpreterClassAction), argv[1]);
   }
}
