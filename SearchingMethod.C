#include "SearchingMethod.h"
#include "HelpingFuncs.h"
#include <vector>
#include <algorithm>
using namespace std;


 subvisitorTraversal::subvisitorTraversal(){
 }
void subvisitorTraversal::atTraversalEnd() 
{
}

FortranFunctionTraversal::FortranFunctionTraversal(SgFunctionCallExp* FuncCall){
	//assign outside given Funcall to private variable FunctionCall header
	FunctionCall = FuncCall;
	//extract Function output variable name and assign it to class private variable
	SgProcedureHeaderStatement* FunctionHeader = FromCallToDecl(FuncCall);
	if(FunctionHeader->get_result_name()){
		//Function has explicit output variable
		SgInitializedName* FuncResultInitname = FunctionHeader->get_result_name();
		OutputName  =  FuncResultInitname->get_name();
	}else{
		//Function has not explicit output variable:means function name itself should be check at end
		OutputName  =  FunctionHeader->get_mangled_name();
	}
}

void FortranFunctionTraversal::atTraversalEnd(){
	SgFunctionDefinition* FuncDefi = FromCallToDecl(FunctionCall)->get_definition();
	SgVariableSymbol* OutputSymbol = FuncDefi->lookup_var_symbol(OutputName);
	if(OutputSymbol->getAttribute("UQ")){
		OutputContainsUQ = true;
		AstAttribute* existAttri = OutputSymbol->getAttribute("UQ");
		OutputUQType = dynamic_cast<persistantAttribute*>(existAttri)->value;
	}

}

vector<int> search_FortranFunction(SgFunctionCallExp* FuncCall){
	vector<vector<int>> UQ_Type_in_ParaList;
	vector<int> UQposi = CountUQPosi(FuncCall->get_args(),&UQ_Type_in_ParaList);
	SgProcedureHeaderStatement* FunHeader = FromCallToDecl(FuncCall);

	if(UQposi.size()>0 && FunHeader!=NULL){
		vector<string> UQName = get_UQName_InFunction(FunHeader,UQposi);
		Setting_Token_InFUnction(FunHeader,UQName,UQ_Type_in_ParaList);
		//different to the Subroutine.
		//here the function searching uses another subtree Traversal
		FortranFunctionTraversal innerFortranFuncSearch(FuncCall);
		innerFortranFuncSearch.traverse(FunHeader->get_definition(),preorder);
		//no need to update the callExp list
		//output check is performed instead
		if(innerFortranFuncSearch.OutputContainsUQ){
			return innerFortranFuncSearch.OutputUQType;
		}
	}else{
		if(!isFuncSearched(FuncCall) && (FunHeader!=NULL)){
			PresearchUQinFunc(FunHeader,FuncCall);
		}
		if(FunHeader!=NULL){
			if(isFuncHasUQ(FuncCall)){
				//same operation as before
				FortranFunctionTraversal innerFortranFuncSearch(FuncCall);
    	        innerFortranFuncSearch.traverse(FunHeader->get_definition(),preorder);
    	        //update the call exp
    	        //no need to update the callExp list
				//output check is performed instead
				if(innerFortranFuncSearch.OutputContainsUQ)
				{
					return innerFortranFuncSearch.OutputUQType;
				}
			}
		}
	}
	if(FunHeader==NULL){
		//meaning the function is probabily a system function such
		// int, float, size, etc...
		//compare the function name with given function databank(an array with function name that you want to enable)
		//incldung math opration like sin, cos, log ...
		SgName FunctionName = extractFunctionName(FuncCall);
		bool isMathOp = isFunctionInBuildInList(FunctionName);
		if(isMathOp){
			//get UQ_type from rhs
			vector<int> UQ_type = GetUQTypeUnderSubtree(FuncCall);
			//update left hand side
			return UQ_type;
		}else{
			cout<<"find function call which is not recognizeable"<<"\n";
			cout <<FuncCall->unparseToString()<<"\n";
			cout <<"in file: "<<FuncCall->get_file_info()->get_filename()<<" line :"<<FuncCall->get_file_info()->get_line()<<"\n";
		}
	}
	return {};

}

