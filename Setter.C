#include "SearchingMethod.h"
#include "HelpingFuncs.h"
#include <vector>
using namespace std;

//---------------------defining attribute setter---------------------
AttributeSetter::AttributeSetter(string filename){
	ifstream myfile(filename);
	string line;
	if(!myfile){
		cout <<"Error Opening input file "<<filename <<"\n";
		cout <<"please check it again\n";
		exit(1);
	}
	
/*	
 *	while(!myfile.eof()){
		getline (myfile,line);
		string word;

		InitUQName.push_back(line);
	}
*/
	int count = -1;
	string word;
/*	string line;
	if(myfile.is_open()){
		while(!myfile.eof()){
			getline(myfile,line);
			word[count] = line;
			count++;
		}
		myfile.close();
	}*/


	while(myfile >> word){
     	    count = count + 1;
		    if(count%2 == 0){
				InitUQFuncName.push_back(word);
			}	
			else{
				InitUQVarName.push_back(word);
			}
     	 }
	if(count%2 == 1){
		myfile.close();
	}else{
		cout <<"input file :"<<filename<<" has format error!"<<"\n";
		exit(0);
	}

}

void AttributeSetter::visit(SgNode* n){
	/*if(isSgBasicBlock(n)){
		SgSymbolTable* symbol_table =isSgBasicBlock(n)->get_symbol_table();
		//if(isSgScopeStatement(symbol_table)){
		//	cout << "setting is done\n";
		//SgVariableSymbol* symbol = isSgScopeStatement(symbol_table)->lookup_var_symbol("a");
		SgVariableSymbol* symbol = symbol_table->find_variable("a");
		AstAttribute* newAttribute = new persistantAttribute(1);
		symbol->addNewAttribute("UQ",newAttribute);
		//}
	}*/
	if(isSgBasicBlock(n)){
		SgNode* Parent = isSgBasicBlock(n)->get_parent()->get_parent();
		if(Parent!=NULL){
			SgFunctionDeclaration* FunDecl = isSgFunctionDeclaration(Parent);
			if(FunDecl!=NULL){
				std::string Funcname = FunDecl->get_name().getString();
				if(!Funcname.compare("test_trielem")){
					SgSymbolTable* symbol_table =isSgBasicBlock(n)->get_symbol_table();
					SgVariableSymbol* symbol;
					AstAttribute* newAttribute = new persistantAttribute({1});
					for(size_t i =0 ; i < InitUQVarName.size();i++){
						symbol = symbol_table->find_variable(InitUQVarName[i]);
						if(symbol!=NULL){
							symbol->addNewAttribute("UQ",newAttribute);
						}
					}
                  }

			}
		}
	}
}



void AttributeSetter :: setter(SgNode* n){

     Rose_STL_Container<SgNode*> FuncDecl =  NodeQuery::querySubTree(n,V_SgFunctionDeclaration);
     for(size_t iFunc = 0;iFunc<InitUQFuncName.size();iFunc++){
         string TargetFuncName = InitUQFuncName[iFunc];

         for(Rose_STL_Container<SgNode*>::iterator i = FuncDecl.begin() ;i!=FuncDecl.end();i++){
             SgFunctionDeclaration* FuncDdecl_body = isSgFunctionDeclaration(*i);
             if(!TargetFuncName.compare(FuncDdecl_body->get_name())){
                     if(FuncDdecl_body->get_definition()){
                         SgFunctionDefinition* FuncDefi = isSgFunctionDefinition(FuncDdecl_body->get_definition());
						 SgSymbolTable* symbol_table =isSgBasicBlock(FuncDefi->get_body())->get_symbol_table();
						 //SgSymbolTable* symbol_table =FuncDefi->get_symbol_table();
                         SgVariableSymbol* symbol;
                         AstAttribute* newAttribute = new persistantAttribute({iFunc});
                         symbol = symbol_table->find_variable(InitUQVarName[iFunc]);
						 if(symbol!=NULL){
							//varibale defined at function body
						 	symbol->addNewAttribute("UQ",newAttribute);
						 }else
						 {
	
							SgSymbolTable* symbol_table =FuncDefi->get_symbol_table();
							symbol = symbol_table->find_variable(InitUQVarName[iFunc]);
							if(symbol!= NULL){
								symbol->addNewAttribute("UQ",newAttribute);
							}else
							{
							 	cout <<"not found variable: "<<InitUQVarName[iFunc]<<"\n";
								cout <<"in Function: "<<InitUQFuncName[iFunc]<<"\n";
								cout <<"please check input file again\n";
								exit(0);
							}

						 }
                         
                 }
             }
         }
     }
 }



/*void AttributeSetter:: setter_module(SgNode* n){
	
	Rose_STL_Container<SgNode*> FuncDecl =  NodeQuery::querySubTree(n,V_SgFunctionDeclaration);
	Rose_STL_Container<SgNode*> ModuleDecl = NodeQuery::querySubTree(n,V_SgModuleStatement);
}*/
