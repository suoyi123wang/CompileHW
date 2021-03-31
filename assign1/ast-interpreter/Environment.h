//==--- tools/clang-check/ClangInterpreter.cpp - Clang Interpreter tool --------------===//
//===----------------------------------------------------------------------===//
#include <stdio.h>
#include <iostream>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
using namespace std;
class StackFrame
{
	/// StackFrame maps Variable Declaration to Value
	/// Which are either integer or addresses (also represented using an Integer value)
	std::map<Decl *, int64_t> mVars;
	std::map<Stmt *, int64_t> mExprs;
	/// The current stmt
	Stmt *mPC;
	// 新加
	// 只考虑int(1) 和 void(0)
	bool retType = 0;
	int64_t retValue = 0;

public:
	StackFrame() : mVars(), mExprs(), mPC()
	{
	}
	void bindDecl(Decl *decl, int64_t val){
		mVars[decl] = val;
	}
	int64_t getDeclVal(Decl *decl){
		assert(mVars.find(decl) != mVars.end());
		return mVars.find(decl)->second;
	}
	void bindStmt(Stmt *stmt, int64_t val){
		mExprs[stmt] = val;
	}
	int64_t getStmtVal(Stmt *stmt){
		// 一定要取到表达式的末尾的值
		assert(mExprs.find(stmt) != mExprs.end());
		return mExprs[stmt];
	}
	void setPC(Stmt *stmt){
		mPC = stmt;
	}
	Stmt *getPC(){
		return mPC;
	}
	// 新加
	void setReturn(bool tp, int64_t val){
		retType = tp;
		retValue = val;
	}
	bool ifReturn(){
		if (retType == 0 && retValue == 0){
			return false;
		}else{
			return true;
		}
	}
	int getReturn(){
		if (retType){
			return retValue;
		}else{
			return 0;
		}
	}
};

class Environment
{
	std::vector<StackFrame> mStack;

	FunctionDecl *mFree; /// Declartions to the built-in functions
	FunctionDecl *mMalloc;
	FunctionDecl *mInput;
	FunctionDecl *mOutput;

	FunctionDecl *mEntry;

public:
	/// Get the declartions to the built-in functions
	Environment() : mStack(), mFree(NULL), mMalloc(NULL), mInput(NULL), mOutput(NULL), mEntry(NULL)
	{
	}