void search_Statement(SgNode* n){
	SgAssignOp* assig = isSgAssignOp(n);
	// bool containsUQ = false;
	if(!isSgFunctionCallExp(assig->get_rhs_operand_i())){
		//if right side is not funtion calling, means 
		//it is some Arithmetic operation with variable reference.
		
		vector<int> UQ_type = GetUQTypeUnderSubtree(assig->get_rhs_operand_i());
		
		if(UQ_type.size()>0){
			vector<int> Left_Type = reduce_redudent_Type(UQ_type); 
			SgVarRefExp* Var_InLHS; 
			//if left hand side contains type expression
			if(isSgDotExp(assig->get_lhs_operand_i())){
				Var_InLHS = get_RightInnerMost_Var(assig->get_lhs_operand_i());
			}else{//else deal as usual
				 Var_InLHS = get_LeftInnerMost_Var(assig); 
			}
			if(return_Var_AstAttribute(Var_InLHS,"UQ")){
				Set_VarRef_UQ_Atrribute_Union2Sides(Var_InLHS,Left_Type);	
			}else{
				Set_VarRef_UQ_Atrribute(Var_InLHS,Left_Type);
			}
		}
	}else{
		//now dealing with function calling rhs
		SgFunctionCallExp* FortranFunction = isSgFunctionCallExp(assig->get_rhs_operand_i());
		SgVarRefExp* Var_InLHS; 
		vector<int> UQ_Type = search_FortranFunction(FortranFunction);
		//if left hand side contains type expression
		if(isSgDotExp(assig->get_lhs_operand_i())){
			Var_InLHS = get_RightInnerMost_Var(assig->get_lhs_operand_i());
		}else{//else deal as usual
			 Var_InLHS = get_LeftInnerMost_Var(assig); 
		}
		if(UQ_Type.size()>0){
			if(return_Var_AstAttribute(Var_InLHS,"UQ")){
				Set_VarRef_UQ_Atrribute_Union2Sides(Var_InLHS,UQ_Type);	
			}else{
				Set_VarRef_UQ_Atrribute(Var_InLHS,UQ_Type);
			}
		}
	}
}





void UpdateFatherNode(SgFunctionCallExp* FuncCall,SgProcedureHeaderStatement* SubRoutineHeader){
	vector<int> UQ_posi_inSubRoutine;
	vector<vector<int>> UQ_type_inSubRoutine;
	SgFunctionParameterList* FuncPara = SubRoutineHeader->get_parameterList();
    SgInitializedNamePtrList &VarList = FuncPara->get_args();
	for (size_t i =0 ; i<VarList.size();i++){
		SgVariableSymbol* VarSymbol = isSgVariableSymbol(VarList[i]->get_symbol_from_symbol_table());
		if(VarSymbol->getAttribute("UQ")){
			UQ_posi_inSubRoutine.push_back(i);
			AstAttribute* existAttri = VarSymbol->getAttribute("UQ");
			UQ_type_inSubRoutine.push_back(dynamic_cast<persistantAttribute*>(existAttri)->value);
		}
	}
	//if there is UQ in header
	if(UQ_posi_inSubRoutine.size()>0){
		SgExprListExp* FuncCallList = FuncCall->get_args();
		SgExpressionPtrList &exprs = FuncCallList->get_expressions();
		for(size_t i = 0;i<UQ_posi_inSubRoutine.size();i++){
			SgVarRefExp* varExp = get_VarRefExp(exprs[UQ_posi_inSubRoutine[i]]);
			if(return_Var_AstAttribute(varExp,"UQ")){
				Set_VarRef_UQ_Atrribute_Union2Sides(varExp,UQ_type_inSubRoutine[i]);			    
			}else{
				Set_VarRef_UQ_Atrribute(varExp,UQ_type_inSubRoutine[i]);
			}
		}
	}
}





