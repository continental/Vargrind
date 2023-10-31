#ifndef __STACK_H
#define __STACK_H

#include "pub_tool_hashtable.h"

struct _stack_node{
    struct _stack_node *next;
    char* function_name;
    VgHashTable *table;
};

typedef struct _stack_node stack_node;

void stack_push(char* function_name);
void stack_pop(void);
VgHashTable* stack_peek(void);



#endif

