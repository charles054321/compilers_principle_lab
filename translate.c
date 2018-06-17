#include"translate.h"
#include"symbol.h"
#include"common.h"
#include<stdlib.h>
#include"inter_code.h"

static InterCode head;
#define TRANSLATE_SIZE 10000
typedef struct List{
	struct List *prev, *next;
}List;

static List stack[TRANSLATE_SIZE];
static int top;

typedef struct InterCodeList{
	InterCode *head;
	List list;
}InterCodeList;

void InterCodeInit(){
	(&head)->prev = (&head)->next = &head;
	stack->prev = stack->next = stack;
	top = 0;
}

void InterCodeStackPush(){
	assert(++top < TRANSLATE_SIZE);
	(stack + top)->prev = (stack + top)->next = stack + top;
}

void InterCodeStackPop(){
	assert(top-- > 0);
}

void InterCodeStackInsert(InterCode *Listhead){
	InterCodeList *listnode = (InterCodeList *) malloc(sizeof(InterCodeList));
	listnode->list.prev = (stack + top)->prev;
	listnode->list.next = stack + top;
	if((stack + top)->prev != NULL) (stack + top)->prev->next = &listnode->list;
	if(stack + top != NULL) (stack + top)->prev = &listnode->list;
	listnode->head = Listhead;
}

InterCode *InterCodeStackGet(){
	assert(stack + top != (stack + top)->next);
	List *p = stack[top].next;
	InterCodeList *listnode = ((InterCodeList*)((char*)(p) - (int)(&((InterCodeList*)0)->list)));
	InterCode *listhead = listnode->head;
	free(listnode);
	return listhead;
}

InterCode *InterCodeGet(){
	return &head;
}

void defParams(FUNC *func){
	FieldList *i;
	for(i = func->args.next; i != &func->args; i = i->next){
		Symbol *symbol = SymbolFind(i->name);
		symbol->id = newVarOperandId();
	}
}

void defFunc(char *name, InterCode *irs){
	assert(name != NULL);
	Operand *op = newFuncOperand(name);
	InterCode *ir = newInterCode_0op(DEF_FUNCTION, op);
	InterCodeInsert(&head, ir);
	if(irs == irs->next || irs->prev->kind != RETURN){
		ir = newInterCode_0op(RETURN, ConstOperand(-1));
		InterCodeInsert(irs, ir);
	}
	InterCodeBind(&head, irs);
}

static InterCode *TranslateDefList(Treenode*);
static InterCode *TranslateDef(Treenode*);
static InterCode *TranslateDecList(Treenode*);
static InterCode *TranslateDec(Treenode*);
static InterCode *TranslateStmtList(Treenode*);
static InterCode *TranslateStmt(Treenode*);
static InterCode *TranslateCond(Treenode*, Operand*, Operand*);
static InterCode *TranslateArgs(Treenode*, FieldList*, List*);