void search_FunctionCall(SgNode* n){
	SgFunctionCallExp* FuncCall = isSgFunctionCallExp(n);
	vector<vector<int>> UQ_Type_in_ParaList;
	vector<int> UQposi = CountUQPosi(FuncCall->get_args(),&UQ_Type_in_ParaList);
	SgProcedureHeaderStatement* FunHeader = FromCallToDecl(FuncCall);
	//be aware of that FunHeader can be NULL when it is:
	//1. system defined call
	//2. Libary call
	//if it is in this case, further searching is not required
	if(UQposi.size()>0 && FunHeader!=NULL){
		vector<string> UQName = get_UQName_InFunction(FunHeader,UQposi);
		Setting_Token_InFUnction(FunHeader,UQName,UQ_Type_in_ParaList);
		subvisitorTraversal innerFuncSearch;
		innerFuncSearch.traverse(FunHeader->get_definition(),preorder);
		//update the call exp
		cout<<"in search_FunctionCall searching function"<<FunHeader->unparseToString()<<"\n";
		if(FunHeader->isSubroutine()){
			UpdateFatherNode(FuncCall,FunHeader);
		}
	}else{
		if(!isFuncSearched(FuncCall) && (FunHeader!=NULL)){
			PresearchUQinFunc(FunHeader,FuncCall);
		}
		if(FunHeader!=NULL){
			if(isFuncHasUQ(FuncCall)){
				subvisitorTraversal innerFuncSearch;
    	        innerFuncSearch.traverse(FunHeader->get_definition(),preorder);
    	        //update the call exp
    	        cout<<"in search_FunctionCall searching function"<<FunHeader->unparseToString()<<"\n";
				if(FunHeader->isSubroutine())
				{
					UpdateFatherNode(FuncCall,FunHeader);
				}
			}
		}
		return;
	}
}
void search_DoBlock(SgNode* n){
	SgFortranDo* FortranDo = isSgFortranDo(n);
	cout <<"FortranDo is"<<FortranDo->unparseToString()<<"\n";
	SgExpression* DoBound  = FortranDo->get_bound();
	SgAssignOp* InitOp     = isSgAssignOp(FortranDo->get_initialization());
//get interator SgVarRefExp
	cout <<" in do block"<<"\n";
	SgVarRefExp* interator = isSgVarRefExp(InitOp->get_lhs_operand_i());
	cout <<"sucessfully called get_lhs\n";
	Rose_STL_Container<SgNode*> VarlistInBound = NodeQuery::querySubTree(DoBound,V_SgVarRefExp);
	for(Rose_STL_Container<SgNode*>::iterator i = VarlistInBound.begin();i!=VarlistInBound.end();i++){
                  SgVarRefExp* Vari = isSgVarRefExp((*i));
                  if(Vari->getAttribute("UQ")||Vari->get_symbol()->getAttribute("UQ")){
					  //give lhs UQ atrribute according to the UQ from right;
					  AstAttribute* existAttri = return_Var_AstAttribute(Vari,"UQ");
					  vector<int> Buffer_Vec = dynamic_cast<persistantAttribute*>(existAttri)->value;
					  Set_VarRef_UQ_Atrribute(interator,Buffer_Vec);
           }
     }


}

