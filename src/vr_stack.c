#include "vr_stack.h"
#include "vr_output.h"
#include "pub_tool_mallocfree.h"

stack_node *top = NULL;

stack_node* create_new_stack_node(char* function_name){
    stack_node *new_node = VG_(malloc)("malloc in vargirnd for stack node", sizeof(stack_node));

    VgHashTable *table = VG_(HT_construct) ( function_name);
    new_node->table = table;
    new_node->function_name = VG_(strdup)("vargrdin add function name to stack node", function_name);
    return new_node;
}

void stack_push(char* function_name){
    stack_node *new = create_new_stack_node(function_name);
    new->next = top;
    top = new;
}

VgHashTable* stack_peek(void){
    return top->table;
} 

int stack_is_empty(void){
    return top == NULL;
}

void stack_pop(void){
    if (!stack_is_empty()){
        stack_node *node_to_pop;
        
        node_to_pop = top;
        top = top->next;
        node_to_pop->next = NULL;

        print_table(node_to_pop->table, node_to_pop->function_name);

        //free
        VG_(free)(node_to_pop->function_name);
        VG_(free)(node_to_pop);
    }

}
