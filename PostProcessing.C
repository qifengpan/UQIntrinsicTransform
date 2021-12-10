#include "rose.h"
#include "SearchingMethod.h" 
#include "HelpingFuncs.h"
  using namespace std;
  using namespace SageInterface;
  using namespace SageBuilder;

#define FILE_SOURCE Sg_File_Info::generateDefaultFileInfoForTransformationNode()


SgArrayType* get_array_type(SgNode* AstNode){
	SgVarRefExp* Var = isSgVarRefExp(AstNode);
	SgVariableSymbol* VariSymbol = Var->get_symbol();
	SgInitializedName* VariInit  = VariSymbol->get_declaration();
	SgArrayType* VarType         = isSgArrayType(VariInit->get_typeptr());
	return VarType;
}
SgSubscriptExpression* judgeIfAllocateable(SgArrayType* VarType){
	bool result = false;
	SgTreeCopy tc;
	SgSubscriptExpression* copy_result;
	SgExprListExp* IndexList = isSgExprListExp(VarType->get_dim_info());
	SgExpressionPtrList &exprs = IndexList->get_expressions();
	size_t NumberInput  = IndexList->get_numberOfTraversalSuccessors();
		for (size_t i = 0;i <NumberInput;i++){
				SgSubscriptExpression* SubScript = isSgSubscriptExpression(exprs[i]);
				if(SubScript){
					SgNullExpression* NullIndex = isSgNullExpression(SubScript->get_upperBound());
					if(NullIndex){
						copy_result = isSgSubscriptExpression(SubScript->copy(tc));
						result = true;	
					}
				}
        	}
	if(result){
		return copy_result;
	}else{
		return NULL;
	}
}
void replace_Vari_declaration(SgNode* AstNode){
	//get pointer of node
	// check if this variable Array type
	SgVarRefExp* Vari = isSgVarRefExp(AstNode);
	
	bool isArray = false;
	
	SgArrayType* VarType = get_array_type(Vari);
	
	if(VarType!=NULL){
		isArray=true;
	}

	if(isSgVarRefExp(AstNode)->get_symbol()->getAttribute("ModifiedDecl")==NULL && !isArray){
	Set_VarRef_Atrribute("ModifiedDecl",Vari);	
	//create a new variable declaration
	SgExprListExp* Dim_info = buildExprListExp(buildIntVal(10));
	SgVariableDeclaration* variableArray = buildVariableDeclaration(Vari->get_symbol()->get_name(),  buildArrayType(Vari->get_symbol()->get_type(),Dim_info));
	//SgVariableDeclaration* variableArray   = creatType()
	//find the origination decalration
	SgVariableDeclaration* Variabledecl  = isSgVariableDeclaration(Vari->get_symbol()->get_declaration()->get_parent());
	//replace using Sageinterface
	//if it has Variabledecl in the blcok: explicit defined, needed to be replace,otherwise no need to do it.
	//for e.g int sub(int a, int b): a and b will not decleared in the scope again, therefore, no need to replace them
	//cout <<"new vari is"<<variableArray->unparseToString()<<"\n";
	if(Variabledecl){
		replaceStatement(Variabledecl,variableArray);
	//	cout <<variableArray->unparseToString()<<"\n";
	}
	//Parent_scope->replace_statement(Variabledecl,variableArray);
	//isSgVarRefExp(AstNode)->get_symbol()->addNewAttribute("ModifiedDecl",newAttribute);
	}
	if(isSgVarRefExp(AstNode)->get_symbol()->getAttribute("ModifiedDecl")==NULL && isArray){
		SgSubscriptExpression* SubScript = judgeIfAllocateable(VarType);
		Set_VarRef_Atrribute("ModifiedDecl",Vari);
		if(!SubScript){
			//cout <<"in if branch \n";
			//cout <<"Vari is "<<Vari->unparseToString()<<"\n";
			SgIntVal* extra_dim = buildIntVal(10);
			SgExprListExp* Index_list = isSgExprListExp(isSgArrayType(VarType)->get_dim_info());	
			//cout <<"Index list is: "<<Index_list->unparseToString()<<"\n";
			Index_list->prepend_expression(extra_dim);
		}else{
			//add extra dimension for allocatable array declaration
			//adding extra dimension for allocation is done at top level
			SgExprListExp* IndexList = isSgExprListExp(VarType->get_dim_info());
			IndexList->prepend_expression(SubScript);

		}
	}

}


