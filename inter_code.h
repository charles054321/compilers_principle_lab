#ifndef COMPILERS_INTER_NODE_H
#define COMPILERS_INTER_NODE_H

typedef enum{
	VARIABLE, TEMP, CONSTANT, LABEL, FUNCTION
}OperandKind;

typedef struct Operand{
	OperandKind kind;
	union{
		int id;
		int value;
		char *name;
	};
	char *text;
}Operand;

extern Operand *const CONST_ZERO;
extern Operand *const CONST_ONE;

typedef enum{
	DEF_LABEL, DEF_FUNCTION, ASSIGN, ADD, SUB, MUL, DIV,
	GET_REF, GET_ADDR, SET_ADDR, GOTO, GOTO_WITH_COND,
	RETURN, DEC, ARG, CALL, PARAM, READ, WRITE
}InterCodeKind;

typedef struct InterCode{
	InterCodeKind kind;
	Operand *result, *op1, *op2;
	union{
		char *relop;
		int size;
	};
	struct InterCode *prev, *next;
}InterCode;

#endif
















