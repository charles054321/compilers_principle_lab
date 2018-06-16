#include"inter_code.h"
#include"common.h"
#include<stdlib.h>
#include<stdio.h>

static InterCode IrsPool[10000];

static const char *INTER_CODE[] = {
    "LABEL %s :",
    "FUNCTION %s :",
    "%s := %s",
    "%s := %s + %s",
    "%s := %s - %s",
    "%s := %s * %s",
    "%s := %s / %s",
    "%s := &%s",
    "%s := *%s",
    "*%s := %s",
    "GOTO %s",
    "",
    "RETURN %s",
    "",
    "ARG %s",
    "%s := CALL %s",
    "PARAM %s",
    "READ %s",
    "WRITE %s"
};

InterCode *newInterCode(InterCodeKind kind, Operand *res, Operand *op1, Operand *op2){
	static int cnt = 0;
	InterCode *p = &IrsPool[cnt++];
	p->kind = kindl
	p->res = res;
	p->op1 = op1;
	p->op2 = op2;
	return p;
}

InterCode *newInterCode_1op(InterCodeKind kind, Operand *res, Operand *op1){
	newInterCode(kind, res, op1, NULL);
}

InterCode *newInterCode_0op(InterCodeKind kind, Operand *res){
	newInterCode(kind, res, NULL, NULL);
}

InterCode *newInterCode_chain(){
	InterCode *irs = (InterCode *) malloc(sizeof(InterCode));
	irs->prev = irs->next = irs;
	return irs;
}

InterCode *InterCodeInsert(InterCode *head, InterCode *p){
	assert(head != NULL);
	assert(p != NULL);
	p->prev = head->prev;
	p->next = head;
	head->prev->next = p;
	head->prev = p;
	return head;
}

InterCode *InterCodeBind(InterCode *first, InterCode *second){
	assert(first != NULL);
	assert(second != NULL);
	assert(first != second);
	first->prev->next = second->next;
	second->next->prev = first->prev;
	first->prev = second->prev;
	second->prev->next = first;
	free(second);
	return first;
}

void InterCodeToStr(InterCode *p, char *s) {
	assert(p != NULL);
	assert(s != NULL);
	if (p->kind == GOTO_WITH_COND) {
	    sprintf(s, "IF %s %s %s GOTO %s", operandToStr(p->op1), p->relop,
	            operandToStr(p->op2), operandToStr(p->res));
	}
	else if (p->kind == DEC) {
	    sprintf(s, "DEC %s %d", operandToStr(p->res), p->size);
	}
	else {
	    sprintf(s, INTER_CODE[p->kind], operandToStr(p->res),
	            operandToStr(p->op1), operandToStr(p->op2));
	}
}

void InterCodePrint(InterCode *head){
	assert(head != NULL);
	static char buf[120];
	InterCode *i;
	for(i = head->next; i != head; i = i->next){
		InterCodeToStr(i, buf);
		printf("%s\n", buf);
	}
}
	














