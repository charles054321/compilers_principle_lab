#include<stdio.h>
#include"common.h"
#include<assert.h>
#include"syntax_tree.h"
#include"translate.h"
extern FILE *yyin;
void yyrestart();
void yyparse();
int errorstatus = 0;
void AnalyseProgram(Treenode *);

void init(){
	SymbolTableInit();
	TypeInit();
	OperandInit();
	InterCodeInit();
}

int main(int argc, char** argv){
	if(argc <= 2) return 1;
	FILE* f = fopen(argv[1], "r");
	freopen(argv[2], "w", stdout);
	if(!f)
	{
		perror(argv[1]);
		return 1;
	}
	init();
	yyrestart(f);
	yyparse();
	if (!errorstatus){
		//print(root, 0);
		AnalyseProgram(root);
		InterCodePrint(InterCodeGet());
	}
	return 0;
}
