#include<stdlib.h>
#include<assert.h>
#include<stdio.h>
#include"symbol.h"
#include"syntax_tree.h"
#include"common.h"
#include"translate.h"
#include"inter_code.h"

#define name_equal(node, token) \
	((node != NULL) && (strcmp((node)->name, #token) == 0))

#define semantic_error(ErrorType, Line, ...)\
do {\
	errorstatus = 2;\
	printf("Error type %d at line %d :", (ErrorType), (Line));\
	printf(SEMANTIC_ERROR[(ErrorType) - 1], __VA_ARGS__);\
	puts(".");\
} while(0)

const char *SEMANTIC_ERROR[] = {
	"Undefined variable : \"%s\"",
	"Undefined function : \"%s\"",
	"Redefined variable : \"%s\"",
	"Redefined function : \"%s\"",
	"Type mismatched for assignment",
	"The left-hand side of an assignment must be a variable",
	"Type mismatched for operands",
	"Type mismatched for return",
	"Function \"%s(%s)\" is not applicable for arguments \"(%s)\"",
	"\"%s\" is not an array",
	"\"%s\" is not a function",
	"\"%s\" is not an integer",
	"Illegal use of \".\"",
	"Non-existent field \"%s\"",
	"Redefined field \"%s\"",
	"Duplicated name \"%s\"",
	"Undefined structure \"%s\"",
	"Undefined function \"%s\"",
	"Inconsistent declaration of function \"%s\""
};

typedef FieldList Dec;
static TYPE *retType;

typedef struct Val{
	TYPE *type;
	bool *isVar;
}Val;

static void AnalyseExtDefList(Treenode *);
static void AnalyseExtDef(Treenode *);
static void AnalyseExtDecList(Treenode *, TYPE *);
static TYPE *AnalyseSpecifier(Treenode *);
static void AnalyseDefList(Treenode *, FieldList *);
static void AnalyseDef(Treenode *, FieldList *);
static void AnalyseDecList(Treenode *, TYPE *, FieldList *);
static void AnalyseDec(Treenode *, TYPE *, FieldList *);
static Dec *AnalyseVarDec(Treenode *, TYPE *);
static Symbol *AnalyseFunDec(Treenode *, TYPE *, bool);
static void AnalyseVarList(Treenode *, FieldList *);
static FieldList *AnalyseParamDec(Treenode *);
static void AnalyseCompSt(Treenode *, FUNC *);
static void AnalyseStmtList(Treenode *);
static void AnalyseStmt(Treenode *);
static Val AnalyseExp(Treenode *);
static void AnalyseArgs(Treenode *, FieldList *);

static Val RequireType(Treenode *, TYPE *, int );

typedef struct FuncSymbol{
	Symbol *symbol;
	int line;
	struct FuncSymbol *next, *prev;
} FuncSymbol;
FuncSymbol FuncSymbol_head;

void FuncSymbolDelete(FuncSymbol *p) {
	assert(p != NULL);
	FuncSymbol *prev = p->prev;
	FuncSymbol *next = p->next;
	if (prev != NULL) prev->next = next;
	if (next != NULL) next->prev = prev;
}

void AnalyseProgram(Treenode *p){
	assert(name_equal(p, Program));
	//printf("a\n");
	FuncSymbol_head.next = FuncSymbol_head.prev = &FuncSymbol_head;
	Treenode *ExtDefList = TreeFirstChild(p);
	AnalyseExtDefList(ExtDefList);
	while(&FuncSymbol_head != FuncSymbol_head.next){
		FuncSymbol *q = FuncSymbol_head.next;
		assert(q->symbol->kind == FUNCTION);
		if(!q->symbol->func->defined)
			semantic_error(18, q->line, q->symbol->name);
		FuncSymbolDelete(q);
		free(q);
	}
}

static void AnalyseExtDefList(Treenode *p){
	assert(name_equal(p, ExtDefList));
	//printf("b\n");
	AnalyseExtDef(TreeFirstChild(p));
	Treenode *next = TreeLastChild(p);
	if(name_equal(next, ExtDefList))
		AnalyseExtDefList(next);
}

static void AnalyseExtDef(Treenode *p){
	assert(name_equal(p, ExtDef));
	//printf("c\n");
	TYPE *type = AnalyseSpecifier(TreeFirstChild(p));
	Treenode *second = TreeKthChild(p, 2);
	Treenode *last = TreeLastChild(p);
	if(name_equal(second, ExtDecList)){
		AnalyseExtDecList(second, type);
	}
	else if(name_equal(second, FunDec)){
		bool isDef = name_equal(last, CompSt);
		Symbol *symbol = AnalyseFunDec(second, type, isDef);
		if(symbol == NULL) return;
		FUNC *func = symbol->func;
		retType = func->retType;
		if(isDef){
			AnalyseCompSt(last, func);
			func->defined = true;
			InterCode *irs = InterCodeStackGet();
			defFunc(symbol->name, irs);
		}
	}
}

static void AnalyseExtDecList(Treenode *p, TYPE *type){
	assert(name_equal(p, ExtDecList));
	//printf("d\n");
	Treenode *first = TreeFirstChild(p);
	Treenode *last = TreeLastChild(p);
	Dec *VarDec = AnalyseVarDec(first, type);
	Symbol *symbol = NewVarSymbol(VarDec->name, type);
	if(!SymbolInsert(symbol))
		semantic_error(3, first->lineno, symbol->name);
	if(name_equal(last, ExtDecList))
		AnalyseExtDecList(last, type);
}

static TYPE *AnalyseSpecifier(Treenode *p){
	assert(name_equal(p, Specifier));
	//printf("e\n");
	Treenode *first = TreeFirstChild(p);
	if(name_equal(first, TYPE)){
		if(strcmp(first->text, "int") == 0) return TYPE_INT;
		else return TYPE_FLOAT;
	}
	else {
		assert(name_equal(first, StructSpecifier));
		assert(name_equal(TreeFirstChild(first), STRUCT));
		Treenode *tag = TreeKthChild(first, 2);
		if(name_equal(tag, Tag)){
			Treenode *id = TreeFirstChild(tag);
			assert(name_equal(id, ID));
			Symbol *symbol = SymbolFind(id->text);
			if((symbol == NULL) || (symbol->kind != STRUCT)){
				semantic_error(17, id->lineno, id->text);
				return TYPE_INT;
			}
			return symbol->type;
		}
		else{
			TYPE *type = (TYPE *) malloc(sizeof(TYPE));
			type->kind = STRUCTURE;
			type->structure.next = type->structure.prev = &type->structure;
			Treenode *defList = TreeLastKthChild(first, 2);
			if(name_equal(defList, DefList)){
				AnalyseDefList(defList, &type->structure);
			}
			if(name_equal(tag, OptTag)){
				Treenode *id = TreeFirstChild(tag);
				assert(name_equal(id, ID));
				Symbol *symbol = NewStructSymbol(id->text, type);
				if(!SymbolInsert(symbol))
					semantic_error(16, id->lineno, id->text);
			}
			return type;
		}
	}
	return NULL;
}

static void AnalyseDefList(Treenode *p, FieldList *list){
	assert(name_equal(p, DefList));
	//printf("f\n");
	AnalyseDef(TreeFirstChild(p), list);
	Treenode *last = TreeLastChild(p);
	if(name_equal(last, DefList))
		AnalyseDefList(last, list);
}

static void AnalyseDef(Treenode *p, FieldList *list){
	assert(name_equal(p, Def));
	//printf("g\n");
	Treenode *Specifier = TreeFirstChild(p);
	Treenode *DecList = TreeKthChild(p, 2);
	TYPE *type = AnalyseSpecifier(Specifier);
	AnalyseDecList(DecList, type, list);
}

static void AnalyseDecList(Treenode *p, TYPE *type, FieldList *list){
	assert(name_equal(p, DecList));
	//printf("h\n");
	Treenode *Dec = TreeFirstChild(p);
	Treenode *last = TreeLastChild(p);
	AnalyseDec(Dec, type, list);
	if(name_equal(last, DecList))
		AnalyseDecList(last, type, list);
}

static void AnalyseDec(Treenode *p, TYPE *type, FieldList *list){
	assert(name_equal(p, Dec));
	//printf("i\n");
	Treenode *first = TreeFirstChild(p);
	Treenode *last = TreeLastChild(p);
	Dec *VarDec = AnalyseVarDec(first, type);
	if(list != NULL){
		if(FieldFind(list, VarDec->name) != NULL)
			semantic_error(15, p->lineno, VarDec->name);
		else{
			VarDec->next = list;
			VarDec->prev = list->prev;
			if(list->prev != NULL) list->prev->next = VarDec;
			if(list != NULL) list->prev = VarDec;
		}
		if(name_equal(last, Exp))
			semantic_error(15, p->lineno, VarDec->name);
	}
	else{
		Symbol *symbol = NewVarSymbol(VarDec->name, VarDec->type);
		if(!SymbolInsert(symbol))
			semantic_error(3, p->lineno, symbol->name);
		else if(name_equal(last, Exp)){
			Val val = AnalyseExp(last);
			if((val.type == NULL) && (!TypeEqual(val.type, symbol->type)))
				semantic_error(5, TreeKthChild(p, 2)->lineno, "");
		}
		free(VarDec->name);
		free(VarDec);
	}
}

static Dec *AnalyseVarDec(Treenode *p, TYPE *type){
	assert(name_equal(p, VarDec));
	//printf("j\n");
	Treenode *first = TreeFirstChild(p);
	if(name_equal(first, ID)){
		Dec *dec = (Dec *) malloc(sizeof(Dec));
		dec->name = toArray(first->text);
		dec->type = type;
		return dec;
	}
	else{
		Treenode *size = TreeKthChild(p, 3);
		assert(name_equal(first, VarDec));
		assert(name_equal(size, INT));
		TYPE *newType = (TYPE *) malloc(sizeof(TYPE));
		newType->kind = ARRAY;
		newType->array.elem = type;
		newType->array.size = size->intval;
		return AnalyseVarDec(first, newType);
	}
}

static Symbol *AnalyseFunDec(Treenode *p, TYPE *type, bool isDef){
	assert(name_equal(p, FunDec));
	assert(type != NULL);
	//printf("j\n");
	FUNC *func = NewFunc(type);
	Treenode *id = TreeFirstChild(p);
	assert(name_equal(id, ID));
	Symbol *symbol = SymbolFind(id->text);
	if((symbol != NULL) && ((symbol->kind != FUNCTION) || (isDef && (symbol->func->defined))))
		semantic_error(4, id->lineno, symbol->name);
	else{
		Treenode *varlist = TreeKthChild(p, 3);
		if(symbol == NULL){
			symbol = NewFuncSymbol(id->text, func);
			if(SymbolInsert(symbol)){
				FuncSymbol *funcsymbol = (FuncSymbol *) malloc(sizeof(FuncSymbol));
				funcsymbol->symbol = symbol;
				funcsymbol->line = p->lineno;
				funcsymbol->prev = FuncSymbol_head.prev;
				funcsymbol->next = &FuncSymbol_head;
				FuncSymbol *a = FuncSymbol_head.prev;
				if(FuncSymbol_head.prev != NULL) a->next = funcsymbol;
				FuncSymbol_head.prev = funcsymbol;
			}
		}
		if(name_equal(varlist, VarList))
			AnalyseVarList(varlist, &func->args);
		if(FuncEqual(symbol->func, func)){
			if(symbol->func != func){
				func->defined = symbol->func->defined;
				FuncRelease(symbol->func);
				symbol->func = func;
			}
			return symbol;
		}
		else{
			semantic_error(19, p->lineno, symbol->name);
		}
	}
	FuncRelease(func);
}

static void AnalyseVarList(Treenode *p, FieldList *field){
	assert(name_equal(p, VarList));
	//printf("k\n");
	FieldList *arg = AnalyseParamDec(TreeFirstChild(p));
	assert(arg != NULL);
	arg->prev = field->prev;
	arg->next = field;
	if(field->prev != NULL) field->prev->next = arg;
	if(field != NULL) field->prev = arg;
	Treenode *varlist = TreeLastChild(p);
	if(name_equal(varlist, VarList)){
		AnalyseVarList(varlist, field);
	}
}

static FieldList *AnalyseParamDec(Treenode *p){
	assert(name_equal(p, ParamDec));
	//printf("l\n");
	TYPE *type = AnalyseSpecifier(TreeFirstChild(p));
	return AnalyseVarDec(TreeLastChild(p), type);
}

static void AnalyseCompSt(Treenode *p, FUNC *func){
	assert(name_equal(p, CompSt));
	//printf("m\n");
	Treenode *deflist = TreeKthChild(p, 2);
	Treenode *stmtlist = TreeLastKthChild(p, 2);
	SymbolStackPush();
	InterCodeStackPush();
	if(func != NULL){
		FieldList *q;
		for(q = (&(func->args))->next;q != &(func->args);q = q->next){
			Symbol *symbol = NewVarSymbol(q->name, q->type);
			if(!SymbolInsert(symbol)){
				semantic_error(3, p->lineno, symbol->name);
			}
		}
	}
	if(name_equal(deflist, DefList)){
		AnalyseDefList(deflist, NULL);
	}
	if(name_equal(stmtlist, StmtList)){
		AnalyseStmtList(stmtlist);
	}
	InterCode *irs = TranslateCompSt(p, func);
	InterCodeStackPop();
	InterCodeStackInsert(irs);
	SymbolStackPop();
}

static void AnalyseStmtList(Treenode *p){
	assert(name_equal(p, StmtList));
	//printf("n\n");
	Treenode *last = TreeLastChild(p);
	AnalyseStmt(TreeFirstChild(p));
	if(name_equal(last, StmtList)){
		AnalyseStmtList(last);
	}
}

static void AnalyseStmt(Treenode *p){
	assert(name_equal(p, Stmt));
	//printf("o\n");
	Treenode *first = TreeFirstChild(p);
	if(name_equal(first, Exp)){
		AnalyseExp(first);
	}
	else if(name_equal(first, ComSt)){
		AnalyseCompSt(first, NULL);
	}
	else if(name_equal(first, RETURN)){
		TYPE *type = AnalyseExp(TreeKthChild(p, 2)).type;
		if(!TypeEqual(type, retType))
			semantic_error(8, p->lineno, "");
	}
	else{
		RequireType(TreeKthChild(p, 3), TYPE_INT, 7);
		AnalyseStmt(TreeKthChild(p, 5));
		if(TreeLastChild(p) != TreeKthChild(p, 5)) AnalyseStmt(TreeLastChild(p));
	}
}

static Val MakeVar(TYPE *type){
	Val val;
	val.type = type;
	val.isVar = true;
	return val;
}

static Val MakeVal(TYPE *type){
	Val val;
	val.type = type;
	val.isVar = false;
	return val;
}

static Val RequireBasic(Treenode *p, int ErrorType){
	Val val = AnalyseExp(p);
	assert(val.type != NULL);
	if(val.type->kind != BASIC){
		semantic_error(ErrorType, p->lineno, p->text);
	}
	return val;
}

static Val RequireType(Treenode *p, TYPE *type, int ErrorType){
	Val val = AnalyseExp(p);
	if(!TypeEqual(val.type, type)){
		semantic_error(ErrorType, p->lineno, p->text);
	}
	return val;
}

static Val AnalyseExp(Treenode *p){
	assert(name_equal(p, Exp));
	//printf("p\n");
	Treenode *first = TreeFirstChild(p);
	Treenode *second = TreeKthChild(p, 2);
	Treenode *last = TreeLastChild(p);
	if(name_equal(first, Exp)){
		if(name_equal(second, ASSIGNOP)){
			Val left = AnalyseExp(first);
			if(!left.isVar) semantic_error(6, first->lineno, "");
			else{
				RequireType(last, left.type, 5);
				return left;
			}
		}
		else if(name_equal(second, AND) || name_equal(second, OR)){
			RequireType(first, TYPE_INT, 7);
			RequireType(last, TYPE_INT, 7);
			return MakeVal(TYPE_INT);
		}
		else if(name_equal(second, RELOP) || name_equal(second, PLUS)
			|| name_equal(second, MINUS) || name_equal(second, STAR)
			|| name_equal(second, DIV)){
			Val left = RequireBasic(first, 7);
			RequireType(last, left.type, 7);
			return MakeVar(left.type);
		}
		else if(name_equal(second, LB)){
			Val base = AnalyseExp(first);
			RequireType(TreeKthChild(p, 3), TYPE_INT, 12);
			if(base.type->kind != ARRAY)
				semantic_error(10, first->lineno, first->text);
			else{
				base.type = base.type->array.elem;
				return base;
			}
		}
		else{
			Val base = AnalyseExp(first);
			char *fieldname = last->text;
			if(base.type->kind != STRUCTURE)
				semantic_error(13, second->lineno, "");
			else{
				FieldList *field = FieldFind(&(base.type->structure), fieldname);
				if(field == NULL)
					semantic_error(14, last->lineno, fieldname);
				else{
					base.type = field->type;
					return base;
				}
			}
		}
	}
	else if(name_equal(first, LP)){
		return AnalyseExp(second);
	}
	else if(name_equal(first, MINUS)){
		Val val = RequireBasic(second, 7);
		return MakeVal(val.type);
	}
	else if(name_equal(first, NOT)){
		Val val = RequireType(second, TYPE_INT, 7);
		return MakeVal(TYPE_INT);
	}
	else if(name_equal(first, INT)){
		return MakeVal(TYPE_INT);
	}
	else if(name_equal(first, FLOAT)){
		return MakeVal(TYPE_FLOAT);
	}
	else{
		if(name_equal(last, ID)){
			Symbol *symbol = SymbolFind(first->text);
			if(symbol == NULL)
				semantic_error(1, first->lineno, first->text);
			else{
				return MakeVar(symbol->type);
			}
		}
		else{
			Symbol *symbol = SymbolFind(first->text);
			if(symbol == NULL)
				semantic_error(2, first->lineno, first->text);
			else if(symbol->kind != FUNCTION)
				semantic_error(11, first->lineno, first->text);
			else{
				FieldList args;
				args.next = args.prev = &args;
				if(name_equal(TreeKthChild(p, 3), Args)) AnalyseArgs(TreeKthChild(p, 3), &args);
				if(!ArgsEqual(&args, &(symbol->func->args))){
					char funcstr[32], argsstr[32];
					ArgsToStr(&(symbol->func->args), funcstr);
					ArgsToStr(&args, argsstr);
					semantic_error(9, first->lineno, symbol->name, funcstr, argsstr);
				}
				ArgsRelease(&args);
				return MakeVal(symbol->func->retType);
			}
		}
	}
	return MakeVal(TYPE_INT);
}

static void AnalyseArgs(Treenode *p, FieldList *args){
	assert(name_equal(p, Args));
	//printf("q\n");
	Treenode *first = TreeFirstChild(p);
	Treenode *last = TreeLastChild(p);
	FieldList *arg = (FieldList *) malloc(sizeof(FieldList));
	arg->type = AnalyseExp(first).type;
	arg->name = NULL;
	arg->prev = args->prev;
	arg->next = args;
	if(args->prev != NULL) args->prev->next = arg;
	if(args != NULL) args->prev = arg;
	if(name_equal(last, Args))
		AnalyseArgs(last, args);
}
















