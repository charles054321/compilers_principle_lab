#ifndef COMPILER_COMMON_H
#define COMPILER_COMMON_H

#include<stdio.h>

char *toArray(const char*);

extern int errorstatus;

#define name_equal(node, token) \
	((node != NULL) && (strcmp((node.name), str(token)) == 0))

#endif
