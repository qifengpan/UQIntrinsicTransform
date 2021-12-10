#include "rose.h"
#include <string>
#include "SearchingMethod.h"

using namespace std;

SgScopeStatement* Var_has_ClassDefinition(SgVarRefExp* node){
	SgScopeStatement* NewScope = NULL;
	SgVariableSymbol* LocalSymbol = node->get_symbol();
	SgInitializedName* LocalName  = LocalSymbol->get_declaration();
	if(LocalName->get_declptr()){
    SgVariableDefinition* VariableDecl = isSgVariableDefinition(LocalName->get_declptr());
	SgInitializedName* NewName = isSgInitializedName(VariableDecl->get_vardefn());
		if(LocalName->get_scope()!=NewName->get_scope()){
			NewScope = NewName->get_scope();
		}
	}
	return NewScope;
}

AstAttribute* return_Var_AstAttribute(SgVarRefExp* Node,string Type){
	if(Node->get_symbol()->getAttribute(Type)){
		return Node->get_symbol()->getAttribute(Type);
	}else if(Var_has_ClassDefinition(Node)){
		SgScopeStatement* NewScope = Var_has_ClassDefinition(Node);
		SgVariableSymbol* NewSymbolInClass = NewScope-> lookup_var_symbol(Node->unparseToString());
		return NewSymbolInClass->getAttribute(Type);
	}else{
		return NULL;
	}

}

vector<int> reduce_redudent_Type(vector<int> UQ_type){
	vector<int>::iterator vector_iterator;
	sort(UQ_type.begin(),UQ_type.end());
	vector_iterator = unique(UQ_type.begin(),UQ_type.end());
	if(vector_iterator != UQ_type.end()){
        UQ_type.erase(vector_iterator,UQ_type.end());
    }
	return UQ_type;
}

vector<int> set_Type_Union(vector<int> Vector1, vector<int> Vector2){
	vector<int> result;
	set_union(Vector1.begin(),Vector1.end(),Vector2.begin(),Vector2.end(),back_inserter(result));
	return result;
}

void Set_VarRef_UQ_Atrribute(SgVarRefExp* node,vector<int> v){
       AstAttribute* newAttribute = new persistantAttribute(v);
       node->get_symbol()->addNewAttribute("UQ",newAttribute);
	   if(Var_has_ClassDefinition(node)){
		   //get symbol table || get scope statement
	   		SgScopeStatement* NewScope = Var_has_ClassDefinition(node);
	   		//search corresponding symbol
			SgVariableSymbol* NewSymbolInClass = NewScope-> lookup_var_symbol(node->unparseToString());
		   //set UQ attribute
			NewSymbolInClass->addNewAttribute("UQ",newAttribute);
	   }
   }

void Set_AstNode_UQ_Atrribute(SgNode* node,vector<int> v){
       AstAttribute* newAttribute = new persistantAttribute(v);
       node->addNewAttribute("UQ",newAttribute);
   }


void Set_VarRef_UQ_Atrribute_Union2Sides(SgVarRefExp* node,vector<int> Left_Type){
	AstAttribute* existAttri = return_Var_AstAttribute(node,"UQ");
    vector<int> Original_Type = dynamic_cast<persistantAttribute*>(existAttri)->value;
    vector<int> Left_Type_Final = set_Type_Union(Left_Type,Original_Type);
    dynamic_cast<persistantAttribute*>(existAttri)->set_value(Left_Type_Final);

}


void Set_VarRef_Atrribute(string name,SgVarRefExp* node){
     AstAttribute* newAttribute = new persistantAttribute({1});
     node->get_symbol()->addNewAttribute(name,newAttribute);
 }

 void Set_AstNode_Atrribute(string name, SgNode* node){
     AstAttribute* newAttribute = new persistantAttribute({1});
     node->addNewAttribute(name,newAttribute);
 }

SgVarRefExp* get_LeftInnerMost_Var(SgAssignOp* AssingOp){
	SgExpression* Expr = isSgExpression(AssingOp);
	while(isSgBinaryOp(Expr)){
		Expr = isSgExpression(isSgBinaryOp(Expr)->get_lhs_operand_i());
	}
	return isSgVarRefExp(Expr);
}