void replace_Vari_Reference(SgNode* AstNode){
	SgTreeCopy tc;
	SgNode* newNode = AstNode->copy(tc);
	if(isSgVarRefExp(AstNode)){
	  SgVarRefExp* Vari = isSgVarRefExp(AstNode);
	  SgPntrArrRefExp* lhs_var = buildPntrArrRefExp(isSgExpression(newNode),buildVarRefExp("iSample"));
      //replace the original one
      replaceExpression(Vari,lhs_var);
      //replace the original one
      //replaceExpression(Vari,lhs_var);
      //lhs_var->set_parent(Assign);
      //Assign->replace_expression(Vari,lhs_var);

	}
	if(isSgPntrArrRefExp(AstNode)){
	  //lhs_var->set_parent(Assign);
      //Assign->replace_expression(Vari,lhs_var);
	  SgPntrArrRefExp* Vari = isSgPntrArrRefExp(AstNode);
		//create a new expression for array reference
	  SgExprListExp* Index_list = isSgExprListExp(Vari->get_rhs_operand_i());

      SgVarRefExp* new_index = buildVarRefExp("iSample");

	  Index_list->prepend_expression(new_index);

	}
}

SgPntrArrRefExp* find_pointer_Vari(SgNode* vari){
	SgPntrArrRefExp* Vari_pr = isSgPntrArrRefExp(vari->get_parent());
	ROSE_ASSERT(Vari_pr!=NULL);
	return Vari_pr;
}


void VectorzingStatement(SgAssignOp* Assign, SgExprStatement* currentStmt, SgScopeStatement* Parent_scope){
	SgTreeCopy tc;
	SgNode* newNode = Assign->copy(tc);
	Rose_STL_Container<SgNode*> VarVector = NodeQuery::querySubTree(newNode,V_SgVarRefExp);
	for(Rose_STL_Container<SgNode*>::iterator i = VarVector.begin();i!=VarVector.end();i++){
		if(isSgVarRefExp((*i))->getAttribute("UQ")||isSgVarRefExp((*i))->get_symbol()->getAttribute("UQ")){
			replace_Vari_declaration(*i);
			//cout <<"replace_Vari_declaration done\n";
			replace_Vari_Reference(*i);
			//cout <<"replace_Vari_Reference done\n";
		}

	}
	//For init statement : int i = 0;
	SgExpression*         init_exp = buildAssignOp(buildVarRefExp("iSample"),buildIntVal(1));
	//For Condition statement: i<10;
	//SgExprStatement* cond_stmt = buildExprStatement(buildLessThanOp(buildVarRefExp("iSample"),buildIntVal(10)));
	SgExpression*         bound    = buildIntVal(10);
	//For increment statement: i++;
	//SgExpression*         increment    = buildNullExpression();//buildIntVal(1);
	SgExpression*         increment    = buildNullExpression();
	// basicblock would embrace the statements	
	SgBasicBlock* For_block = buildBasicBlock(buildExprStatement(isSgExpression(newNode)));
	//build For body by the duplicated Ast Assing node
	SgFortranDo* NewStatement = buildFortranDo(init_exp,bound,increment,For_block);
	NewStatement->set_has_end_statement(true);
	//cout <<NewStatement->unparseToString()<<"\n";
	Parent_scope->replace_statement(currentStmt,NewStatement);
}


