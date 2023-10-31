#include "pub_tool_basics.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_stacktrace.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_options.h"
#include "pub_tool_mallocfree.h"

UInt offset;
UInt number_of_functions_on_stack_in_last_check;

UInt get_num_of_fn_on_st_last_check(void){
   return number_of_functions_on_stack_in_last_check;
}
UInt get_offset(void){
   return offset;
}

void set_num_of_fn_in_last_check(UInt number){
   number_of_functions_on_stack_in_last_check = number ;
}

UInt get_number_of_functions_on_stack(void){
   UInt max_n_ips = VG_(clo_backtrace_size);
   Addr ips[max_n_ips];
   UInt n_ips
            = VG_(get_StackTrace)(VG_(get_running_tid)(), ips, max_n_ips,
                                    NULL/*array to dump SP values in*/,
                                    NULL/*array to dump FP values in*/,
                                    0/*first_ip_delta*/);
   return n_ips;
}

void set_offset(void){
   offset = get_number_of_functions_on_stack();
   set_num_of_fn_in_last_check(0);
   // 0 because we want to is_new_function_called return true if we enter the main function for the first time
}


int is_new_function_called(void){
   
   /* return <0 if there is less functions on stack than in last check
      return 0 if there is equal number of functions on stack like in last check
      return >0 if there is more functions on stack than in last check
      value is number of functions on stack before - new number of functions on stack
   */

   int number_of_functions = get_number_of_functions_on_stack() - get_offset();
   int number_of_functions_old = get_num_of_fn_on_st_last_check();
   if (number_of_functions != number_of_functions_old){
       set_num_of_fn_in_last_check(number_of_functions);       
   }

   return number_of_functions - number_of_functions_old;
}


HChar* get_function_data_from_stack(DiEpoch ep, Addr ip, void* uu_opaque){
   InlIPCursor *iipc = VG_(new_IIPC)(ep, ip);
   HChar* function_name = NULL;
   do {
      const HChar *buf = VG_(describe_IP)(ep, ip, iipc);
      
      int len = VG_(strlen)(buf);
      function_name = (char*) VG_(malloc)("vargrind function name allocation", (len+1)*sizeof(HChar));   
      VG_(strcpy)(function_name, buf);
      
       
      // Increase n to show "at" for only one level.
   } while (VG_(next_IIPC)(iipc));
   VG_(delete_IIPC)(iipc);

   return function_name;

}

char** get_all_new_functions_names(int nr_of_functions, DiEpoch ep, HChar** function_names){
   UInt max_n_ips = VG_(clo_backtrace_size);
   Addr ips[max_n_ips];
   UInt n_ips
      = VG_(get_StackTrace)(VG_(get_running_tid)(), ips, max_n_ips,
                            NULL/*array to dump SP values in*/,
                            NULL/*array to dump FP values in*/,
                            0/*first_ip_delta*/);
   
   Int i;

   for (i = 0; i < nr_of_functions; i++){ //get names only for new
      function_names[i] = get_function_data_from_stack(ep, ips[i], NULL);
   }
   //VG_(printf)("%s\n", function_names[0]);

   return NULL;
}