SgVarRefExp* get_RightInnerMost_Var(SgExpression* Expr){
	//get the right most var is tricky here:
	//problem would meet:
	//    a%b(1:2)
	//    b is an Pointer, so it should return lhs of this pointer!
	//solution here:
	//add judgement in the loop. when Pointer detected, we should determine whether it
	//is time to break the loop
	while(isSgBinaryOp(Expr)){
		if(isSgPntrArrRefExp(Expr)){
			SgPntrArrRefExp* Pntr = isSgPntrArrRefExp(Expr);
			SgExpression* rhs_pntr=Pntr->get_rhs_operand_i();

			if(isSgPntrArrRefExp(rhs_pntr)){
				Expr = rhs_pntr;
			}else{
				Expr = isSgExpression(Pntr->get_lhs_operand_i());
			}
		}else{
			Expr = isSgExpression(isSgBinaryOp(Expr)->get_rhs_operand_i());
		}
	}

	return isSgVarRefExp(Expr);
}

//extract SgVarRef from different kind of struction
//1.SgVarRefExp or SgPntrArrRefExp
//2.SgDotRefExp(Type)
//3.Expr with SgVarRefExp 
SgVarRefExp* get_VarRefExp(SgExpression* Expr){
	//call (-a), call(1),call(a=b)A
	if(isSgSubtractOp(Expr)){
		SgSubtractOp* SubExpr = isSgSubtractOp(Expr);
		if(isSgVarRefExp(SubExpr->get_lhs_operand_i())){
			return isSgVarRefExp(SubExpr->get_lhs_operand_i());
		}else if(isSgVarRefExp(SubExpr->get_rhs_operand_i())){
			return isSgVarRefExp(SubExpr->get_rhs_operand_i());
		}else{
			if(isSgFunctionCallExp(SubExpr->get_lhs_operand_i())){
				SgFunctionCallExp* FuncCall = isSgFunctionCallExp(SubExpr->get_lhs_operand_i());
				return get_VarRefExp(FuncCall);
			}
			return NULL;
		}

	}
	if(isSgValueExp(Expr)||isSgMinusOp(Expr)){
		return NULL;		
	}

	if(!isSgValueExp(Expr) && !isSgMinusOp(Expr) && isSgVarRefExp(Expr)){
		return isSgVarRefExp(Expr);
	}
	if(!isSgValueExp(Expr) && !isSgMinusOp(Expr) && isSgActualArgumentExpression(Expr)){
		SgExpression* NewExpr = isSgActualArgumentExpression(Expr)->get_expression();
		if(isSgBinaryOp(NewExpr)){
			while(isSgBinaryOp(NewExpr)){
	             NewExpr = isSgExpression(isSgBinaryOp(Expr)->get_lhs_operand_i());
	     }
		}
		return isSgVarRefExp(NewExpr);
	}
	//dot,Pointer
	if(isSgBinaryOp(Expr)){
		//should have a selcet?
		//if(Pointer){getLhsMost}
		//if(Dot){getRhsMost}
		while(isSgBinaryOp(Expr)){
			Expr = isSgExpression(isSgBinaryOp(Expr)->get_lhs_operand_i());
		}
		return isSgVarRefExp(Expr);
	}
	//function call like func(int(a))
	//This version just works for single variable input
	//multiple judgement is not implemented.
	if(isSgFunctionCallExp(Expr)){
		SgExprListExp* VarList = isSgFunctionCallExp(Expr)->get_args();
		SgExpressionPtrList &exprs = VarList[0].get_expressions();
		if(isSgVarRefExp(exprs[0])){
			return isSgVarRefExp(exprs[0]);
		}
		if(isSgPntrArrRefExp(exprs[0])){
			return get_VarRefExp(exprs[0]);
		}
	}
	return NULL;
}

SgFortranDo* get_UpperMost_for(SgFortranDo* InnerMost){
	SgFortranDo* UpperMost = InnerMost;
	while(isSgFortranDo(UpperMost->get_parent()->get_parent())){
		UpperMost = isSgFortranDo(UpperMost->get_parent()->get_parent());
	}
	return UpperMost;
}


bool SameType(SgExpression* Expr,SgInitializedName* Name){
	if(isSgVarRefExp(Expr)){
		SgInitializedName* NameFromExpr = isSgInitializedName(isSgVarRefExp(Expr)->get_symbol()->get_declaration());
		SgType* ExprType = NameFromExpr->get_typeptr();
		SgType* NameType = Name->get_typeptr();
		return ExprType->isEquivalentType(NameType);
	}
	if(isSgPntrArrRefExp(Expr)){

	}
}

//judge if two exprlist are indentical
bool isSameExprList(SgExprListExp* ExprFromCall,SgFunctionParameterList* ExprFromHeader){
	SgExpressionPtrList &Expr = ExprFromCall->get_expressions();
	SgInitializedNamePtrList &Name = ExprFromHeader->get_args();
	if(Expr.size()!= Name.size()){
		return false;
	}else{
		for(size_t i = 0;i<Expr.size();i++){
			if(!SameType(Expr[i],Name[i])){
				return false;
			}
		}
	}
	return true;
}

