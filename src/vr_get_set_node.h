#ifndef __VR_GET_SET_NODE_H
#define __VR_GET_SET_NODE_H


var_info *create_new_node(int type, char *variable_name, unsigned long address, char* declared_at,long size);
void init_counter(var_info *node);

unsigned long get_variable_address_gs(var_info *node);
int get_variable_type(var_info *node);
char* get_variable_name(var_info *node);
char* get_declared_at(var_info *node);
int get_variable_counter(var_info *node);
long get_variable_size(var_info *node);

#endif