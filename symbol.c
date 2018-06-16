#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<stdbool.h>
#include"common.h"
#include"symbol.h"
#include"inter_code.h"

#define MASK     0x3FFF
#define SIZE     (MASK + 1)

typedef struct list{
	struct list *next, *prev;
}list;

typedef struct SymbolNode {
    Symbol *symbol;
    list chain, stack;
} SymbolNode;

static void listInit(list *a){
	assert(a != NULL);
	a->prev = a->next = a;
}

static void listAdd(list *prev, list *next, list *data) {
	assert(data != NULL);
	data->prev = prev;
	data->next = next;
	if (prev != NULL) prev->next = data;
	if (next != NULL) next->prev = data;
}

static void listAddBefore(list *List, list *data) {
	assert(List != NULL);
	listAdd(List->prev, List, data);
}

static void listAddAfter(list *List, list *data) {
	assert(List != NULL);
	listAdd(List, List->next, data);
}

static list SymbolTable[SIZE], stack[SIZE];
unsigned top;

static Symbol *NewSymbol(SymbolKind kind, const char *name){
	Symbol *symbol = (Symbol *) malloc(sizeof(Symbol));
	symbol->kind = kind;
	symbol->name = toArray(name);
	symbol->id = -1;
	symbol->isRef = false;
	return symbol;
}

Symbol *NewVarSymbol(const char *name, TYPE *type){
	Symbol *symbol = NewSymbol(VAR, name);
	symbol->type = type;
	return symbol;
}

Symbol *NewFuncSymbol(const char *name, FUNC *func){
	Symbol *symbol = NewSymbol(FUNCTION, name);
	symbol->func = func;
	return symbol;
}

Symbol *NewStructSymbol(const char *name, TYPE *type){
	Symbol *symbol = NewSymbol(STRUCT, name);
	symbol->type = type;
	return symbol;
}

void argsinit(FieldList *list){
	assert(list != NULL);
	list->next = list->prev = list;
}

FUNC *NewFunc(TYPE *retType){
	FUNC *func = (FUNC *) malloc(sizeof(FUNC));
	argsinit(&(func->args));
	func->retType = retType;
	return func;
}

//Type

static TYPE _TYPE_INT, _TYPE_FLOAT;
TYPE *const TYPE_INT = &_TYPE_INT;
TYPE *const TYPE_FLOAT = &_TYPE_FLOAT;

void TypeInit(){
	_TYPE_INT.kind = BASIC;
	_TYPE_INT.basic = 0;
	_TYPE_FLOAT.kind = BASIC;
	_TYPE_FLOAT.basic = 1;
}

bool TypeEqual(TYPE *a, TYPE *b){
	assert(a != NULL);
	assert(b != NULL);
	if(a == b) return true;
	if(a->kind != b->kind) return false;
	FieldList *p, *q, *as, *bs;
	switch(a->kind){
		case BASIC:
			return (a->basic == b->basic);
		case ARRAY:
			return (TypeEqual(a->array.elem, b->array.elem) && (a->array.size == b->array.size));
		case STRUCTURE:
			as = &a->structure;
			bs = &b->structure;
			p = as->next;
			q = bs->next;
			while((p != &a->structure) && (q != &b->structure)){
				if(!TypeEqual(p->type, q->type)) return false;
				if(strcmp(p->name, q->name) != 0) return false;
				p = p->next;
				q = q->next;
			}
			return ((p == &a->structure) && (q == &b->structure));
	}
	return false;
}

bool ArgsEqual(FieldList *a, FieldList *b){
	assert(a != NULL);
	assert(b != NULL);
	FieldList *p = a->next;
	FieldList *q = b->next;
	while((p != a) && (q != b)){
		if(!TypeEqual(p->type, q->type)){
			return false;
		}
		p = p->next;
		q = q->next;
	}
	return ((p == a) && (q == b));
}

bool FuncEqual(FUNC *a, FUNC *b){
	assert(a != NULL);
	assert(b != NULL);
	return (TypeEqual(a->retType, b->retType) && ArgsEqual(&a->args, &b->args));
}

void TypeRelease(TYPE *a){
	assert(a != NULL);
	if(a->kind == ARRAY){
		TYPE *baseType = a->array.elem;
		if(baseType->kind == ARRAY) TypeRelease(baseType);
		free(a);
	}
	else if(a->kind == STRUCTURE){
		FieldList *o = &a->structure;
		FieldList *p;
		for(p = o->next;p != (&a->structure);){
			FieldList *field = p;
			p = p->next;
			TypeRelease(field->type);
			free(field->name);
			free(field);
		}
		free(a);
	}
}

void ArgsRelease(FieldList *a){
	assert(a != NULL);
	while(a != a->next){
		FieldList *arg = a->next;
		FieldList *prev = arg->prev;
		FieldList *next = arg->next;
		if(prev != NULL) prev->next = next;
		if(next != NULL) next->prev = prev;
		free(arg->name);
		free(arg);
	}
}

void FuncRelease(FUNC *a){
	assert(a != NULL);
	ArgsRelease(&a->args);
	free(a);
}

//table

void SymbolTableInit(){
	top = 0;
	int i;
	for(i = 0;i < SIZE;i++){
		listInit(SymbolTable + i);
		listInit(stack + i);
	}
	FUNC *readfunc = NewFunc(TYPE_INT);
	Symbol *read = NewFuncSymbol("read", readfunc);
	SymbolInsert(read);
	FUNC *writefunc = NewFunc(TYPE_INT);
	FieldList *param = (FieldList *) malloc(sizeof(FieldList));
	param->type = TYPE_INT;
	param->prev = writefunc->args.prev;
	param->next = &writefunc->args;
	writefunc->args.prev->next = param;
	writefunc->args.prev = param;
	Symbol *write = NewFuncSymbol("write", writefunc);
	SymbolInsert(write);
}

