#include"inter_code.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

static Operand OperandPool[10000];

static Operand zero, one;
Operand *const CONST_ZERO = &zero;
Operand *const CONST_ONE = &one;

void OperandInit(){
	zero.kind = CONSTANT;
	zero.value = 0;
	one.kind = CONSTANT;
	one.value = 1;
}

Operand *newOperand(OperandKind kind){
	static int cnt = 0;
	Operand *p = &OperandPool[cnt++];
	p->kind = kind;
	p->name = p->text = NULL;
	return p;
}

int newVarOperandId(){
	static int cnt = 0;
	return ++cnt;
}

int newTempOperandId(){
	static int cnt = 0;
	return ++cnt;
}

int newLabelOperandId(){
	static int cnt = 0;
	return ++cnt;
}

Operand *ConstOperand(int val){
	Operand *p = newOperand(CONSTANT);
	p->value = val;
	return p;
}

Operand *VarOperand(int id){
	Operand *p = newOperand(VARIABLE);
	p->id = id;
	return p;
}

Operand *TempOperand(int id){
	Operand *p = newOperand(TEMP);
	p->id = id;
	return p;
}

Operand *LabelOperand(int id){
	Operand *p = newOperand(LABEL);
	p->id = id;
	return p;
}

Operand *newVarOperand(){
	return VarOperand(newVarOperandId());
}

Operand *newTempOperand(){
	return TempOperand(newTempOperandId());
}

Operand *newLabelOperand(){
	return LabelOperand(newLabelOperandId());
}

Operand *newFuncOperand(char *s){
	Operand *p = newOperand(FUNCT);
	p->name = s;
	return p;
}

#define GetStr(str, ...) do { \
	sprintf(buf, __VA_ARGS__); \
	str = malloc(strlen(buf) + 1); \
	strcpy(str, buf); \
	return str; \
} while (0)

char *OperandToStr(Operand *p){
	if (p == NULL) return NULL;
	//if (p->text == NULL) return p->text;
	static char buf[30];
	switch (p->kind){
	case TEMP:
		GetStr(p->text, "t%d", p->id);
	case VARIABLE:
		GetStr(p->text, "v%d", p->id);
	case CONSTANT:
		GetStr(p->text, "#%d", p->value);
	case LABEL:
		GetStr(p->text, "label%d", p->id);
	case FUNCT:
		GetStr(p->text, "%s", p->name);
	}
	return NULL;
}



















