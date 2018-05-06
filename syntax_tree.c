#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include"syntax_tree.h"

Treenode *root = NULL;

Treenode *newnode(){
	Treenode *p = (Treenode*) malloc(sizeof(Treenode));
	(*p).child = NULL;
	(*p).prev = (*p).next = p;
	return p;
}

void treeAddChild(Treenode *now, Treenode *p){
	if(p != NULL && now != NULL){
		if((*now).child == NULL) (*now).child = p;
		else {
			Treenode *q = (*now).child;
			(*p).next = (*q).next;
			(*q).next = p;
			(*(*p).next).prev = p;
			(*p).prev = q;
		}
	}
}

void print(Treenode *head, int level){
	int i;
	for(i = 0; i <= level;i++){
		printf("  ");
	}
	if(head == NULL){
		return;
	}
	printf("%s", (*head).name);
	if((*head).name[1] <= 'Z'){
		if(strcmp((*head).name, "TYPE") == 0 || strcmp((*head).name, "ID") == 0){
			printf(": %s", (*head).text);
		}
		else if(strcmp((*head).name, "INT") == 0){
			printf(": %d", (*head).intval);
		}
		else if(strcmp((*head).name, "FLOAT") == 0){
			printf(": %f", (*head).floatval);
		}
	}
	else {
		printf(" (%d)", (*head).lineno);
	}
	printf("\n");
	Treenode *q = (*head).child;
	Treenode *p;
	if((*head).child != NULL){
		print(q, level + 1);
		for(p = (*q).prev; p != q; p = (*p).prev){
			print(p, level + 1);
		}
	}
}		

