//compare the input Calling Expr list with all functions in funciont table
SgProcedureHeaderStatement* CompareCallExprList(SgExprListExp* inputExprList,SgSymbolTable* inputTable){
	Rose_STL_Container<SgNode*> FunctionSymbolList  = NodeQuery::querySubTree(inputTable,V_SgFunctionSymbol);
	for(Rose_STL_Container<SgNode*>::iterator i = FunctionSymbolList.begin();i!=FunctionSymbolList.end();i++){
		SgFunctionSymbol* iSymbol = isSgFunctionSymbol(*i);
		//get function header
		SgProcedureHeaderStatement* iHeader = isSgProcedureHeaderStatement(iSymbol->get_declaration());
		SgFunctionParameterList* FuncParam  = iHeader->get_parameterList();
		bool foundHeader = isSameExprList(inputExprList,FuncParam);
		if(foundHeader){
			return iHeader;
		}
	}
	return NULL;
}

//find corresponding Header for rename interfac
SgProcedureHeaderStatement* FindCorrespondingHeader(SgRenameSymbol* Node,SgFunctionCallExp* FuncCall){
	SgProcedureHeaderStatement* Target_Header;
	SgExprListExp* CallingExprList = FuncCall->get_args();
	SgSymbolTable* FunctionTable   = isSgSymbolTable(Node->get_parent());
	//searching all function symbol in table and compare the input list between
	//input one and target one. if they are the same, the target header is found
	Target_Header = CompareCallExprList(CallingExprList,FunctionTable);
	return Target_Header;
}


SgProcedureHeaderStatement* FromCallToDecl(SgFunctionCallExp* FuncCall){
	SgFunctionRefExp* FuncRef = isSgFunctionRefExp(FuncCall->get_function());
	ROSE_ASSERT(FuncRef!=NULL);
	SgFunctionSymbol* Node = isSgFunctionSymbol(FuncRef->get_symbol_i());
	ROSE_ASSERT(Node!=NULL);
	SgProcedureHeaderStatement* Decl = isSgProcedureHeaderStatement(Node->get_declaration());
	ROSE_ASSERT(Decl!=NULL);
	SgProcedureHeaderStatement* DeclFinal = isSgProcedureHeaderStatement(Decl->get_definingDeclaration());
	if(DeclFinal==NULL){
		//FuncCall could have interface, try to find correct interface 
		//since ROSE has a known bug for finding the correct function delcaration
		//this must be implemented manually.
		SgRenameSymbol* Node = isSgRenameSymbol(FuncRef->get_symbol_i());
		if(Node==NULL){
			//if it is not interface either, 
			//return NULL. It could be a system function
			return NULL;
		}
		SgProcedureHeaderStatement* DeclFinal = FindCorrespondingHeader(Node,FuncCall);

	}
	//ROSE_ASSERT(DeclFinal!=NULL);
	return DeclFinal;
	
}
bool isSgFunctionCallSubroutine(SgNode* node){
	if(isSgFunctionCallExp(node)){
		SgFunctionCallExp* FuncCall = isSgFunctionCallExp(node);
		SgProcedureHeaderStatement* FunHeader = FromCallToDecl(FuncCall);
		if(FunHeader){
			return FunHeader->isSubroutine();
		}
	}
	return false;
}


vector<int> CountUQPosi(SgNode* n,vector<vector<int>> *UQ_Para_Type){
	SgExprListExp* ArgList = isSgExprListExp(n);
	ROSE_ASSERT(ArgList!=NULL);
	vector<int> UQposi;
	SgExpressionPtrList &exprs = ArgList->get_expressions();
	size_t NumberInput  = ArgList->get_numberOfTraversalSuccessors();
	for (size_t i = 0;i <NumberInput;i++){
		SgVarRefExp* varExp =get_VarRefExp(exprs[i]);
		if(varExp){
			AstAttribute* var_attribute =  return_Var_AstAttribute(varExp,"UQ");
			if((var_attribute!=NULL)){
				UQposi.push_back(i);
				vector<int> Buffer_vector = dynamic_cast<persistantAttribute*>(var_attribute)->value;
                (*UQ_Para_Type).push_back(Buffer_vector);
			}
		}
	}
	return UQposi;
}

vector<string> get_UQName_InFunction(SgProcedureHeaderStatement* FuncHeader,vector<int> UQposi){
	vector<string> UQname;
	SgFunctionParameterList* FuncPara = FuncHeader->get_parameterList();
	SgInitializedNamePtrList &VarList = FuncPara->get_args();
	for (size_t i =0 ; i<UQposi.size();i++){
		UQname.push_back(VarList[UQposi[i]]->get_name());
	}
	return UQname;
}

