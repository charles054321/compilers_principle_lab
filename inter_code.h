#ifndef COMPILERS_INTER_NODE_H
#define COMPILERS_INTER_NODE_H

typedef enum{
	VARIABLE, TEMP, CONSTANT, LABEL, FUNCT
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

void OperandInit();
Operand *newOperand(OperandKind );
int newVarOperandId();
int newTempOperandId();
int newLabelOperandId();
Operand *ConstOperand(int );
Operand *VarOperand(int );
Operand *TempOperand(int );
Operand *LabelOperand(int );
Operand *newVarOperand();
Operand *newTempOperand();
Operand *newLabelOperand();
Operand *newFuncOperand(char *);

char *OperandToStr(Operand *);

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
	struct InterCode *child, *prev, *next;
}InterCode;

InterCode *newInterCode(InterCodeKind , Operand *, Operand *, Operand *);
InterCode *newInterCode_1op(InterCodeKind , Operand *, Operand *);
InterCode *newInterCode_0op(InterCodeKind , Operand *);

InterCode *newInterCode_chain();
InterCode *InterCodeInsert(InterCode *, InterCode *);
InterCode *InterCodeBind(InterCode *, InterCode *);
void InterCodeToStr(InterCode *, char *);
void InterCodePrint(InterCode *);

#endif
