	/// Initialize the Environment
	void init(TranslationUnitDecl *unit){
		mStack.push_back(StackFrame());
		for (TranslationUnitDecl::decl_iterator i = unit->decls_begin(), e = unit->decls_end(); i != e; ++i){
			// 入口就是函数
			if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(*i)){
				// 获取函数名称，依次对应，最后进入main
				if (fdecl->getName().equals("FREE"))
					mFree = fdecl;
				else if (fdecl->getName().equals("MALLOC"))
					mMalloc = fdecl;
				else if (fdecl->getName().equals("GET"))
					mInput = fdecl;
				else if (fdecl->getName().equals("PRINT"))
					mOutput = fdecl;
				else if (fdecl->getName().equals("main"))
					mEntry = fdecl;
			}else if (VarDecl *vardecl = dyn_cast<VarDecl>(*i)){
				// 全局变量初始化
				if (vardecl->hasInit()){
					mStack.back().bindDecl(vardecl, matchType(vardecl->getInit()));
				}else{
					// 初始化默认为0
					mStack.back().bindDecl(vardecl, 0);
				}
			}
		}
	}

	// mEntry存储的是main函数体
	FunctionDecl *getEntry(){
		return mEntry;
	}

	// 匹配各种表达式的类型，然后从栈里面取出值
	int64_t matchType(Expr *expr){
		expr = expr->IgnoreImpCasts();
		if (auto integerLiteral = dyn_cast<IntegerLiteral>(expr)){
			//int类型获取对应的值
			llvm::APInt result = integerLiteral->getValue();
			return result.getSExtValue();
		}else if (BinaryOperator *bop = dyn_cast<BinaryOperator>(expr)){ //+ - * / < > ==
			binop(bop);
			return mStack.back().getStmtVal(bop);
		}else if (UnaryOperator *unaryExpr = dyn_cast<UnaryOperator>(expr)){
			unaryop(unaryExpr);
			return mStack.back().getStmtVal(unaryExpr);
		}else if (auto decl = dyn_cast<DeclRefExpr>(expr)){
			// 把变量绑定再获取值
			declref(decl);
			return mStack.back().getStmtVal(decl);
		}else if (auto callexpr = dyn_cast<CallExpr>(expr)){
			return mStack.back().getStmtVal(callexpr);
		}else if (auto castexpr = dyn_cast<CStyleCastExpr>(expr)){
			return matchType(castexpr->getSubExpr());
		}else if (auto array = dyn_cast<ArraySubscriptExpr>(expr)){
			// Array讨论三种类型 int char 和 *
			if (DeclRefExpr *declexpr = dyn_cast<DeclRefExpr>(array->getLHS()->IgnoreImpCasts())){
				Decl *decl = declexpr->getFoundDecl();
				int64_t index = matchType(array->getRHS());
				if (VarDecl *vardecl = dyn_cast<VarDecl>(decl)){
					if (auto array = dyn_cast<ConstantArrayType>(vardecl->getType().getTypePtr())){
						int64_t tmp = mStack.back().getDeclVal(vardecl);
						if (array->getElementType().getTypePtr()->isIntegerType()){ 
							return *((int *)tmp + index);
						}else if (array->getElementType().getTypePtr()->isIntegerType()){ 
							return *((char *)tmp + index);
						}else{
							int64_t **p = (int64_t **)tmp;
							return (int64_t)(*(p + index));
						}
					}
				}
			}
		}else if (auto sizeofexpr = dyn_cast<UnaryExprOrTypeTraitExpr>(expr)){
			if (sizeofexpr->getKind() == UETT_SizeOf){ 
				//sizeof
				if (sizeofexpr->getArgumentType()->isIntegerType()){
					return sizeof(int); 
				}else if (sizeofexpr->getArgumentType()->isPointerType()){
					return sizeof(int *); 
				}
			}
		}else if (auto charLiteral = dyn_cast<CharacterLiteral>(expr)){									
			return charLiteral->getValue(); 
		}else if (auto parenExpr = dyn_cast<ParenExpr>(expr)){ // (E)
			return matchType(parenExpr->getSubExpr());
		}

		return 0;
	}

	/// !TODO Support comparison operation
	void binop(BinaryOperator *bop){
		Expr *left = bop->getLHS();	 // 操作符左边
		Expr *right = bop->getRHS(); // 操作符右边

		if (bop->isAssignmentOp()){
			// 如果是赋值操作
			if (DeclRefExpr *declexpr = dyn_cast<DeclRefExpr>(left)){
				// 普通类型赋值
				mStack.back().bindStmt(left, matchType(right));
				mStack.back().bindDecl(declexpr->getFoundDecl(), matchType(right));
			}else if (UnaryOperator *unop = dyn_cast<UnaryOperator>(left)){
				// 指针赋值  int* a =
				// 指针类型是在unary里面定义的
				int64_t *p = (int64_t *)matchType(unop->getSubExpr());
				*p = matchType(right);
			}else if (auto array = dyn_cast<ArraySubscriptExpr>(left)){
				if (DeclRefExpr *declexpr = dyn_cast<DeclRefExpr>(array->getLHS()->IgnoreImpCasts())){
					Decl *decl = declexpr->getFoundDecl();
					int64_t val = matchType(right);
					int64_t index = matchType(array->getRHS());
					if (VarDecl *vardecl = dyn_cast<VarDecl>(decl)){
						if (auto array = dyn_cast<ConstantArrayType>(vardecl->getType().getTypePtr())){
							int64_t tmp = mStack.back().getDeclVal(vardecl);
							if (array->getElementType().getTypePtr()->isIntegerType()){ // IntegerArray
								int *p = (int *)tmp;
								*(p + index) = val;
							}else if (array->getElementType().getTypePtr()->isCharType()){ // Char
								char *p = (char *)tmp;
								*(p + index) = (char)val;
							}else{
								int64_t **p = (int64_t **)tmp;
								*(p + index) = (int64_t *)val;
							}
						}
					}
				}
			}
		}else{
			// 实现+-*/<>==
			auto opcode = bop->getOpcode();
			int64_t result = 0;
			switch (opcode){
			// +
			case (BO_Add):
				if (left->getType().getTypePtr()->isPointerType()){
					// 考虑指针类型加法
					result = matchType(left) + sizeof(int64_t) * matchType(right);
				}else{
					result = matchType(left) + matchType(right);
				}
				break;
			// -
			case (BO_Sub):
				result = matchType(left) - matchType(right);
				break;
			// *
			case (BO_Mul):
				result = matchType(left) * matchType(right);
				break;
			// /
			case (BO_Div):
				//除数不能为0
				if (matchType(right) == 0){
					exit(0);
				}else{
					result = matchType(left) / matchType(right);
					break;
				}
			// <
			case (BO_LT):
				if (matchType(left) < matchType(right)){
					result = 1;
				}else{
					result = 0;
				}
				break;
			// >
			case (BO_GT):
				if (matchType(left) > matchType(right)){
					result = 1;
				}else{
					result = 0;
				}
				break;
			// ==
			case (BO_EQ):
				if (matchType(left) == matchType(right)){
					result = 1;
				}else{
					result = 0;
				}
				break;
			default:
				exit(0);
				break;
			}
			// 把计算结果放进栈里
			mStack.back().bindStmt(bop, result);
		}
	}

	// 新加unaryop
	void unaryop(UnaryOperator *uop){
		auto opCode = uop->getOpcode();
		auto expr = uop->getSubExpr();
		switch (opCode){
		//'-'
		case (UO_Minus):
			mStack.back().bindStmt(uop, -1 * matchType(expr));
			break;
		//'+'
		case (UO_Plus):
			mStack.back().bindStmt(uop, matchType(expr));
			break;
		// '*'
		case (UO_Deref):
			mStack.back().bindStmt(uop, *(int64_t *)matchType(uop->getSubExpr()));
			break;
		default:
			exit(0);
			break;
		}
	}

	// 变量声明  int64_t a
	void decl(DeclStmt *declstmt){
		for (DeclStmt::decl_iterator it = declstmt->decl_begin(), ie = declstmt->decl_end();
			 it != ie; ++it){
			Decl *decl = *it;
			if (VarDecl *vardecl = dyn_cast<VarDecl>(decl)){
				// int64_t或者指针类型赋初值
				auto type = vardecl->getType().getTypePtr();
				if (type->isIntegerType() || type->isPointerType()){
					// 变量要查看有无初始值
					if (vardecl->hasInit()){
						mStack.back().bindDecl(vardecl, matchType(vardecl->getInit()));
					}else{
						mStack.back().bindDecl(vardecl, 0);
					}
				}else{
					if (auto array = dyn_cast<ConstantArrayType>(vardecl->getType().getTypePtr())){ 
						int64_t length = array->getSize().getSExtValue();
						if (array->getElementType().getTypePtr()->isIntegerType()){ 
							int *a = new int[length];
							for (int i = 0; i < length; i++){
								a[i] = 0;
							}
							mStack.back().bindDecl(vardecl, (int64_t)a);
						}else if (array->getElementType().getTypePtr()->isCharType()){ 
							char *a = new char[length];
							for (int i = 0; i < length; i++){
								a[i] = 0;
							}
							mStack.back().bindDecl(vardecl, (int64_t)a);
						}else{ // int* c[2];
							int64_t **a = new int64_t *[length];
							for (int i = 0; i < length; i++){
								a[i] = 0;
							}
							mStack.back().bindDecl(vardecl, (int64_t)a);
						}
					}
				}
			}
		}
	}
	void declref(DeclRefExpr *declref){
		mStack.back().setPC(declref);
		// int64_t类型变量
		if (declref->getType()->isIntegerType() || declref->getType()->isPointerType()){
			Decl *decl = declref->getFoundDecl();		  // 去 vars里面找对应变量的类型
			int64_t val = mStack.back().getDeclVal(decl); //去vars里面找值
			mStack.back().bindStmt(declref, val);
		}
	}

	void cast(CastExpr *castexpr){
		mStack.back().setPC(castexpr);
		if (castexpr->getType()->isIntegerType()){
			Expr *exp = castexpr->getSubExpr();
			int64_t val = matchType(exp);
			mStack.back().bindStmt(castexpr, val);
		}
	}

	void call(CallExpr *callexpr){
		mStack.back().setPC(callexpr);
		int val = 0;
		FunctionDecl *callee = callexpr->getDirectCallee();
		if (callee == mInput){
			llvm::errs() << "Please Input an int64_teger Value : ";
			scanf("%d", &val);
			mStack.back().bindStmt(callexpr, val);
		}else if (callee == mOutput){
			// char可以输出吗？ 没有换行输出
			Expr *decl = callexpr->getArg(0);
			val = mStack.back().getStmtVal(decl);
			llvm::errs() << val;
		}else if (callee == mMalloc){
			int64_t *p = (int64_t *)std::malloc(matchType(callexpr->getArg(0)));
			mStack.back().bindStmt(callexpr, (int64_t)p);
		}else if (callee == mFree){
			int64_t *p = (int64_t *)matchType(callexpr->getArg(0));
			std::free(p);
		}else{
			// 调用自定义的函数
			vector<int64_t> args;
			for (auto i = callexpr->arg_begin(), e = callexpr->arg_end(); i != e; i++){
				args.push_back(matchType(*i));
			}
			mStack.push_back(StackFrame()); // 又创建了一个新的stack,此时mSatck size为2
			int64_t j = 0;
			for (auto i = callee->param_begin(), e = callee->param_end(); i != e; i++, j++){
				mStack.back().bindDecl(*i, args[j]);
			}
		}
	}
	// 新加
	void integer(IntegerLiteral *integerLiteral){
		mStack.back().setPC(integerLiteral);
		mStack.back().bindStmt(integerLiteral, matchType(integerLiteral));
	}
	void returnOp(ReturnStmt *returnStmt){
		mStack.back().setReturn(true, matchType(returnStmt->getRetValue()));
	}
	bool ifReturn(){
		return mStack.back().ifReturn();
	}
	// 非系统函数的调用
	void customFunction(CallExpr *call){
		int64_t returnValue = mStack.back().getReturn();
		mStack.pop_back();
		mStack.back().bindStmt(call, returnValue);
	}
	void doCStyleCastExpr(CStyleCastExpr *cStyleCastExpr){
		mStack.back().setPC(cStyleCastExpr);
		mStack.back().bindStmt(cStyleCastExpr, matchType(cStyleCastExpr));
	}
};