void VectorzingForStatement(SgAssignOp* Assign, SgExprStatement* currentStmt, SgFortranDo* UpperMostFor){
	SgTreeCopy tc;
	//SgNode* newNode = Assign->copy(tc);
	//SgFortranDo* UpperMostFor = get_UpperMost_for(isSgFortranDo(Parent_scope->get_parent()));
	if(UpperMostFor->getAttribute("Modified_For")==NULL){
	SgNode* newNode = UpperMostFor->copy(tc);
	Rose_STL_Container<SgNode*> VarVector = NodeQuery::querySubTree(newNode,V_SgVarRefExp);
	for(Rose_STL_Container<SgNode*>::iterator i = VarVector.begin();i!=VarVector.end();i++){
		if(isSgVarRefExp((*i))->getAttribute("UQ")||isSgVarRefExp((*i))->get_symbol()->getAttribute("UQ")){
			if(isSgPntrArrRefExp((*i)->get_parent())){
			SgPntrArrRefExp* Vari_pr = find_pointer_Vari(*i);
			replace_Vari_declaration(*i);
			//cout <<"replace_Vari_declaration done\n";
			replace_Vari_Reference(Vari_pr);
			//cout <<"replace_Vari_Reference done\n";
			}else{
				replace_Vari_declaration(*i);
				replace_Vari_Reference(*i);
			}
		}

	}
	//For init statement : int i = 0;
	//SgAssignInitializer* init_exp =buildAssignInitializer(buildIntVal(0),buildIntType());
    //SgVariableDeclaration* init_stmt =buildVariableDeclaration("iSample",buildIntType(),init_exp);
	SgExpression*         init_exp = buildAssignOp(buildVarRefExp("iSample"),buildIntVal(0));
	//For Condition statement: i<10;
	//SgExprStatement* cond_stmt = buildExprStatement(buildLessThanOp(buildVarRefExp("iSample"),buildIntVal(10)));
	SgExpression*         bound    = buildIntVal(10);
	//For increment statement: i++;
	//SgExpression* incr_exp = buildPlusPlusOp(buildVarRefExp("iSample"),SgUnaryOp::postfix);
	SgExpression*         increment    = buildNullExpression();
	//build basic block to include all statments in the for loop
	SgBasicBlock* for_block = buildBasicBlock(isSgStatement(newNode));
	//build For body by the duplicated Ast Assing node
	
	
	SgFortranDo* NewStatement = buildFortranDo(init_exp,bound,increment,for_block);
	NewStatement->set_has_end_statement(true);
	isSgStatement(UpperMostFor->get_parent())->replace_statement(UpperMostFor,NewStatement);
	Set_AstNode_Atrribute("Modified_For",NewStatement);
	Set_AstNode_Atrribute("Modified_For",UpperMostFor);
	}
}

void addVariable(string Name,SgScopeStatement* scope){
	SgSymbolTable* symboltable = scope->get_symbol_table();
	SgVariableSymbol* Vari     = symboltable->find_variable(Name);
	
	//SgVariableDeclaration* vari_decl = buildVariableDeclaration(Name,buildIntType());
	SgVariableDeclaration* variableDeclaration = new SgVariableDeclaration(FILE_SOURCE,Name,buildIntType());

	if(!Vari && !scope->statementExistsInScope (variableDeclaration)){
		SgInitializedName* initializedName = *(variableDeclaration->get_variables().begin());
		initializedName->set_file_info(FILE_SOURCE);
		initializedName->set_scope(scope);
		variableDeclaration->set_firstNondefiningDeclaration(variableDeclaration);
		isSgBasicBlock(scope)->get_statements().insert(next(isSgBasicBlock(scope)->get_statements().begin(),1),variableDeclaration);
		variableDeclaration->set_parent(scope);	
		SgVariableSymbol* variableSymbol = new SgVariableSymbol(initializedName);
		scope->insert_symbol(Name,variableSymbol);
	}
}

