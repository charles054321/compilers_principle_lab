#ifndef COMPILERS_TRANSLATE_H
#define COMPILERS_TRANSLATE_H

#include"syntax_tree.h"
#include"inter_code.h"
#include"symbol.h"

void InterCodeInit();
void InterCodeStackPush();
void InterCodeStackPop();
void InterCodeStackInsert(InterCode*);
InterCode *InterCodeStackGet();
InterCode *InterCodeGet();
InterCode *TranslateExp(Treenode*, Operand*);
InterCode *TranslateCompSt(Treenode*, FUNC*);
void defParams(FUNC*);
void defFunc(char*, InterCode*);

#endif
