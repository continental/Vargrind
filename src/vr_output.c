#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_machine.h"
#include "pub_tool_vki.h"
#include "pub_tool_vkiscnums.h"
#include "pub_tool_options.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_oset.h"
#include "pub_tool_hashtable.h"

#include "vr_hash.h"
#include "vr_get_set_node.h"

#include "vr_include.h"
#include "vr_output.h"

extern struct vr_st vr_stats;

void print_node(var_info* node);

VgFile* output_file = NULL;


void open_file(HChar* clo_output_filename){
     //Assert output filename is provided 
    //tl_assert(clo_output_filename);
    //tl_assert(clo_output_filename[0]);

    output_file = VG_(fopen)(clo_output_filename, VKI_O_CREAT | VKI_O_TRUNC | VKI_O_RDWR , VKI_S_IRUSR|VKI_S_IWUSR);
    if(output_file==NULL){
        VG_(printf)("ERROR WHILE OPENING THE FILE:%s!\n",clo_output_filename);
        VG_(exit)(-1);
    }
    VG_(fprintf)(output_file, "=========================================================================Vargrind output=========================================================================\n\n");
    VG_(fprintf)(output_file,"VARIABLE NAME\t\t\t\t\t\t\t\t\tADDRESS\t\t\t\t\t\tTYPE\t\t\t\t\tSIZE\t\t\t\t\tCOUNT\t\t\t\t\tDECLARED AT\n\n");
    VG_(fprintf)(output_file, "=================================================================================================================================================================\n");

}

void print_output(char* name,Addr address,char* type, int count,char* declared_at,long size){
    VG_(printf)("%-45s\t%#-25lX\t%-20s\t%-20d\t%-20lu\t%s\n",name,address,type,size,count,declared_at);
    if(output_file!=NULL)
    {
        output_to_file(name,address,type,count,declared_at,size);
    }

}

void output_to_file(char* var_name,Addr address,char* type,int count,char* declared_at,long size){
    VG_(fprintf)(output_file,"%-45s\t%#-25lX\t%-20s\t%-20d\t%-20lu\t%s\n",var_name,address,type,size,count,declared_at);
    //VG_(printf)("Total Loads: %9llu\n", vr_stats.operations[LoadOp].executed);
    //VG_(printf)("Total Stores: %9llu\n", vr_stats.operations[StoreOp].executed);
}


void print_table(VgHashTable *table, HChar* function_name){
    if(get_show_table()){
        VG_(printf)("\n\n\t\t\t\t\t\t\t-------------------------Function: %s---------------------\n",function_name);
        if(output_file!=NULL)
        {
            VG_(fprintf)(output_file,"\n\n\t\t\t\t\t\t\t-------------------------Function: %s---------------------\n",function_name);
        }
        //some pretty print of these data
        VG_(HT_ResetIter)(table); //reset the table iterator

        vr_hash_node* hash_node = VG_(HT_Next)(table); //we got the first element and moved to the next one

        while(hash_node!=NULL){

            print_node(hash_node->data);

            hash_node = VG_(HT_Next)(table);

        }
    }
}


void print_load_store(Addr addr,long size,char* typeof_var,char* var_name,char* declared_at,operation_type operation){
        char op[7]="";
        switch (operation)
        {
            case LOAD:
                VG_(strcpy)(op,"Load");
                break;
            case STORE:
                VG_(strcpy)(op,"Store");
                break; 
            case MODIFY:
                VG_(strcpy)(op,"Modify");
                break;
            default:
                break;
        }
        VG_(printf)("%s\t0x%lx  \tSize: %lu \tVar type: %s \tName: %s \tDeclared at: %s\n",op, addr, size,typeof_var,var_name,declared_at);
        if(output_file!=NULL){
            VG_(fprintf)(output_file,"%s\t0x%lx  \tSize: %lu \tVar type: %s \tName: %s \tDeclared at: %s\n",op, addr, size,typeof_var,var_name,declared_at);
        }

}

void close_file(){
    VG_(fprintf)(output_file, "=================================================================================================================================================================\n");
    VG_(fprintf)(output_file, "Total Loads: %9llu\n", vr_stats.operations[LoadOp].executed);
    VG_(fprintf)(output_file, "Total Stores: %9llu\n", vr_stats.operations[StoreOp].executed);
        
    VG_(fclose)(output_file);
}

void print_node(var_info* node){
    char* name;
    Addr address;
    static char type[8]="";
    char* declared_at;
    long count=0;
    long size=NULL;
    
    switch (get_variable_type(node))
    {
        case GLOBAL:
         if(get_show_global()){
            
            VG_(strcpy)(type,"Global");

            if(get_show_name()){
                name = VG_(strdup)("assigned the name to pointer",get_variable_name(node));
            }

            if(get_show_address()){
                address = get_variable_address_gs(node);
            }
            
            count=get_variable_counter(node);
            size = get_variable_size(node);

            declared_at = VG_(strdup)("assigned declared_at to pointer",get_declared_at(node));

            print_output(name,address,type,count,declared_at,size);
        }
        break;

        case LOCAL:
        if(get_show_local()){

            VG_(strcpy)(type,"Local");

            
            if(get_show_name()){name = VG_(strdup)("assigned the name to pointer",get_variable_name(node));
            }

            if(get_show_address()){
                address = get_variable_address_gs(node);

            }
            
            count=get_variable_counter(node);
            size = get_variable_size(node);

            declared_at = VG_(strdup)("addigned declared_at to pointer",get_declared_at(node));

            print_output(name,address,type,count,declared_at,size);


        }

        break;

        case DYNAMIC:
            if(get_show_dynamic()){
                VG_(strcpy)(type,"Dynamic");

                
                if(get_show_name()){name = VG_(strdup)("assigned the name to pointer",get_variable_name(node));
                }

                if(get_show_address()){
                    address = get_variable_address_gs(node);

                }
                
                count=get_variable_counter(node);
                size = get_variable_size(node);

                declared_at = VG_(strdup)("addigned declared_at to pointer",get_declared_at(node));

                print_output(name,address,type,count,declared_at,size);

            }
        break;

        case UNKNOWN:
            if(get_show_unknown()){

                VG_(strcpy)(type,"Unknown");

                
                if(get_show_name()){name = VG_(strdup)("assigned the name to pointer",get_variable_name(node));
                }

                if(get_show_address()){
                    address = get_variable_address_gs(node);

                }
                
                count=get_variable_counter(node);
                size = get_variable_size(node);
                size=0;
                declared_at = VG_(strdup)("addigned declared_at to pointer",get_declared_at(node));

                print_output(name,address,type,count,declared_at,size);

            }
        break;
    
        default:
        break;
    }

}
