#include "rose.h"
// #include "SearchingMethod.h"

#include <string>

using namespace std;



vector<int> reduce_redudent_Type(vector<int> UQ_type);

vector<int> set_Type_Union(vector<int> Vector1, vector<int> Vector2);
 void Set_VarRef_UQ_Atrribute(SgVarRefExp* node,vector<int> v);
 void Set_AstNode_UQ_Atrribute(SgNode* node,vector<int> v);
 void Set_VarRef_UQ_Atrribute_Union2Sides(SgVarRefExp* node, vector<int> Left_Type);

 void Set_VarRef_Atrribute(string name,SgVarRefExp* node);
 void Set_AstNode_Atrribute(string name, SgNode* node);
 SgVarRefExp* get_LeftInnerMost_Var(SgAssignOp* AssingOp);
 SgVarRefExp* get_RightInnerMost_Var(SgExpression* Expr);
 SgFortranDo* get_UpperMost_for(SgFortranDo* InnerMost);
 SgProcedureHeaderStatement* FromCallToDecl(SgFunctionCallExp* FuncCall);
 SgVarRefExp* get_VarRefExp(SgExpression* Expr);

 AstAttribute* return_Var_AstAttribute(SgVarRefExp* Node,string Tpye);
 bool isSgFunctionCallSubroutine(SgNode* node); 
 vector<int> CountUQPosi(SgNode* n,vector<vector<int>> *UQ_Para_Type);
 vector<string> get_UQName_InFunction(SgProcedureHeaderStatement* FuncHeader,vector<int> UQposi);
 void Setting_Token_InFUnction(SgProcedureHeaderStatement* FuncHeader,vector<string> UQname,vector<vector<int>> UQ_Type_in_ParaList);
 SgFunctionSymbol* GetFuncSymbolFromCall(SgFunctionCallExp* FuncCall);
 bool isFuncHasUQ(SgFunctionCallExp* FuncCall);
 void PresearchUQinFunc(SgProcedureHeaderStatement* FuncHeader,SgFunctionCallExp* FuncCall);
 SgFunctionSymbol* GetFuncSymbolFromCall(SgFunctionCallExp* FuncCall);
 bool isFuncSearched(SgFunctionCallExp* FuncCall);
 bool isFunctionInBuildInList(SgName FunctionName);
 vector<int> GetUQTypeUnderSubtree(SgNode* n);
 SgName extractFunctionName(SgFunctionCallExp* FuncCall);