static unsigned hashPJW(const char *name) {
	assert(name != NULL);
	unsigned val = 0;
	for (; *name; name++) {
		val = (val << 2) + *name;
		unsigned i = val & ~MASK;
		if (i) val = (val ^ (i >> 12)) & MASK;
	}
	return val;
}

bool SymbolInsert(Symbol *a){
	assert(a != NULL);
	assert(a->name != NULL);
	if(SymbolAtStackTop(a->name)) return false;
	SymbolNode *p = (SymbolNode *) malloc(sizeof(SymbolNode));
	a->depth = top;
	p->symbol = a;
	listInit(&p->chain);
	listInit(&p->stack);
	unsigned hash = hashPJW(a->name);
	listAddAfter(SymbolTable + hash, &p->chain);
	listAddBefore(stack + top, &p->stack);
	return true;
}

bool SymbolAtStackTop(const char *name){
	Symbol *s = SymbolFind(name);
	return (s != NULL) && (s->depth == top);
}

Symbol *SymbolFind(const char *name){
	assert(name != NULL);
	unsigned hash = hashPJW(name);
	list *p;
	for(p = (SymbolTable + hash)->next;p != SymbolTable + hash;p = p->next){
		Symbol *symbol = ((SymbolNode*)((char*)(p) - (long)(&((SymbolNode*)0)->chain)))->symbol;
		if(strcmp(symbol->name, name) == 0) return symbol;
	}
	return NULL;
}

void SymbolRelease(Symbol *symbol){
	assert(symbol != NULL);
	SymbolKind kind = symbol->kind;
	TYPE *type = symbol->type;
	if(((kind == STRUCT) && (type->kind == ARRAY)) || (kind == STRUCT)){
		TypeRelease(type);
	}
	free(symbol->name);
	free(symbol);
}

void SymbolStackPush(){
	top++;
}

void SymbolStackPop(){
	assert(top >= 0);
	list *head = stack + top;
	while(head != head->next){
		SymbolNode *p = (SymbolNode*)((char*)(head->next) - (long)(&((SymbolNode*)0)->stack));
		list *prev = (&p->chain)->prev;
		list *next = (&p->chain)->next;
		if(prev != NULL) prev->next = next;
		if(next != NULL) next->prev = prev;
		prev = (&p->stack)->prev;
		next = (&p->stack)->next;
		if(prev != NULL) prev->next = next;
		if(next != NULL) next->prev = prev;
		SymbolRelease(p->symbol);
		free(p);
	}
	top--;
}

FieldList *FieldFind(FieldList *field, const char *name){
	assert(field != NULL);
	assert(name != NULL);
	FieldList *p;
	for(p = field->next;p != field;p = p->next){
		if(strcmp(p->name, name) == 0) return p;
	}
	return NULL;
}

static void typeArrayToStr(TYPE *type, char *s) {
	assert(type != NULL);
	assert(s != NULL);
	if (type->kind == ARRAY) {
		sprintf(s, "[%d]", type->array.size);
		s += strlen(s);
		typeArrayToStr(type->array.elem, s);
	}
}

static TYPE *baseType(TYPE *type) {
	assert(type != NULL);
	if (type->kind != ARRAY) return type;
	return baseType(type->array.elem);
}

void typeToStr(TYPE *type, char *s) {
	assert(type != NULL);
	assert(s != NULL);
	if (TypeEqual(type, TYPE_INT)) {
		strcpy(s, "int");
	} else if (TypeEqual(type, TYPE_FLOAT)) {
		strcpy(s, "float");
	} else if ((*type).kind == ARRAY) {
		typeToStr(baseType(type), s);
		s += strlen(s);
		typeArrayToStr(type, s);
	} else if ((*type).kind == STRUCTURE) {
		strcpy(s, "struct");
	}
}

void ArgsToStr(FieldList *args, char *s) {
	assert(args != NULL);
	assert(s != NULL);
	FieldList *p;
	for(p = (*args).next;p != args;p = (*p).next){
		if (p != (*args).next) {
			strcpy(s, ", ");
			s += 2;
		}
		typeToStr((*p).type, s);
		s += strlen(s);
	}
	*s = 0;
}

Operand *SymbolGetOperand(Symbol *symbol) {
    if (symbol->id < 0) symbol->id = newVarOperandId();
    return VarOperand(symbol->id);
}

int TypeSize(TYPE *type) {
    assert(type != NULL);
    int size = 0;
    FieldList *p;
    switch (type->kind) {
    case BASIC:
        return 4;
    case ARRAY:
        return TypeSize(type->array.elem) *type->array.size;
    case STRUCTURE:
        for(p = type->structure.next; p != &type->structure; p = p->next){
            size += TypeSize(p->type);
        }
        return size;
    }
    return 0;
}

int FieldOffest(FieldList *structure, const char *fieldname) {
    assert(structure != NULL);
    assert(fieldname != NULL);
    int offest = 0;
    FieldList *p;
    for(p = structure->next; p != structure; p = p->next){
        FieldList *field = p;
        if (strcmp(field->name, fieldname) == 0) return offest;
        offest += TypeSize(field->type);
    }
    return -1;
}