void Setting_Token_InFUnction(SgProcedureHeaderStatement* FuncHeader,vector<string> UQname,vector<vector<int>> UQ_Type_in_ParaList){
	SgFunctionDefinition* Defi = FuncHeader->get_definition();
	ROSE_ASSERT(Defi);
	SgSymbolTable* symbolTable = Defi->get_symbol_table();
	ROSE_ASSERT(symbolTable);
 	SgVariableSymbol* var_symbol;
	for(size_t i = 0;i<UQname.size();i++){
		var_symbol = symbolTable->find_variable(UQname[i]);
		Set_AstNode_UQ_Atrribute(var_symbol,UQ_Type_in_ParaList[i]);
	}

}

bool isFuncSearched(SgFunctionCallExp* FuncCall){
	SgFunctionRefExp* FuncRef    = isSgFunctionRefExp(FuncCall->get_function());
	SgFunctionSymbol* FuncSymbol = isSgFunctionSymbol(FuncRef->get_symbol_i());	
	if(FuncSymbol->getAttribute("FuncUQ")){
		return true;
	}else{
		return false;
	}
}


SgFunctionSymbol* GetFuncSymbolFromCall(SgFunctionCallExp* FuncCall){
	SgFunctionRefExp* FuncRef    = isSgFunctionRefExp(FuncCall->get_function());
    SgFunctionSymbol* FuncSymbol = isSgFunctionSymbol(FuncRef->get_symbol_i());
	return FuncSymbol;

}

bool isFuncHasUQ(SgFunctionCallExp* FuncCall){
	SgFunctionSymbol* FuncSymbol = GetFuncSymbolFromCall(FuncCall);	

    AstAttribute* existAttri     = FuncSymbol->getAttribute("FuncUQ");
	int marker = dynamic_cast<persistantAttributeFunc*>(existAttri)->value;
	if(marker==1){
		return true;
	}else{
		return false;
	}
}

void PresearchUQinFunc(SgProcedureHeaderStatement* FuncHeader,SgFunctionCallExp* FuncCall){
	Rose_STL_Container<SgNode*> SymbolList = NodeQuery::querySubTree(FuncHeader->get_definition()->get_body(),V_SgVarRefExp);
	bool Set_flag = false;
    SgFunctionSymbol* FuncSymbol = GetFuncSymbolFromCall(FuncCall);
	for(Rose_STL_Container<SgNode*>::iterator i = SymbolList.begin();i!=SymbolList.end();i++){
		SgVarRefExp* VarRef  = isSgVarRefExp(*i);
		if(return_Var_AstAttribute(VarRef,"UQ")){
			Set_flag = true;
		}
	}
	if(Set_flag){
		AstAttribute* newFuncAttri = new persistantAttributeFunc(1);
		FuncSymbol->addNewAttribute("FuncUQ",newFuncAttri);
	}else{
		AstAttribute* newFuncAttri = new persistantAttributeFunc(0);
		FuncSymbol->addNewAttribute("FuncUQ",newFuncAttri);
	}
	

}


//get function name from function calling expression.
SgName extractFunctionName(SgFunctionCallExp* FuncCall){
	SgFunctionRefExp* FuncRef = isSgFunctionRefExp(FuncCall->get_function());
	SgName  FunctionName = FuncRef->get_symbol_i()->get_name();
	return FunctionName;
}

bool isFunctionInBuildInList(SgName FunctionName)
{

	std::vector<string> v = {"int","float","aint","mod","trim","adjustl","size","nint","sin","cos","real","sum","pack"};
	bool FuncInList = false;
	for(size_t i = 0;i<v.size();i++){
		if(v[i].compare(FunctionName)==0){
			FuncInList = true;
		}
	}
	return FuncInList;
}

vector<int> GetUQTypeUnderSubtree(SgNode* n)
{
	vector<int> UQ_type={};
	Rose_STL_Container<SgNode*> nodes = NodeQuery::querySubTree(n,V_SgVarRefExp); 
	for(Rose_STL_Container<SgNode*> ::iterator i = nodes.begin();i!=nodes.end();i++){
		SgVarRefExp* Var = isSgVarRefExp((*i));
		if(return_Var_AstAttribute(Var,"UQ")){
			AstAttribute* existAttri = return_Var_AstAttribute(Var,"UQ");
			vector<int> Buffer_Vec = dynamic_cast<persistantAttribute*>(existAttri)->value;
            for(size_t i = 0; i < Buffer_Vec.size();i++){
				UQ_type.push_back(Buffer_Vec[i]);
			}
		}
	}
	return UQ_type;
}