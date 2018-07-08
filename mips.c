#include "common.h"
#include "inter_code.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static const char *MIPS[] = {
    "%s:\n",
    "\n%s:\n",
    " %s, %s\n",
    " %s, %s, %s\n",
    " %s, %s, ",
    "  mul %s, %s, %s\n",
    "  div %s, %s\n  mflo %s\n",
    "",
    "  lw %s, 0(%s)\n",
    "  sw %s, 0(%s)\n",
    "  j %s\n",
    " %s, %s, %s\n",
    "  move $v0, %s\n  jr $ra\n",
    "",
    "",
    "  jal %s\n  move %s, $v0\n",
    "",
    "\
  addi $sp, $sp, -4\n\
  sw $ra, 0($sp)\n\
  jal read\n\
  lw $ra, 0($sp)\n\
  addi $sp, $sp, 4\n\
  move %s, $v0\n",
    "\
  move $a0, %s\n\
  addi $sp, $sp, -4\n\
  sw $ra, 0($sp)\n\
  jal write\n\
  lw $ra, 0($sp)\n\
  addi $sp, $sp, 4\n"
};

static void MipsRead();
static void MipsWrite();
static int paramnum = 0, argnum = 0;
static char *GetReg(Operand *);
static Operand *regs[32] = { NULL };
static Operand *args[8] = { NULL };
static char stringforconst[10];

void MipsInit() {
	printf("\
.data\n\
_prompt: .asciiz \"Enter an integer:\"\n\
_ret: .asciiz \"\\n\"\n\
.globl main\n\
.text\n");
	MipsRead();
	MipsWrite();
}

static void MipsRead() {
	printf("\
read:\n\
  li $v0, 4\n\
  la $a0, _prompt\n\
  syscall\n\
  li $v0, 5\n\
  syscall\n\
  jr $ra\n\n");
}

static void MipsWrite() {
	printf("\
write:\n\
  li $v0, 1\n\
  syscall\n\
  li $v0, 4\n\
  la $a0, _ret\n\
  syscall\n\
  move $v0, $0\n\
  jr $ra\n");
}

static void MipsTranslate(InterCode *ic){
	assert(ic != NULL);
	Operand *res = ic->result, *op1 = ic->op1, *op2 = ic->op2;
	int kind = ic->kind;
	if(kind == ASSIGN){
		if(op1->kind == CONSTANT) printf("  li");
		else printf("  move");
		printf(MIPS[kind], GetReg(res), GetReg(op1));
	}
	else if(kind == ADD){
		if(op2->kind == CONSTANT) printf("  addi");
		else printf("  add");
		printf(MIPS[kind], GetReg(res), GetReg(op1), GetReg(op2));
	}
	else if(kind == SUB){
		if(op2->kind == CONSTANT) printf("  addi");
		else printf("  sub");
		printf(MIPS[kind], GetReg(res), GetReg(op1));
		if(op2->kind == CONSTANT) printf("-");
		printf("%s\n", GetReg(op2));
	}
	else if(kind == DIV){
		printf(MIPS[kind], GetReg(op1), GetReg(op2), GetReg(res));
	}
	else if(kind == SET_ADDR){
		printf(MIPS[kind], GetReg(op1), GetReg(res));
	}	
	else if(kind == CALL){
		printf("  addi $sp, $sp, -%d\n", (paramnum + 1) << 2);
		int i;
		for(i = 0;i < paramnum;i++){
			printf("  sw $a%d, %d($sp)\n", i, i << 2);
		}
		printf("  sw $ra, %d($sp)\n", paramnum << 2);
		for(i = 0;i < argnum;i++){
			printf("  move $a%d, %s\n", i, GetReg(args[i]));
		}
		printf(MIPS[kind], GetReg(op1), GetReg(res));
		for(i = 0;i < paramnum;i++){
			printf("  lw $a%d, %d($sp)\n", i, i << 2);
		}
		printf("  lw $ra, %d($sp)\n", paramnum << 2);
		printf("  addi $sp, $sp, 8\n");
		argnum = 0;
	}
	else if(kind == GOTO_WITH_COND){
		char *relop = ic->relop;
		printf("  ");
		if(strcmp(relop, "==") == 0) printf("beq");
		else if(strcmp(relop, "!=") == 0) printf("bne");
		else if(strcmp(relop, ">") == 0) printf("bgt");
		else if(strcmp(relop, "<") == 0) printf("blt");
		else if(strcmp(relop, ">=") == 0) printf("bge");
		else if(strcmp(relop, "<=") == 0) printf("ble");
		else assert(0);
		printf(MIPS[kind], GetReg(op1), GetReg(op2), GetReg(res));
	}
	else if(kind == RETURN){
		if(res->kind == CONSTANT){
			printf("  li $t1, %s\n", GetReg(res));
			printf(MIPS[kind], "$t1");
		}
		else printf(MIPS[kind], GetReg(res));
	}
	else if(kind == PARAM){
		regs[4 + paramnum] = res;
		paramnum++;
	}
	else if(kind == ARG){
		args[argnum++] = res;
	}
	else if(kind == DEF_FUNCTION){
		paramnum = 0;
		printf(MIPS[kind], GetReg(res));
	}
	else printf(MIPS[kind], GetReg(res), GetReg(op1), GetReg(op2));
}

void MipsMainLoop(){
	InterCode *ListHead = GetInterCodeHead();
	InterCode *i;
	for(i = ListHead->next; i != ListHead; i = i->next){
		InterCode *m = i;
		MipsTranslate(m);
	}
}

static int regnum = 0;
static char *REG_NAME[] = {
	"$zero", "$at", "$v0", "$v1",
	"$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3",
	"$t4", "$t5", "$t6", "$t7",
	"$s0", "$s1", "$s2", "$s3",
	"$s4", "$s5", "$s6", "$s7",
	"$t8", "$t9", "$k0", "$k1",
	"$gp", "$sp", "$fp", "$ra"
};

bool opEqual(Operand *op1, Operand *op2){
	if(op1 == NULL || op2 == NULL) return false;
	if(op1->kind != op2->kind) return false;
	return op1->id == op2->id;
}

static char *GetReg(Operand *op){
	if(op == NULL) return "";
	if(op->kind == LABEL || op->kind == FUNCT) return OperandToStr(op);
	else if(op->kind == CONSTANT){
		sprintf(stringforconst, "%d", op->value);
		return stringforconst;
	}
	else{
		int i;
		for(i = 0;i < 32;i++)
			if(opEqual(regs[i], op)) return REG_NAME[i];
		regnum++;
		regnum &= 7;
		regs[regnum + 8] = op;
		return REG_NAME[regnum + 8];
	}
	return "";
}
	















