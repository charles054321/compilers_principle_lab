#ifndef COMPILER_SYNTAX_TREE_H
#define COMPILER_SYNTAX_TREE_H

#include<string.h>

typedef struct Treenode {
	int lineno, token;
	union {
		int intval;
		float floatval;
	};
	char *text, *name;
	struct Treenode *child, *prev, *next;
}Treenode;

extern Treenode *root;

Treenode *newnode();
void treeAddChild(Treenode*, Treenode*);
void print(Treenode*, int);

#endif
