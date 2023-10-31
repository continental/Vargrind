#include "vr_hash.h"
#include "vr_get_set_node.h"
#include "pub_tool_mallocfree.h"

//function name is not in structure because table is dumped for every function 

int get_variable_type(var_info *node){
    return node->type;
}
void set_variable_type(var_info *node, int type){
    node->type = type;
}

char* get_variable_name(var_info *node){
    return node->variable_name;
}

void set_variable_name(var_info *node, char* variable_name){
    node->variable_name = VG_(strdup)("vargrind set variable name", variable_name);
}

unsigned long get_variable_address_gs(var_info *node){
    return node->address;
}

void set_variable_address(var_info *node, unsigned long address){
    node->address = address;
}

char* get_declared_at(var_info *node){
    return node->declared_at;
}
void set_declared_at(var_info *node, char* declared_at){
    return node->declared_at = VG_(strdup)( "vargrind set declared at", declared_at);
}

int get_variable_counter(var_info *node){
    return node->counter;
}
void set_variable_counter(var_info *node, int number){
    node->counter = number;
}

long get_variable_size(var_info *node){
    return node->size;
}
void set_variable_size(var_info *node, long size){
    node->size = size;
}

void update_counter(var_info *node){
    int counter = get_variable_counter(node);
    counter ++;
    set_variable_counter(node, counter);
}

void init_counter(var_info *node){
    set_variable_counter(node, 0);
}

var_info *create_new_node(int type, char *variable_name, unsigned long address, char* declared_at,long size){
    var_info *node = (var_info*) VG_(malloc)("node", sizeof(var_info));
    set_variable_type (node, type);
    set_variable_name(node, variable_name);
    set_variable_address(node, address);
    set_declared_at(node, declared_at);
    set_variable_size(node,size);
    //set_counter(node, 0);
    return node;
}