void Searching_WritingStatement(SgNode* n){
	SgWriteStatement* WriteStmt = isSgWriteStatement(n);
	SgExprListExp* ExpList = WriteStmt->get_io_stmt_list();
	SgExpressionPtrList &exprs = ExpList->get_expressions();
	vector<int> UQ_Type = GetUQTypeUnderSubtree(ExpList);
	SgVarRefExp* lhsVari = isSgVarRefExp(WriteStmt->get_unit());
	if(UQ_Type.size()>0){
		if(return_Var_AstAttribute(lhsVari,"UQ")){
			Set_VarRef_UQ_Atrribute_Union2Sides(lhsVari,UQ_Type);			    
		}else{
			Set_VarRef_UQ_Atrribute(lhsVari,UQ_Type);
		}
	}
}
/*void search_ReturnStmt(SgNode* n){
	SgReturnStmt* ReturnStmt = isSgReturnStmt(n);
	SgExpression* ReturnExpr = ReturnStmt->get_expression();
	if(isSgNullExpression(n)){
		// if no specific output, meaning the function name should be checked!
		SgBasicBlock* FuncBlock = isSgBasicBlock(ReturnStmt->get_parent());
		SgFunctionDefinition* FuncDefi = isSgFunctionDefinition(FuncBlock->get_parent());
		SgName FuncName = FuncDefi->get_mangled_name();
		SgVariableSymbol* ReturnSymbol = NewScope-> lookup_var_symbol(FuncName);
		if(ReturnSymbol->getAttribute("UQ")){
			subvisitorTraversal.hasUQFunction=true;
		}else{
			
		
		}

}*/
 void subvisitorTraversal:: visit(SgNode *n)
 {
	if(isSgAssignOp(n)!=NULL){
		 search_Statement(n);
	}


	 if(isSgFunctionCallExp(n)!=NULL){
		 // searching only for Subroutine
		 // Function Searching is implemented in another Subvisitor because
		 // Function is related with Assigment mostly, then assign brach should 
		 // handle it already
		 SgFunctionCallExp* FuncCall = isSgFunctionCallExp(n);
		 SgProcedureHeaderStatement* FuncHeader = FromCallToDecl(FuncCall);

		 if(FuncHeader!=NULL && FuncHeader->isSubroutine()){
		 	search_FunctionCall(n);
		 }
		 /*else{
		 	search_FortranFunction(n);
		 }*/
		 
	 }
	 if(isSgFortranDo(n)){
	 	//cout <<n->unparseToString()<<"\n";
	 	//search_DoBlock(n);
	 }
	 if(isSgWriteStatement(n)){
	 	Searching_WritingStatement(n);
	 }
//	 if(isSgReturnStmt(n)){
//		search_ReturnStmt(n);
//	 }

}

void FortranFunctionTraversal::visit(SgNode* n)
 {
	if(isSgAssignOp(n)!=NULL){
		 search_Statement(n);
	}


	 if(isSgFunctionCallExp(n)!=NULL){
		 // searching only for Subroutine
		 // Function Searching is implemented in another Subvisitor because
		 // Function is related with Assigment mostly, then assign brach should 
		 // handle it already
		 SgFunctionCallExp* FuncCall = isSgFunctionCallExp(n);
		 SgProcedureHeaderStatement* FuncHeader = FromCallToDecl(FuncCall);

		 if(FuncHeader!=NULL && FuncHeader->isSubroutine()){
		 	search_FunctionCall(n);
		 }
		 /*else{
		 	search_FortranFunction(n);
		 }*/
		 
	 }
	 if(isSgFortranDo(n)){
	 	//search_DoBlock(n);
	 }
	 if(isSgWriteStatement(n)){
	 	Searching_WritingStatement(n);
	 }
//	 if(isSgReturnStmt(n)){
//		search_ReturnStmt(n);
//	 }

}


FuncPreProcess::FuncPreProcess(vector<string> input){
	UQName = input;
}

void FuncPreProcess::visit(SgNode* n){
	if(isSgVarRefExp(n)){
		string VarName = isSgVarRefExp(n)->get_symbol()->get_name();
		for (size_t i = 0; i <UQName.size();i++){
			if(VarName.compare(UQName[i])==0){
				 AstAttribute* newAttribute = new persistantAttribute({1});
                 n->addNewAttribute("UQ",newAttribute);
				 isSgVarRefExp(n)->get_symbol()->addNewAttribute("UQ",newAttribute);
				 n->addNewAttribute("UQ",newAttribute);

			}
		}
	}
}

