#ifndef __VR_OUTPUT_H
#define __VR_OUTPUT_H

#include "pub_tool_tooliface.h"

typedef enum{LOAD,STORE,MODIFY}operation_type;

void close_file(void/*VgFile* output_file*/);
void output_to_file(char* var_name,Addr address,char* type,int count,char* declared_at,long size);
void open_file(HChar* clo_output_filename);
void print_output(char* name,Addr address,char* type, int count,char* declared_at,long size);
void print_table(VgHashTable *table, HChar* function_name);
void print_load_store(Addr addr,long size,char* typeof_var,char* var_name,char* declared_at,operation_type operation);


#endif