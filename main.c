#include<stdio.h>
#include"common.h"
#include<assert.h>
#include"syntax_tree.h"
void yyrestart();
void yyparse();
int errorstatus = 0;
void AnalyseProgram(Treenode *);
int main(int argc, char** argv){
	if(argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	if(!f)
	{
		perror(argv[1]);
		return 1;
	}
	SymbolTableInit();
	TypeInit();
	yyrestart(f);
	yyparse();
	if (!errorstatus){
		//print(root, 0);	
		AnalyseProgram(root);
	}
	return 0;
}