/*void output_Judge(SgNode* node){
		Rose_STL_Container<SgNode*> VarVector = NodeQuery::querySubTree(node,V_SgVarRefExp);
		for(Rose_STL_Container<SgNode*>::iterator i = VarVector.begin();i!=VarVector.end();i++){
                 SgVarRefExp* Vari = isSgVarRefExp((*i));
                 if(Vari->getAttribute("UQ")||Vari->get_symbol()->getAttribute("UQ")){
					outputflag = true;
				 	UQname.push_back(Vari->get_symbol()->get_name());
				 }
		}
		if(outputflag){
			cout <<"in file: "<<node->get_file_info()->get_filename()<<" line :"<<node->get_file_info()->get_line()<<"\n";
			cout <<"statement: "<<node->unparseToString()<<"\n";
			cout <<"with following Varible constain UQ: ";
			for(vector<string>::iterator it=UQname.begin();it!=UQname.end();it++){
				cout << *it <<" ";
			}
			cout <<"\n";
		}
}*/


  void readAttribute:: visit(SgNode* node){
	if(isSgAssignOp(node)){
		bool outputflag = false;
		vector<string> UQname;
		vector<vector<int>> UQ_Type;
		Rose_STL_Container<SgNode*> VarVector = NodeQuery::querySubTree(node,V_SgVarRefExp);
		for(Rose_STL_Container<SgNode*>::iterator i = VarVector.begin();i!=VarVector.end();i++){
                 SgVarRefExp* Vari = isSgVarRefExp((*i));
                 if(return_Var_AstAttribute(Vari,"UQ")){
					outputflag = true;
				 	UQname.push_back(Vari->get_symbol()->get_name());
					 AstAttribute* existAttri = return_Var_AstAttribute(Vari,"UQ");
                     UQ_Type.push_back(dynamic_cast<persistantAttribute*>(existAttri)->value);
				 }
		}
		if(outputflag){
			cout <<"in file: "<<node->get_file_info()->get_filename()<<" line :"<<node->get_file_info()->get_line()<<"\n";
			cout <<"statement: "<<node->unparseToString()<<"\n";
			cout <<"with following Varible constain UQ: ";
			for(vector<string>::iterator it=UQname.begin();it!=UQname.end();it++){
				cout << *it <<" ";
			}
			cout <<"\n";
			for(size_t i = 0; i <UQname.size();i++){
				cout <<UQname[i]<<" with type ";
				for (size_t j = 0;j<UQ_Type[i].size();j++)	
				{cout << UQ_Type[i][j]<<" ";}
				cout <<"\n";
			}
			cout <<"\n";
		}
			

	} 
if(isSgFunctionCallExp(node) && isSgFunctionCallSubroutine(isSgFunctionCallExp(node))){
		bool outputflag = false;
		vector<string> UQname;
		vector<vector<int>> UQ_Type;
		Rose_STL_Container<SgNode*> VarVector = NodeQuery::querySubTree(isSgFunctionCallExp(node)->get_args(),V_SgVarRefExp);
		for(Rose_STL_Container<SgNode*>::iterator i = VarVector.begin();i!=VarVector.end();i++){
                 SgVarRefExp* Vari = isSgVarRefExp((*i));
                 if(Vari->getAttribute("UQ")||return_Var_AstAttribute(Vari,"UQ")){
					outputflag = true;
				 	UQname.push_back(Vari->get_symbol()->get_name());
					AstAttribute* existAttri = return_Var_AstAttribute(Vari,"UQ");
					UQ_Type.push_back(dynamic_cast<persistantAttribute*>(existAttri)->value);
				 }
		}
		if(outputflag){
			cout <<"in file: "<<node->get_file_info()->get_filename()<<" line :"<<node->get_file_info()->get_line()<<"\n";
			cout <<"statement: "<<node->unparseToString()<<"\n";
			cout <<"with following Varible constain UQ: ";
			for(vector<string>::iterator it=UQname.begin();it!=UQname.end();it++){
				cout << *it <<" ";
			}
			cout <<"\n";
			for(size_t i = 0; i <UQname.size();i++){
				cout <<UQname[i]<<" with type ";
				for (size_t j = 0;j<UQ_Type[i].size();j++)	
				{cout << UQ_Type[i][j]<<" ";}
				cout <<"\n";
			}
			cout <<"\n";
		}
			

	}
}


