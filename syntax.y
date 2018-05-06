%{
	#include <stdio.h>
	#include <stdarg.h>
	#include<assert.h>
	#include "syntax_tree.h"
	#include "common.h"
	#define YYSTYPE Treenode*
	#define operate(token, head, location, num, ...) \
	do { \
		head = asd(location.first_line, num, __VA_ARGS__); \
		(*head).name = toArray(#token); \
	}while(0)
	Treenode *asd(int location, int num, ...){
		va_list valist; 
		int i; 
		va_start(valist, num); 
		Treenode *p = newnode(); 
		int flag = 0;
		for (i = 0; i < num; i++){ 
			Treenode *q = va_arg(valist, Treenode*);
			if(q != NULL && flag == 0) {
				(*p).lineno = (*q).lineno;
				flag = 1;
			}
			if(q != NULL) treeAddChild(p, q);
		}
		va_end(valist);
		return p;
	}
%}
%token INT FLOAT ID
%token ASSIGNOP
%token OR
%token AND
%token RELOP
%token PLUS MINUS
%token STAR DIV
%token NOT
%token LP RP LB RB DOT
%token LC RC
%token TYPE STRUCT RETURN IF WHILE
%token COMMA SEMI
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/* High-level Definitions */
Program : ExtDefList {operate(Program, $$, @$, 1, $1); root = $$;}
	;
ExtDefList : ExtDef ExtDefList {operate(ExtDefList, $$, @$, 2, $1, $2);}
	| {$$ = NULL;}
	;
ExtDef : Specifier ExtDecList SEMI {operate(ExtDef, $$, @$, 3, $1, $2, $3);}
	| Specifier SEMI {operate(ExtDef, $$, @$, 2, $1, $2);}
	| Specifier FunDec CompSt {operate(ExtDef, $$, @$, 3, $1, $2, $3);}
	;
ExtDecList : VarDec {operate(ExtDecList, $$, @$, 1, $1);}
	| VarDec COMMA ExtDecList {operate(ExtDecList, $$, @$, 3, $1, $2, $3);}
	;
/* Specifiers */
Specifier : TYPE {operate(Specifier, $$, @$, 1, $1);}
	| StructSpecifier {operate(Specifier, $$, @$, 1, $1);}
	;
StructSpecifier : STRUCT OptTag LC DefList RC {operate(StructSpecifier, $$, @$, 5, $1, $2, $3, $4, $5);}
	| STRUCT Tag {operate(StructSpecifier, $$, @$, 2, $1, $2);}
	;
OptTag : ID {operate(OptTag, $$, @$, 1, $1);}
	| {$$ = NULL;}
	;
Tag : ID {operate(Tag, $$, @$, 1, $1);}
	;
/* Declarators */
VarDec : ID {operate(VarDec, $$, @$, 1, $1);}
	| VarDec LB INT RB {operate(VarDec, $$, @$, 4, $1, $2, $3, $4);}
	;
FunDec : ID LP VarList RP {operate(FunDec, $$, @$, 4, $1, $2, $3, $4);}
	| ID LP RP {operate(FunDec, $$, @$, 3, $1, $2, $3);}
	;
VarList : ParamDec COMMA VarList {operate(VarList, $$, @$, 3, $1, $2, $3);}
	| ParamDec {operate(VarList, $$, @$, 1, $1);}
	;
ParamDec : Specifier VarDec {operate(ParamDec, $$, @$, 2, $1, $2);}
	;
/* Statements */
CompSt : LC DefList StmtList RC {operate(CompSt, $$, @$, 4, $1, $2, $3, $4);}
	| error RC {}
	;
StmtList : Stmt StmtList {operate(StmtList, $$, @$, 2, $1, $2);}
	| {$$ = NULL;}
	;
Stmt : Exp SEMI {operate(Stmt, $$, @$, 2, $1, $2);}
	| CompSt {operate(Stmt, $$, @$, 1, $1);}
	| RETURN Exp SEMI {operate(Stmt, $$, @$, 3, $1, $2, $3);}
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {operate(Stmt, $$, @$, 5, $1, $2, $3, $4, $5);}
	| IF LP Exp RP Stmt ELSE Stmt {operate(Stmt, $$, @$, 7, $1, $2, $3, $4, $5, $6, $7);}
	| WHILE LP Exp RP Stmt {operate(Stmt, $$, @$, 5, $1, $2, $3, $4, $5);}
	| error SEMI {}
	;
/* Local Definitions */
DefList : Def DefList {operate(DefList, $$, @$, 2, $1, $2);}
	| {$$ = NULL;}
	;
Def : Specifier DecList SEMI {operate(Def, $$, @$, 3, $1, $2, $3);}
	;
DecList : Dec {operate(DecList, $$, @$, 1, $1);}
	| Dec COMMA DecList {operate(DecList, $$, @$, 3, $1, $2, $3);}
	;
Dec : VarDec {operate(Dec, $$, @$, 1, $1);}
	| VarDec ASSIGNOP Exp {operate(Dec, $$, @$, 3, $1, $2, $3);}
	;
/* Expressions */
Exp : Exp ASSIGNOP Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp AND Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp OR Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp RELOP Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp PLUS Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp MINUS Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp STAR Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp DIV Exp {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| LP Exp RP {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| MINUS Exp {operate(Exp, $$, @$, 2, $1, $2);}
	| NOT Exp {operate(Exp, $$, @$, 2, $1, $2);}
	| ID LP Args RP {operate(Exp, $$, @$, 4, $1, $2, $3, $4);}
	| ID LP RP {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| Exp LB Exp RB {operate(Exp, $$, @$, 4, $1, $2, $3, $4);}
	| Exp DOT ID {operate(Exp, $$, @$, 3, $1, $2, $3);}
	| ID {operate(Exp, $$, @$, 1, $1);}
	| INT {operate(Exp, $$, @$, 1, $1);}
	| FLOAT {operate(Exp, $$, @$, 1, $1);}
	| error RP {}
	;
Args : Exp COMMA Args {operate(Args, $$, @$, 3, $1, $2, $3);}
	| Exp {operate(Args, $$, @$, 1, $1);}
	;
%%
#include "lex.yy.c"
void yyerror(char* msg) {
	printf("Error type B at line %d: %s.\n", yylineno, msg);
	errorstatus = 2;
}












