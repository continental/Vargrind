#ifndef __VR_TRACE_FUNCTIONS_H
#define __VR_TRACE_FUNCTIONS_H

#include "pub_tool_basics.h"

void set_offset(void);
int is_new_function_called(void);
char** get_all_new_functions_names(int nr_of_functions, DiEpoch ep, HChar **function_names);

#endif