#ifndef COMPILERS_SYMBOL_H
#define COMPILERS_SYMBOL_H

#include<stdbool.h>

typedef enum{
	BASIC, ARRAY, STRUCTURE
}TypeKind;

typedef enum{
	VAR, STRUCT, FUNC
}SymbolKind;

typedef struct TYPE{
	TypeKind kind;
	union{
		int basic;
		struct{
			struct TYPE *elem;
			int size;
		}array;
		FieldList structure;
	};
}TYPE;

extern TYPE *const TYPE_INT;
extern TYPE *const TYPE_FLOAT;

void TypeInit();

bool TypeEqual(Type *, Type *);

bool ArgsEqual(FieldList *, FieldList *);

bool FuncEqual(FUNC *, FUNC *);

void TypeRelease(Type *);

void ArgsRelease(FieldList *);

void FuncRelease(FUNC *);

typedef struct FieldList{
	char *name;
	TYPE *type;
	FieldList *next, *prev;
}FieldList;

typedef struct FUNC{
	TYPE *retType;
	FieldList args;
	bool defined;
}

typedef struct Symbol{
	char *name;
	SymbolKind kind;
	union{
		TYPE *type;
		FUNC *func;
	}
	int id;
	bool isRef;
	int depth;
}Symbol;

Symbol *NewVarSymbol(const char *, TYPE *);

Symbol *NewFuncSymbol(const char *, FUNC *);

Symbol *NewStructSymbol(const char *, TYPE *);

void argsinit(FieldList *);

FUNC *NewFunc(TYPE *);

void SymbolTableInit();

bool SymbolInsert(Symbol *);

bool SymbolAtStackTop(const char *);

Symbol SymbolFind(const char *);

void SymbolRelease(Symbol *);

void SymbolStackPush();

void SymbolStackPop();

FieldList *FieldFind(FieldList *, const char *);

void typeToStr(Type *, char *);

void ArgsToStr(FieldList *, char *);




















