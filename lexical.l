%{
	#include <stdio.h>
	#include <string.h>
	#include "common.h"
	#include "syntax.tab.h"
	#define YYSTYPE Treenode*
	#include "syntax_tree.h"
	int yycolumn = 1;
	#define YY_USER_ACTION do{ \
		yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng; \
	}while(0);
	#define operate(token) do{ \
		yylval = newtoken(token); \
		(*yylval).name = toArray(#token); \
		if (token == FLOAT) (*yylval).floatval = atof(yytext); \
		return token; \
	}while(0)
	Treenode *newtoken(int token){
		Treenode *p = newnode(token);
		(*p).lineno = yylineno;
		(*p).token = token;
		(*p).text = toArray(yytext);
		return p;
	}
%}
INT 0|[1-9]([0-9]){0,31}
INT16 0[xX]([0-9a-fA-F]){1,32}
INT8 0([0-7]){1,31}
FLOAT (0|[1-9][0-9]{0,31})\.([0-9]{1,32})|([0-9]+\.[0-9]+[Ee][+-]?[0-9]+)
ID [a-zA-Z_]([0-9a-zA-Z_]){0,31}
space [ \t]
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE int|float
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
%option yylineno
%%
"//" {
	char c = input();
	while (c != '\n') c = input();
}
"/*" {
	char c1 = input();
	char c2 = input();
	while ((c1 != '*') && c2 != '/'){
		c1 = c2;
		c2 = input();
		if(c2 < 0) break;
	}
}
{space} {}
{SEMI} {operate(SEMI);}
{COMMA} {operate(COMMA);}
{ASSIGNOP} {operate(ASSIGNOP);}
{RELOP} {operate(RELOP);}
{PLUS} {operate(PLUS);}
{MINUS} {operate(MINUS);}
{STAR} {operate(STAR);}
{DIV} {operate(DIV);}
{AND} {operate(AND);}
{OR} {operate(OR);}
{DOT} {operate(DOT);}
{NOT} {operate(NOT);}
{TYPE} {operate(TYPE);}
{LP} {operate(LP);}
{RP} {operate(RP);}
{LB} {operate(LB);}
{RB} {operate(RB);}
{LC} {operate(LC);}
{RC} {operate(RC);}
{STRUCT} {operate(STRUCT);}
{RETURN} {operate(RETURN);}
{IF} {operate(IF);}
{ELSE} {operate(ELSE);}
{WHILE} {operate(WHILE);}
{ID} {operate(ID);}
{INT} {
	yylval = newtoken(INT);
	(*yylval).name = "INT";
	(*yylval).intval = strtol(yytext, NULL, 10);
	return INT;}
{INT16} {
	yylval = newtoken(INT);
	(*yylval).name = "INT";
	(*yylval).intval = strtol(yytext, NULL, 16);	
	return INT;}
{INT8} {
	yylval = newtoken(INT);
	(*yylval).name = "INT";
	(*yylval).intval = strtol(yytext, NULL, 8);	
	return INT;}
{FLOAT} {operate(FLOAT);}
\n { yycolumn = 1; }
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); errorstatus = 1;}
%%
