#define name_equal(node, token) \
	((node != NULL) && (strcmp((node)->name, #token) == 0))

InterCode *TranslateCompSt(Treenode *p, FUNC *func){
	assert(name_equal(p, CompSt));
	InterCode *irs = newInterCode_chain();
	if(func != NULL){
		FieldList *i;
		for(i = func->args.next; i != &func->args; i = i->next){
			Symbol *symbol = SymbolFind(i->name);
			Operand *op = VarOperand(symbol->id);
			InterCode *ir = newInterCode_0op(PARAM, op);
			InterCodeInsert(irs, ir);
		}
	}
	Treenode *second = TreeKthChild(p, 2);
	Treenode *last = TreeLastKthChild(p, 2);
	if(name_equal(second, DefList))
		InterCodeBind(irs, TranslateDefList(second));
	if(name_equal(last, StmtList))
		InterCodeBind(irs, TranslateStmtList(last));
	return irs;
}

typedef struct OperandList{
	Operand *op;
	List list;
}OperandList;

static InterCode *TranslateDefList(Treenode *p) {
	assert(name_equal(p, DefList));
	InterCode *irs = TranslateDef(TreeFirstChild(p));
	if (name_equal(TreeLastChild(p), DefList)) InterCodeBind(irs, TranslateDefList(TreeKthChild(p, 2)));
	return irs;
}

static InterCode *TranslateDef(Treenode *p) {
	assert(name_equal(p, Def));
	return TranslateDecList(TreeKthChild(p, 2));
}

static InterCode *TranslateDecList(Treenode *p) {
	assert(name_equal(p, DecList));
	InterCode *irs = TranslateDec(TreeFirstChild(p));
	if (name_equal(TreeLastChild(p), DecList)) InterCodeBind(irs, TranslateDecList(TreeKthChild(p, 3)));
	return irs;
}

char *VarDecToStr(Treenode *p) {
	assert(name_equal(p, VarDec));
	Treenode *first = TreeFirstChild(p);
	if (name_equal(first, ID)) return first->text;
	return VarDecToStr(first);
}

static InterCode *TranslateDec(Treenode *p) {
	assert(name_equal(p, Dec));
	Treenode *first = TreeFirstChild(p);
	Treenode *last = TreeKthChild(p, 3);
	Symbol *symbol = SymbolFind(VarDecToStr(first));
	InterCode *irs = newInterCode_chain();
	Operand *var = SymbolGetOperand(symbol);
	if (symbol->type->kind != BASIC) {
	    InterCode *ir = newInterCode_0op(DEC, var);
	    ir->size = TypeSize(symbol->type);
	    InterCodeInsert(irs, ir);
	}
	if (name_equal(TreeLastChild(p), Exp)) {
	    Operand *op = newTempOperand();
	    InterCodeBind(irs, TranslateExp(last, op));
	    InterCodeInsert(irs, newInterCode_1op(ASSIGN, var, op));
	}
	return irs;
}

static InterCode *TranslateStmtList(Treenode *p) {
	assert(name_equal(p, StmtList));
	Treenode *first = TreeFirstChild(p);
	Treenode *second = TreeLastChild(p);
	InterCode *irs = TranslateStmt(first);
	if (name_equal(second, StmtList)) InterCodeBind(irs, TranslateStmtList(second));
	return irs;
}

static InterCode *TranslateStmt(Treenode *p) {
	assert(name_equal(p, Stmt));
	Treenode *first = TreeFirstChild(p);
	Treenode *c_3 = TreeKthChild(p, 3);
	Treenode *c_5 = TreeKthChild(p, 5);
	Treenode *c_7 = TreeKthChild(p, 7);
	if (name_equal(first, Exp)) {
		//printf("a\n");
		return TranslateExp(first, NULL);
	}
	else if (name_equal(first, CompSt)) {
		return InterCodeStackGet();
	}
	else if (name_equal(first, RETURN)) {
		Operand *op = newTempOperand();
		InterCode *irs = TranslateExp(TreeKthChild(p, 2), op);
		InterCode *ir = newInterCode_0op(RETURN, op);
		InterCodeInsert(irs, ir);
		return irs;
	}
	else if (name_equal(first, IF)) {
		bool el = TreeLastChild(p) == TreeKthChild(p, 7);
		Treenode *stmt1, *stmt2;
		stmt1 = c_5;
		if (el) stmt2 = c_7;
		else stmt2 = NULL;
		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		Operand *label3 = NULL;
		InterCode *irs = TranslateCond(c_3, label1, label2);
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label1));
		//printf("a\n");
		InterCode *i = TranslateStmt(stmt1);
		InterCodeBind(irs, i);
		if (stmt2) {
			label3 = newLabelOperand();
			InterCodeInsert(irs, newInterCode_0op(GOTO, label3));
		}
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label2));
		if (stmt2) {
			InterCodeBind(irs, TranslateStmt(stmt2));
			InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label3));
		}
		return irs;
	}
	else {
		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		Operand *label3 = newLabelOperand();
		InterCode *irs = newInterCode_chain();
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label1));
		InterCodeBind(irs, TranslateCond(c_3, label2, label3));
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label2));
		InterCodeBind(irs, TranslateStmt(c_5));
		InterCodeInsert(irs, newInterCode_0op(GOTO, label1));
		return InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label3));
	}
}

static InterCode *TranslateCond(Treenode *p, Operand *labeltrue,
                                 Operand *labelfalse) {
	if (!name_equal(p, Exp)) printf("%s %d\n", p->name, p->token);
	assert(name_equal(p, Exp));
	Treenode *first = TreeFirstChild(p);
	Treenode *second = TreeKthChild(p, 2);
	Treenode *third = TreeKthChild(p, 3);
	if (name_equal(first, NOT)) {
		return TranslateCond(second, labelfalse, labeltrue);
	}
	else if (name_equal(second, RELOP)) {
		Operand *op1 = newTempOperand();
		Operand *op2 = newTempOperand();
		InterCode *irs = TranslateExp(first, op1);
 		InterCode *irs2 = TranslateExp(third, op2);
		InterCodeBind(irs, irs2);
		InterCode *ir = newInterCode(GOTO_WITH_COND, labeltrue, op1, op2);
		ir->relop = second->text;
		InterCodeInsert(irs, ir);
		return InterCodeInsert(irs, newInterCode_0op(GOTO, labelfalse));
	}
	else if (name_equal(second, AND)) {
		Operand *label = newLabelOperand();
		InterCode *irs = TranslateCond(first, label, labelfalse);
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label));
		return InterCodeBind(irs, TranslateCond(third, labeltrue, labelfalse));
	}
	else if (name_equal(second, OR)) {
		Operand *label = newLabelOperand();
		InterCode *irs = TranslateCond(first, labeltrue, label);
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label));
		return InterCodeBind(irs, TranslateCond(third, labeltrue, labelfalse));
	}
	else {
		Operand *op = newTempOperand();
		InterCode *irs = TranslateExp(p, op);
		InterCode *ir = newInterCode(GOTO_WITH_COND, labeltrue, op, CONST_ZERO);
		ir->relop = "!=";
 		InterCodeInsert(irs, ir);
		ir = newInterCode_0op(GOTO, labelfalse);
		return InterCodeInsert(irs, ir);
	}
}

