// ROSE is a tool for building preprocessors, this file is an example preprocessor built with ROSE.
// rose.C: Example (default) ROSE Preprocessor: used for testing ROSE infrastructure

 #include "rose.h"
 #include "SearchingMethod.h"
 #include "PostProcessing.h"
 #include "string"
 #include "iostream"
 #include "fstream"
 #include "wholeAST.h"
// #include "DistributedMemoryAnalysis.h"
using namespace std;

 int
 main ( int argc, char* argv[] )
    {
   // Initialize and check compatibility. See Rose::initialize
      ROSE_INITIALIZE;

      if (SgProject::get_verbose() > 0)
           printf ("In visitorTraversal.C: main() \n");
	  argc--;
      SgProject* project = frontend(argc,argv);  //original form: (argc,argv). modification required cause the last arg is not program relevant
	  cout <<"parsing is done\n";
      ROSE_ASSERT (project != NULL);

   // Build the traversal object
      AttributeSetter setter(argv[argc]);
      subvisitorTraversal exampleTraversal;
	  readAttribute reader;
   // Call the traversal function (member function of AstSimpleProcessing)
   // starting at the project node of the AST, using a preorder traversal.
      setter.setter(project);  
	  cout <<"setting is done\n";
	  //reader.traverseInputFiles(project,preorder);
	  exampleTraversal.traverseInputFiles(project,preorder);
	  exampleTraversal.traverseInputFiles(project,preorder);
	  exampleTraversal.traverseInputFiles(project,preorder);
	  cout <<"searching is done\n";
	  reader.traverseInputFiles(project,preorder);
	  cout <<"reading is done\n";
	  //modifier.traverseInputFiles(project,postorder);
	  //transformCode(project); 
	  cout <<"Skip modification \n";

	  // generate WholeGraf for debugging
	  /*
	  std::string filename = SageInterface::generateProjectName(project);
      std::vector<std::string> argvList(argv, argv + argc);
	  generateWholeGraphOfAST(filename, new CustomMemoryPoolDOTGeneration::s_Filter_Flags(argvList));
	  */
	  //end grafe generating

//	  int fix_number = SageInterface::fixVariableReferences(project);
//	  cout <<"fix "<<fix_number<<"\n";
//	  project->unparse();
//      return backend(project);
	  return 0;
    }