void transformCode(SgProject* project){
	//get all assignment in the project
	Rose_STL_Container<SgNode*> AssignmentVector = NodeQuery::querySubTree(project,V_SgExprStatement);	
	for(Rose_STL_Container<SgNode*> ::iterator i =AssignmentVector.begin();i !=AssignmentVector.end();i++){
		//if the 1.left hand operand has UQ or 2.the lhs symbol has UQ
		//SgScopeStatement* Scope_old = NULL;
		//SgScopeStatement* Scope = NULL;
		SgAssignOp* AssignOp = isSgAssignOp(isSgExprStatement((*i))->get_expression());
		//SgNode* Parent 		 = (*i)->get_parent();
		//Other Op would apear, make sure we are dealing with AssignOp.
		if(AssignOp){
			//if there is an AssignOp
			//there are three possible situations:
			//1. rhs is functioncall(special handle)
			//2. rhs is normal assignment with variable(SgVarRefExp, need to be vectorized)
			//3. rhs is constant value(no need to be vectorized for now!)
			SgExpression* rhs_node = AssignOp->get_rhs_operand_i();
			
			// situation 1!
			if(isSgFunctionCallExp(rhs_node)){
			}
			else
			{
				//Situation 2! there is no Situation 3 explicitly written here!
				if(!isSgValueExp(rhs_node)){
					SgScopeStatement* Parent    	 = isSgStatement((*i))->get_scope();
					SgFortranDo* ForNode             = isSgFortranDo(Parent->get_parent());
			    	SgExprStatement* currentStatement = isSgExprStatement(*i); 	
					//if there is already forstatement,use for vectorization
					if(ForNode){
						SgVarRefExp* Left_inner_var = get_LeftInnerMost_Var(AssignOp);
						if(get_LeftInnerMost_Var(AssignOp)->getAttribute("UQ")
								||get_LeftInnerMost_Var(AssignOp)->get_symbol()->getAttribute("UQ")){
									SgFortranDo* UpperMost_scope = get_UpperMost_for(isSgFortranDo(Parent->get_parent()));
									addVariable("iSample",UpperMost_scope->get_scope());
									VectorzingForStatement(AssignOp,currentStatement,UpperMost_scope);}
					}
					else{
						//otherwise just use normal one
					    SgVarRefExp* Left_inner_var = get_LeftInnerMost_Var(AssignOp);
        	    	    if(get_LeftInnerMost_Var(AssignOp)->getAttribute("UQ")
        	    	             ||get_LeftInnerMost_Var(AssignOp)->get_symbol()->getAttribute("UQ")){
									 addVariable("iSample",Parent);
        	    	                 VectorzingStatement(AssignOp,currentStatement,Parent);}
			
						}
				}
			}
		}//if assing op
//		SgGreaterOrEqualOp* GreaterOrEqual = isSgGreaterOrEqualOp(isSgExprStatement((*i))->get_expression());
//		if(GreaterOrEqual){
//			SgVarRefExp* Left_inner_var = isSgVarRefExp(GreaterOrEqual->get_lhs_operand_i());
//			SgVarRefExp* Right_inner_var = isSgVarRefExp(GreaterOrEqual->get_rhs_operand_i());
//			if(Left_inner_var->get_symbol()->getAttribute("UQ")||Right_inner_var->get_symbol()->getAttribute("UQ")){
//						addVariable("iSample",GreaterOrEqual->get_parent()->get_scope());
//						VectorzingIfStatement()
//					}
//		}//if Greater op Equal
		SgFunctionCallExp* FunctionCall = isSgFunctionCallExp(isSgExprStatement((*i))->get_expression());
		if(FunctionCall){
			//cout <<"found subroutine call at transform\n";
			//cout << FunctionCall->unparseToString()<<"\n";
			SgExprListExp* ArgList = FunctionCall->get_args();
			SgExpressionPtrList &exprs = ArgList->get_expressions();
			size_t NumberInput  = ArgList->get_numberOfTraversalSuccessors();
			for (size_t i = 0;i <NumberInput;i++){
				SgVarRefExp* varExp = isSgVarRefExp(exprs[i]);
				if(varExp){
                AstAttribute* var_attribute = varExp->get_symbol()->getAttribute("UQ");
                if((var_attribute!=NULL)){
					replace_Vari_declaration(varExp);
                	}
            	}
        	}

		}//if functioncall
	}//for statement

	Rose_STL_Container<SgNode*> AllocateVector = NodeQuery::querySubTree(project,V_SgAllocateStatement);
	for(Rose_STL_Container<SgNode*> ::iterator i =AllocateVector.begin();i !=AllocateVector.end();i++){
		SgAllocateStatement* Allocate = isSgAllocateStatement(*i);
		SgExprListExp* ExprList = isSgExprListExp(Allocate->get_expr_list());
        SgExpressionPtrList &exprs = ExprList->get_expressions();
        size_t NumberInput  = ExprList->get_numberOfTraversalSuccessors();
        for (size_t i = 0;i <NumberInput;i++){
                  SgPntrArrRefExp* PointerVari = isSgPntrArrRefExp(exprs[i]);
				  SgVarRefExp* Vari = isSgVarRefExp(PointerVari->get_lhs_operand_i());
				  if(Vari->getAttribute("UQ")||Vari->get_symbol()->getAttribute("UQ")){
				  	SgIntVal* extra_dim = buildIntVal(10);
					SgExprListExp* rhs_index = isSgExprListExp(PointerVari->get_rhs_operand_i());
					rhs_index->prepend_expression(extra_dim);
				  }
		}

	}//for Statement Allocate
	Rose_STL_Container<SgNode*> IOStatement = NodeQuery::querySubTree(project,V_SgWriteStatement);
	for(Rose_STL_Container<SgNode*> ::iterator i =IOStatement.begin();i !=IOStatement.end();i++){
		Rose_STL_Container<SgNode*>  Var= NodeQuery::querySubTree(*i,V_SgVarRefExp);
		for(Rose_STL_Container<SgNode*>::iterator j = Var.begin();j!=Var.end();j++){
			SgVarRefExp* VarRef = isSgVarRefExp(*j);
			if(VarRef->getAttribute("UQ")||VarRef->get_symbol()->getAttribute("UQ")){
				SgPntrArrRefExp* ArrayPntr = isSgPntrArrRefExp(VarRef->get_parent());
				if(ArrayPntr){
					if(ArrayPntr->get_rhs_operand_i()){
						SgExprListExp* IndexList = isSgExprListExp(ArrayPntr->get_rhs_operand_i());
						//SgExprListExp* IOList = isSgExprListExp(ArrayPntr->get_parent()); 
						SgNullExpression* NullExp = buildNullExpression();
						SgIntVal* stride = buildIntVal(1);
						SgSubscriptExpression* NewSub = buildSubscriptExpression_nfi(NullExp,NullExp,stride);
						IndexList->prepend_expression(NewSub);
					}
				}
			}
		}//inner for loop for Variable
	}//out loop For IOStatement
	
	//do a final check
    Rose_STL_Container<SgNode*> IfStmt = NodeQuery::querySubTree(project,V_SgIfStmt);	
	for(Rose_STL_Container<SgNode*> ::iterator i =IfStmt.begin();i !=IfStmt.end();i++){
		SgIfStmt* IfStatement = isSgIfStmt(*i);
		SgExprStatement* Expr_Stmt = isSgExprStatement(IfStatement->get_conditional());
		Rose_STL_Container<SgNode*>  Var= NodeQuery::querySubTree(Expr_Stmt,V_SgVarRefExp);
		for(Rose_STL_Container<SgNode*>::iterator j = Var.begin();j!=Var.end();j++){
			SgVarRefExp* VarRef = isSgVarRefExp(*j);
			if((VarRef->getAttribute("UQ")||VarRef->get_symbol()->getAttribute("UQ"))
					&&(!Expr_Stmt->getAttribute("print"))){
				cout <<"found UQ in if branch\n";
				cout <<"at file:"<<Expr_Stmt->get_file_info()->get_filename()<<" at line:"<<Expr_Stmt->get_file_info()->get_line()<<"\n";
				cout <<Expr_Stmt->unparseToString()<<"\n";
				cout <<"remains unchanged, you can modify it to fit your expect\n";
				Set_AstNode_Atrribute("print",Expr_Stmt);				
			}
		}
	}


}