InterCode *TranslateExp(Treenode *p, Operand *res) {
	assert(name_equal(p, Exp));
	Treenode *first = TreeFirstChild(p);
	Treenode *second = TreeKthChild(p, 2);
	Treenode *third = TreeKthChild(p, 3);
	static TYPE *type = NULL;
	InterCode *irs = newInterCode_chain();
	if (name_equal(first, ID)) {
		Symbol *symbol = SymbolFind(first->text);
		if (name_equal(TreeLastChild(p), ID)) {
			if (res == NULL) return irs;
			Operand *op = SymbolGetOperand(symbol);
			type = symbol->type;
			if (res->id < 0 && type->kind != BASIC) {
				res->id = newTempOperandId();
				if (symbol->isRef) {
					InterCodeInsert(irs, newInterCode_1op(ASSIGN, res, op));
				}
				else {
					InterCodeInsert(irs, newInterCode_1op(GET_REF, res, op));
				}
			}
			else {
				*res = *op;
			}
		}
		else {
			assert(symbol->kind == FUNCTION);
			List args;
			(&args)->prev = (&args)->next = &args;
			if (name_equal(third, Args)) {
				FieldList *firstarg = symbol->func->args.next;
				InterCodeBind(irs, TranslateArgs(third, firstarg, &args));
			}
			if (strcmp(symbol->name, "read") == 0) {
				if (!res) res = newTempOperand();
				InterCode *ir = newInterCode_0op(READ, res);
				InterCodeInsert(irs, ir);
			}
			else if (strcmp(symbol->name, "write") == 0) {
				Operand *op = ((OperandList*)((char*)(args.next) - (int)(&((OperandList*)0)->list)))->op;
				InterCode *ir = newInterCode_0op(WRITE, op);
				InterCodeInsert(irs, ir);
				if (res) {
					op = ConstOperand(0);
					ir = newInterCode_1op(ASSIGN, res, op);
					InterCodeInsert(irs, ir);
				}
			}
			else {
				List *q;
	        		for(q = (&args)->next; q != (&args); q = q->next){
	        			Operand *op = ((OperandList*)((char*)(q) - (int)(&((OperandList*)0)->list)))->op;
	        			InterCode *ir = newInterCode_0op(ARG, op);
	        			InterCodeInsert(irs,ir);
	        		}
	        		if (!res) res = newTempOperand();
	        		Operand *op = newFuncOperand(first->text);
	        		InterCode *ir = newInterCode_1op(CALL, res, op);
	        		InterCodeInsert(irs,ir);
	        	}
	        	while (args.next != &args) {
				List *q = (&args)->next;
	        		OperandList *operandnode = ((OperandList*)((char*)(q) - (int)(&((OperandList*)0)->list)));
				List *prev = q->prev, *next = q->next;
				if (prev != NULL) prev->next = next;
				if (next != NULL) next->prev = prev;
	       			free(operandnode);
	       		}
	     	type = symbol->func->retType;
	    	}
	}
	else if (name_equal(first, INT)) {
		if (!res) return irs;
		*res = *ConstOperand(first->intval);
		type = TYPE_INT;
	}
	else if (name_equal(second, LB)) {
		Operand *index = newTempOperand();
		InterCodeBind(irs, TranslateExp(third, index));
		Operand *base = TempOperand(-1);
		InterCodeBind(irs, TranslateExp(first, base));
		assert(type->kind == ARRAY);
		type = type->array.elem;
		Operand *offest = newTempOperand();
		Operand *size = ConstOperand(TypeSize(type));
		InterCodeInsert(irs, newInterCode(MUL, offest, index, size));
		if (res->id < 0) {
			res->id = newTempOperandId();
			InterCodeInsert(irs, newInterCode(ADD, res, base, offest));
		}
		else {
			Operand *tmp = newTempOperand();
        		InterCodeInsert(irs, newInterCode(ADD, tmp, base, offest));
		        InterCodeInsert(irs, newInterCode_1op(GET_ADDR, res, tmp));
		}
	}
	else if (name_equal(second, DOT)) {
		char *id = third->text;
		Operand *base = TempOperand(-1);
		InterCodeBind(irs, TranslateExp(first, base));
		assert(type->kind == STRUCTURE);
		FieldList *field = FieldFind(&type->structure, id);
		assert(field != NULL);
		Operand *offest = ConstOperand(FieldOffest(&type->structure, id));
		type = field->type;
		if (res->id < 0) {
			res->id = newTempOperandId();
			InterCodeInsert(irs, newInterCode(ADD, res, base, offest));
		}
		else {
			Operand *tmp = newTempOperand();
			InterCodeInsert(irs, newInterCode(ADD, tmp, base, offest));
			InterCodeInsert(irs, newInterCode_1op(GET_ADDR, res, tmp));
		}
	}
	else if (name_equal(first, LP)) {
		free(irs);
		return TranslateExp(second, res);
	}
	else if (name_equal(first, NOT) || name_equal(second, RELOP) ||
		name_equal(second, AND) || name_equal(second, OR)) {
		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		InterCode *condirs = TranslateCond(p, label1, label2);
		if (!res) res = newTempOperand();
		InterCodeInsert(irs, newInterCode_1op(ASSIGN, res, CONST_ZERO));
		InterCodeBind(irs, condirs);
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label1));
		InterCodeInsert(irs, newInterCode_1op(ASSIGN, res, CONST_ONE));
		InterCodeInsert(irs, newInterCode_0op(DEF_LABEL, label2));
		type = TYPE_INT;
	}
	else if (name_equal(first, MINUS)) {
		Operand *op = newTempOperand();
		InterCode *expirs = TranslateExp(second, op);
		InterCodeBind(irs, expirs);
		if (!res) return irs;
		InterCode *ir = newInterCode(SUB, res, CONST_ZERO, op);
		InterCodeInsert(irs, ir);
	}
	else if (name_equal(second, ASSIGNOP)) {
		Operand *op1 = TempOperand(-1);
		Operand *op2 = newTempOperand();
		InterCode *irs1 = TranslateExp(first, op1);
		InterCode *irs2 = TranslateExp(third, op2);
		InterCodeBind(irs, irs1);
		InterCodeBind(irs, irs2);
		InterCode *ir = (op1->kind == VARIABLE) ?
			newInterCode_1op(ASSIGN, op1, op2) :
			newInterCode_1op(SET_ADDR, op1, op2);
		InterCodeInsert(irs, ir);
		if (!res) return irs;
		if (res->id < 0) {
			*res = *op1;
		}
		else {
			InterCodeInsert(irs, newInterCode_1op(GET_ADDR, res, op1));
		}
	}
	else {
		Operand *op1 = newTempOperand();
		Operand *op2 = newTempOperand();
		InterCode *irs1 = TranslateExp(first, op1);
		InterCode *irs2 = TranslateExp(third, op2);
		InterCodeBind(irs, irs1);
		InterCodeBind(irs, irs2);
		InterCodeKind kind = ADD;
		if (!res) return irs;
		if (name_equal(second, PLUS)) kind = ADD;
		else if (name_equal(second, MINUS)) kind = SUB;
		else if (name_equal(second, STAR)) kind = MUL;
		else if (name_equal(second, DIV)) kind = DIV;
		InterCode *ir = newInterCode(kind, res, op1, op2);
		InterCodeInsert(irs, ir);
	}
	return irs;
}

static InterCode *TranslateArgs(Treenode *p, FieldList *curarg, List *args) {
	assert(name_equal(p, Args));
	Treenode *first = TreeFirstChild(p);
	Treenode *last = TreeKthChild(p, 3);
	InterCode *restirs = NULL;
	if (name_equal(last, Args))
		restirs = TranslateArgs(last, curarg->next, args);
	FieldList *arg = curarg;
	Operand *op = (arg->type->kind == BASIC) ? newTempOperand() : TempOperand(-1);
	InterCode *irs = TranslateExp(first, op);
	if (restirs) irs = InterCodeBind(restirs, irs);
	OperandList *operandnode = (OperandList*) malloc(sizeof(OperandList));
	operandnode->op = op;
	(&operandnode->list)->prev = args->prev;
	(&operandnode->list)->next = args;
	args->prev->next = (&operandnode->list);
	args->prev = (&operandnode->list);
	return irs;
}














