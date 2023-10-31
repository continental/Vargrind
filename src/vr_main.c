/*--------------------------------------------------------------------*/
/*--- Vargrind: A Valgrind variable usage tool.          vr_main.c ---*/
/*--------------------------------------------------------------------*/

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

//#include "pub_tool_aspacemgr.h"


#include "pub_tool_hashtable.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_addrinfo.h"

#include "vr_include.h"
#include "vargrind.h"
#include "vr_hash.h"
#include "vr_regex.h"
#include "vr_get_set_node.h"
#include "pub_tool_stacktrace.h"
#include "vr_get_set_node.h"
#include "vr_stack.h"
#include "vr_output.h"

struct vr_st vr_stats;

/* Global variables */
DiEpoch ep;
HChar* clo_output_filename = NULL;
static VgFile* output_file = NULL;
Bool init_table = False;

Bool trace = True;

Bool get_trace(){
    return trace;
}

void set_trace(Bool status){
    trace = True; //trace everything
}

/* Default options */ 
static Bool show_local = True;
static Bool show_global = True;
static Bool show_dynamic = True;
static Bool show_name = True;
static Bool show_address = True;
static Bool merge = True;
static Bool show_unknown=True;
static Bool show_table=True;
static Bool show_load_store=False;
static const HChar* fnname = "main";

/* Process command line options
   return True if succeeds and False if fails. */
static Bool vr_process_cmd_line_option(const HChar* arg)
{
    if VG_STR_CLO(arg, "--output-file", clo_output_filename) {}
    else if VG_STR_CLO(arg, "--fnname", fnname) {}
    else if VG_BOOL_CLO(arg, "--show-local", show_local) {}
    else if VG_BOOL_CLO(arg, "--show-global", show_global) {}
    else if VG_BOOL_CLO (arg, "--show-dynamic", show_dynamic) {}
    else if VG_BOOL_CLO (arg, "--show-name", show_name) {}
    else if VG_BOOL_CLO (arg, "--show-address", show_address) {}
    else if VG_BOOL_CLO (arg, "--show-table", show_table) {}
    else if VG_BOOL_CLO (arg, "--show-unknown", show_unknown) {}
    else if VG_BOOL_CLO (arg, "--show-ls", show_load_store) {}
    else if VG_BOOL_CLO (arg, "--merge", show_address) {}
    else
        return False;

    tl_assert(fnname);
    tl_assert(fnname[0]);

    //tl_assert(clo_output_filename);
    //tl_assert(clo_output_filename[0]);


    return True;
    
}

static void vr_print_usage(void)
{  
        VG_(printf)(
"   --output-file=<path>        store data provenances to <path>\n"
"   --fnname=<name>             show calls in this function [main]\n"
"   --show-local=yes|no         show data for local variables [yes]\n"
"   --show-global=yes|no        show data for global variables [yes]\n"
"   --show-dynamic=yes|no       show data for dynamic allocated variables [yes]\n"
"   --show-name=yes|no          show name of disared variables [yes]\n"
"   --show-address=yes|no       show address for desired variables [yes]\n"
"   --show-table=yes|no         show hash table containing variable info [yes]\n"
"   --show-unknown=yes|no       show data for unknown variables [yes]\n"
"   --show-ls=yes|no            show load and stores [no]\n"
"   --merge=yes|no              load-op-store instructions are treated as if"
"                               it makes just one memory reference (a modify) [yes]\n"
    );
}

static void vr_print_debug_usage(void)
{  
    VG_(printf)(
"    (none)\n"
    );
}

static Event events[N_EVENTS];

Int events_used = 0;

HChar* get_output_file(void){
    return clo_output_filename;
}

Bool get_show_local(void){
    return show_local;
}

Bool get_show_global(void){
    return show_global;
}

Bool get_show_dynamic(void){
    return show_dynamic;
}

Bool get_show_name(void){
    return show_name;
}

Bool get_show_address(void){
    return show_address;
}

HChar* get_fname(void){
    return fnname;
}

Bool get_merge(void){
    return merge;
}

Bool get_dynamic(void){
    return show_dynamic;
}

Bool get_show_unknown(void){
    return show_unknown;
}

Bool get_show_table(void){
    return show_table;
}

Bool get_show_load_store(void){
    return show_load_store;
}

static operation_type operation;

const HChar* VG_(print_sectionKind)( VgSectKind kind )
{
   switch (kind) {
      case Vg_SectUnknown: return "Unknown";
      case Vg_SectText:    return "Text";
      case Vg_SectData:    return "Data";
      case Vg_SectBSS:     return "BSS";
      case Vg_SectGOT:     return "GOT";
      case Vg_SectPLT:     return "PLT";
      case Vg_SectOPD:     return "OPD";
      case Vg_SectGOTPLT:  return "GOTPLT";
   }
}

static const HChar* print_segment_kind ( SegKind sk )
{
   switch (sk) {
      case SkAnonC: return "anonymous";
      case SkFileC: return "mapped file";
      case SkShmC:  return "shared memory";
   }
}

static var_type type;


static char* name;
static char* declared;

void get_varname(Addr addr,SizeT size,operation_type op_type){
        
        const HChar* cc; //something we need for malloc function
        char typeof_var[15] = "Local";
        char* declared_at; //if not working set declared_at[512]
        char* var_name;

        AddrInfo ai;
        ai.tag = Addr_Undescribed;
        VG_(describe_addr) (ep, addr, &ai);  //epoch -> ep


        switch (ai.tag){
        case Addr_Variable:;
            char description1[256];
            VG_(sprintf)(description1,"%s",(HChar*)VG_(indexXA)(ai.Addr.Variable.descr1, 0));

            char description2[256];
            VG_(sprintf)(description2,"%s",(HChar*)VG_(indexXA)(ai.Addr.Variable.descr2, 0));

            // We are creating 2 dynamic arrays to store words from description
            HChar* a1;
            HChar* a2;
            int words=10;
            int letters=256;

            char **array1 = (char **)VG_(malloc)(&a1,words * sizeof(char *));
            for (int r = 0; r < words; r++)                                     // dynamically allocate memory of size n for each row
                array1[r] = (char *)VG_(malloc)(&a1,letters * sizeof(char));


            char **array2 = (char **)VG_(malloc)(&a2,words * sizeof(char *));   // dynamically allocate memory of size n for each row
            for (int r = 0; r < words; r++)
                array2[r] = (char *)VG_(malloc)(&a2,letters * sizeof(char));
            

            
            //Here we are splitting the description into words
            int size_array1;
            int size_array2;
            split_description(description1,array1,&size_array1);
            split_description(description2,array2,&size_array2);


            var_name = (char*)VG_(malloc)(&cc,VG_(strlen)(array1[size_array1-1])+1);
            VG_(strcpy)(var_name,array1[size_array1-1]);

            //INSIDE GLOBAL VAR "VARIABLE NAME"    OR     A GLOBAL VARIABLED DECLARED AT
            if(VG_(strcmp)(array1[size_array1-3],"global")==0){
                VG_(sprintf)(typeof_var,"%s","Global");
                declared_at = (char*)VG_(malloc)(&cc,VG_(strlen)(array2[2])+1);
                VG_(sprintf)(declared_at,"%s",array2[2]);
                type=GLOBAL;
            }
            else if(VG_(strcmp)(array2[1],"global")==0)
            {
                VG_(sprintf)(typeof_var,"%s","Global");
                declared_at = (char*)VG_(malloc)(&cc,VG_(strlen)(array2[5])+1);
                VG_(sprintf)(declared_at,"%s",array2[5]);
                type=GLOBAL;

            }
            else
            {
                VG_(sprintf)(typeof_var,"%s","Local");
                declared_at = (char*)VG_(malloc)(&cc,VG_(strlen)(array2[2])+1);
                VG_(sprintf)(declared_at,"%s",array2[2]);
                type=LOCAL;
            }

            //free array memory
            for (int r = 0; r < words; r++){
                VG_(free)(array1[r]);
                VG_(free)(array2[r]);
            }
            VG_(free)(array1);
            VG_(free)(array2);
            break;

        case Addr_Block:
            var_name = (char*)VG_(malloc)(&cc,VG_(strlen)("dynamically allocated block")+1);
            VG_(sprintf)(var_name,"%s","dynamically allocated block");

            declared_at = (char*)VG_(malloc)(&cc,2); //Random number
            VG_(sprintf)(declared_at,"%s","");


            VG_(sprintf)(typeof_var,"%s","Dynamic");
            type=DYNAMIC;
            break;

        case Addr_Stack:
        
            declared_at = (char*)VG_(malloc)(&cc,512); //Random number
            
            var_name = (char*)VG_(malloc)(&cc,VG_(strlen)("Stack variable")+1);
            VG_(sprintf)(var_name,"%s","Stack variable");

            VG_(sprintf)(typeof_var,"%s","Stack");

            if (ai.Addr.Stack.frameNo != -1 && ai.Addr.Stack.IP != 0){
                const HChar *fn;
                Bool  hasfn;
                const HChar *file;
                Bool  hasfile;
                UInt linenum;
                Bool haslinenum;
                PtrdiffT offset;
                if (VG_(get_inst_offset_in_function)( ai.Addr.Stack.epoch, ai.Addr.Stack.IP,&offset))
                haslinenum = VG_(get_linenum) (ai.Addr.Stack.epoch,
                                                ai.Addr.Stack.IP - offset,
                                                &linenum);
                else
                haslinenum = False;

                hasfile = VG_(get_filename)(ai.Addr.Stack.epoch,
                                            ai.Addr.Stack.IP, &file);

                HChar strlinenum[16] = "";   // large enough
                if (hasfile && haslinenum)
                VG_(sprintf)(strlinenum, "%u", linenum);

                hasfn = VG_(get_fnname)(ai.Addr.Stack.epoch,
                                        ai.Addr.Stack.IP, &fn);

                if (hasfn || hasfile)
                    VG_(sprintf)(declared_at,"%ps:%s (function name: %ps)",hasfile ? file : "???", strlinenum,hasfn ? fn : "???");
            }else{
                    VG_(sprintf)(declared_at,"%s","Unknown");
            }
            type=LOCAL;
            break;

        case Addr_SectKind:
            //==19286==  Address 0x308fd0 is in the GOT segment of path
            declared_at = (char*)VG_(malloc)(&cc,512); //Random number

            var_name = (char*)VG_(malloc)(&cc,VG_(strlen)("Unknown")+1);
            VG_(sprintf)(var_name,"%s","Unknown");

            VG_(sprintf)(declared_at,"%ps segment of %ps",VG_(print_sectionKind)(ai.Addr.SectKind.kind), ai.Addr.SectKind.objname);
            type=UNKNOWN;
            break;

        case Addr_BrkSegment:
             //--in the brk data segment
            declared_at = (char*)VG_(malloc)(&cc,512); //Random number

            var_name = (char*)VG_(malloc)(&cc,VG_(strlen)("Unknown")+1);
            VG_(sprintf)(var_name,"%s","Unknown");

            if (addr < ai.Addr.BrkSegment.brk_limit)
                VG_(sprintf)(declared_at,"brk data segment 0x%lx-0x%lx",0/*VG_(brk_base)*/,ai.Addr.BrkSegment.brk_limit - 1);
            else
                VG_(sprintf)(declared_at,"%lu bytes after the brk data segment limit 0x%lx\n",addr - ai.Addr.BrkSegment.brk_limit, ai.Addr.BrkSegment.brk_limit);
            type=UNKNOWN;
            break;

        case Addr_SegmentKind:
                    //Address 0x10879c is in a r-x mapped file /home/uie54092/valgrind/bsw.ds.leap.vargrind/sample segment
            declared_at = (char*)VG_(malloc)(&cc,512); //Random number

            var_name = (char*)VG_(malloc)(&cc,VG_(strlen)("Unknown")+1);
            VG_(sprintf)(var_name,"%s","Unknown");

            VG_(sprintf)(declared_at,"%s%s%s %s%s%ps segment",ai.Addr.SegmentKind.hasR ? "r" : "-",
                    ai.Addr.SegmentKind.hasW ? "w" : "-",
                    ai.Addr.SegmentKind.hasX ? "x" : "-",
                    print_segment_kind(ai.Addr.SegmentKind.segkind),
                    ai.Addr.SegmentKind.filename ? 
                    " " : "",
                    ai.Addr.SegmentKind.filename ? 
                    ai.Addr.SegmentKind.filename : "");
            type=UNKNOWN;
            
            break;

        default:
            declared_at = (char*)VG_(malloc)(&cc,512); //Random number
            VG_(sprintf)(declared_at,"%s","");

            var_name = (char*)VG_(malloc)(&cc,VG_(strlen)("Unknown")+1);
            VG_(sprintf)(var_name,"%s","Unknown");
            type=UNKNOWN;
            break;
        }

        name=VG_(strdup)("Copying var_name",var_name);
        declared = VG_(strdup)("Copying declared_at",declared_at);

        //Is it a Load, Store or modify Operation
        if(get_show_load_store()==True){
            print_load_store(addr, size,typeof_var,var_name,declared_at,op_type);
        }
        
        VG_(free)(declared_at);
        VG_(free)(var_name);
        //VG_(pp_addrinfo) ( addr, &ai );
        VG_(clear_addrinfo) (&ai);


}

vr_hash_node* add_header_to_node(var_info *node){
    vr_hash_node *table_node = VG_(malloc)("table node", sizeof(vr_hash_node));
    table_node->next = NULL;
    table_node->key = hash_then_xor(node);
    table_node->data = node;
    return table_node;
}

static VG_REGPARM(2) void trace_instr(Addr addr, SizeT size)
{}

static VG_REGPARM(2) void trace_load(Addr addr, SizeT size)
{
        operation=0; //LOAD
        get_varname(addr, size,operation);

    const HChar** dname; //output
    PtrdiffT* offset; //output
    
    if (show_address){
        VG_(get_datasym_and_offset)(ep, addr, &dname, &offset);
    }

    VgHashTable *table = stack_peek();

    // node_info = get_variable_info();
    if (table != NULL){    
        var_info *node_info = create_new_node(type, name, addr, declared,size);
        vr_hash_node *node = VG_(HT_lookup) (table, hash_then_xor(node_info));
        if (node == NULL){
            // if node does not exist
            //create node and add it into the table
            init_counter(node_info);
            node = add_header_to_node(node_info);
            VG_(HT_add_node)(table, node);
        }         
        //update node, counter++;
        update_counter(node->data);
    }

}

static VG_REGPARM(2) void trace_store(Addr addr, SizeT size)
{
        operation=1;//STORE
        get_varname(addr, size,operation);

    const HChar** dname; //output
    PtrdiffT* offset; //output
    
    if (show_address){
        VG_(get_datasym_and_offset)(ep, addr, &dname, &offset);
        //VG_(printf)(" Store\t0x%lx\tSize: %lu  \tName: %s\n", addr, size, dname);
    }

    VgHashTable *table = stack_peek();

    if (table != NULL){    
        var_info *node_info = create_new_node(type, name, addr, declared,size);
        vr_hash_node *node = VG_(HT_lookup) (table, hash_then_xor(node_info));
        if (node == NULL){
            // if node does not exist
            //create node and add it into the table
            init_counter(node_info);
            node = add_header_to_node(node_info);
            VG_(HT_add_node)(table, node);
        }
        //update node, counter++;
        update_counter(node->data);
    }
}

static VG_REGPARM(2) void trace_modify(Addr addr, SizeT size)
{
    operation=2;//MODIFY
    get_varname(addr, size,operation);

    const HChar** dname; //output
    PtrdiffT* offset; //output
    
    if (show_address){
        VG_(get_datasym_and_offset)(ep, addr, &dname, &offset);
        //VG_(printf)(" Modify\t0x%lx\tSize: %lu  \tName: %s\n", addr, size, dname);
    }

    VgHashTable *table = stack_peek();

    if (table != NULL){
        var_info *node_info = create_new_node(type, name, addr, declared,size);
        vr_hash_node *node = VG_(HT_lookup) (table, hash_then_xor(node_info));
        if (node == NULL){
            // if node does not exist
            //create node and add it into the table
            init_counter(node_info);
            node = add_header_to_node(node_info);
            VG_(HT_add_node)(table, node);
        }
        update_counter(node->data);
    }

}



void flushEvents(IRSB* sb) {
   Int        i;
   const HChar* helperName;
   void*      helperAddr;
   IRExpr**   argv;
   IRDirty*   di;
   Event*     ev;

   for (i = 0; i < events_used; i++) {

      ev = &events[i];
      
      // Decide on helper fn to call and args to pass it.
      switch (ev->ekind) {
         case Event_Ir: helperName = "trace_instr";
                        helperAddr =  trace_instr;  break;
                        

         case Event_Dr: helperName = "trace_load";
                        helperAddr =  trace_load;   break;

         case Event_Dw: helperName = "trace_store";
                        helperAddr =  trace_store;  break;

         case Event_Dm: helperName = "trace_modify";
                        helperAddr =  trace_modify; break;
         default:
            tl_assert(0);
      }

      // Add the helper.
      argv = mkIRExprVec_2( ev->addr, mkIRExpr_HWord( ev->size ) );
      di   = unsafeIRDirty_0_N( /*regparms*/2, 
                                helperName, VG_(fnptr_to_fnentry)( helperAddr ),
                                argv );
      if (ev->guard) {
         di->guard = ev->guard;
      }
      addStmtToIRSB( sb, IRStmt_Dirty(di) );
   }

   events_used = 0;
}

void addEvent_Ir ( IRSB* sb, IRAtom* iaddr, UInt isize ){
   Event* evt;
   tl_assert( (VG_MIN_INSTR_SZB <= isize && isize <= VG_MAX_INSTR_SZB)
            || VG_CLREQ_SZB == isize );
   if (events_used == N_EVENTS)
      flushEvents(sb);
   tl_assert(events_used >= 0 && events_used < N_EVENTS);
   evt = &events[events_used];
   evt->ekind = Event_Ir;
   evt->addr  = iaddr;
   evt->size  = isize;
   evt->guard = NULL;
   events_used++;

}


void addEvent_Dr_guarded ( IRSB* sb, IRAtom* daddr, Int dsize, IRAtom* guard ){
   Event* evt;
   tl_assert(isIRAtom(daddr));
   tl_assert(dsize >= 1 && dsize <= MAX_DSIZE);
   if (events_used == N_EVENTS)
      flushEvents(sb);
   tl_assert(events_used >= 0 && events_used < N_EVENTS);
   evt = &events[events_used];
   evt->ekind = Event_Dr;
   evt->addr  = daddr;
   evt->size  = dsize;
   evt->guard = guard;
   events_used++;

   vr_stats.operations[LoadOp].executed++;
}

void addEvent_Dr ( IRSB* sb, IRAtom* daddr, Int dsize ) {
   addEvent_Dr_guarded(sb, daddr, dsize, NULL);
}

void addEvent_Dw_guarded ( IRSB* sb, IRAtom* daddr, Int dsize, IRAtom* guard ){
   Event* evt;
   
   tl_assert(isIRAtom(daddr));
   tl_assert(dsize >= 1 && dsize <= MAX_DSIZE);
   if (events_used == N_EVENTS)
      flushEvents(sb);
   tl_assert(events_used >= 0 && events_used < N_EVENTS);
   evt = &events[events_used];
   evt->ekind = Event_Dw;
   evt->addr  = daddr;
   evt->size  = dsize;
   evt->guard = guard;
   events_used++;

   vr_stats.operations[StoreOp].executed++;
   
}

void addEvent_Dw ( IRSB* sb, IRAtom* daddr, Int dsize ){
   Event* lastEvt;
   Event* evt;
   
   tl_assert(isIRAtom(daddr));
   tl_assert(dsize >= 1 && dsize <= MAX_DSIZE);

   // Is it possible to merge this write with the preceding read?
   lastEvt = &events[events_used-1];
   if (events_used > 0
       && lastEvt->ekind == Event_Dr
       && lastEvt->size  == dsize
       && lastEvt->guard == NULL
       && eqIRAtom(lastEvt->addr, daddr))
   {
      lastEvt->ekind = Event_Dm;
      return;
   }

   // No.  Add as normal.
   if (events_used == N_EVENTS)
      flushEvents(sb);
   tl_assert(events_used >= 0 && events_used < N_EVENTS);
   evt = &events[events_used];
   evt->ekind = Event_Dw;
   evt->size  = dsize;
   evt->addr  = daddr;
   evt->guard = NULL;
   events_used++;

   vr_stats.operations[StoreOp].executed++;
}

/* Initialize after processing command line options */
static void vr_post_clo_init(void){
    /* Assert output filename is provided */
    if(get_show_table()){
        VG_(printf)("=========================================================================Vargrind output=========================================================================\n\n");
        VG_(printf)("VARIABLE NAME\t\t\t\t\t\t\t\t\tADDRESS\t\t\t\t\t\tTYPE\t\t\t\t\tSIZE\t\t\t\t\tCOUNT\t\t\t\t\tDECLARED AT\n\n");
        VG_(printf)("=================================================================================================================================================================\n");
    }
    if(get_output_file()!=NULL){
        open_file(clo_output_filename);
    }
}

/* Finalize when the program terminates
    Output string should be updated */
static void vr_fini(Int exitcode){
    VG_(printf)("=================================================================================================================================================================\n");
    VG_(printf)("Total Loads: %9llu\n", vr_stats.operations[LoadOp].executed); //dmsg?
    VG_(printf)("Total Stores: %9llu\n", vr_stats.operations[StoreOp].executed);

    if(get_output_file()!=NULL){
        close_file();
    }
}

void set_hash_table(HChar *function_name){
    //VgHashTable *table = VG_(HT_construct) ( function_name); //possible change of table name
    stack_push(function_name);
}

void set_init_table_flag (Bool status){
    init_table = status;
}

/* Instrument a super block.
 * This function is called only the first time a super block is executed.
 */
static IRSB* vr_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      const VexGuestLayout* layout, 
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
    
    IRTypeEnv* tyenv = sbIn->tyenv;
    IRSB* sbOut = deepCopyIRSBExceptStmts(sbIn);
    Int i;
    ep = VG_(current_DiEpoch)();

    /* Copy verbatim any IR preamble preceding the first IMark */
    i = 0;
    copy_preceding_first_IMark(&i, &sbOut, sbIn);

    for (/*use current i*/; i < sbIn->stmts_used; i++) {
        IRStmt* st = sbIn->stmts[i];

        if (!st || st->tag == Ist_NoOp) continue;

        switch (st->tag) {
            case Ist_IMark:
            {
                check_function_name(st, ep);
                if (get_trace()){
                    addEvent_Ir( sbOut, mkIRExpr_HWord( (HWord)st->Ist.IMark.addr ),
                                st->Ist.IMark.len );
                }
                break;
            }

            case Ist_WrTmp:
            {
                if (get_trace()){
                    IRExpr* data = st->Ist.WrTmp.data;
                    if (data->tag == Iex_Load) {
                        int new_function_called = is_new_function_called();
                        if (new_function_called != 0 || init_table){

                            if (new_function_called >0 || init_table){
                                //get all functions called in meaintime
                                //and create table for every
                                HChar **function_names = (HChar**) VG_(malloc)("debug string", new_function_called*sizeof(HChar*));
                                get_all_new_functions_names(new_function_called, ep, function_names);
                                for (int i=0;i<new_function_called;i++){
                                    set_hash_table(function_names[i]);
                                }
                            }
                            
                            else{
                                //get all functions returned from in meaintime
                                //and drop table for every
                                for (int i=0;i<(-new_function_called);i++){
                                    stack_pop();
                                }
                            }
                            
                            
                        }
                      addEvent_Dr( sbOut, data->Iex.Load.addr,
                                   sizeofIRType(data->Iex.Load.ty) );
                    }
                }
                break;
            }

            case Ist_Store:
            {
                if (get_trace()){
                    int new_function_called = is_new_function_called();
                    
                    
                    if (new_function_called != 0 || init_table){
                        if (new_function_called >0 || init_table){
                            
                            //get all functions called in meaintime
                            //and create table for every
    
                            HChar **function_names = (HChar**) VG_(malloc)("debug string", new_function_called*sizeof(HChar*));
                            get_all_new_functions_names(new_function_called, ep, function_names);
                            for (int i=0;i<new_function_called;i++){
                                set_hash_table(function_names[i]);
                            }
                        }
                        else{
                            //get all functions returned from in meaintime
                            //and drop table for every
                            for (int i=0;i<(-new_function_called);i++){
                                stack_pop();
                            }
                        }
                    }
                    add_event_for_store(&sbOut ,st, &tyenv);
                }
                break;
            }

            case Ist_StoreG: 
            {
                if (get_trace()) {
                    int new_function_called = is_new_function_called();
                    if (new_function_called != 0 || init_table){
                        if (new_function_called >0 || init_table){
                            
                            //get all functions called in meaintime
                            //and create table for every
                            
                            HChar **function_names = (HChar**) VG_(malloc)("debug string", new_function_called*sizeof(HChar*));
                            get_all_new_functions_names(new_function_called, ep, function_names);
                            for (int i=0;i<new_function_called;i++){
                                set_hash_table(function_names[i]);
                            }    
                        }
                        else{
                            //get all functions returned from in meaintime
                            //and drop table for every
                            for (int i=0;i<(-new_function_called);i++){
                            stack_pop();
                            }
                        }
                    }
                    add_event_for_storeG(&sbOut ,st, &tyenv);
                    
                }
                break;
            }

            case Ist_LoadG: {
                if (get_trace()) {
                    int new_function_called = is_new_function_called();
                    if (new_function_called != 0 || init_table){
                        if (new_function_called >0 || init_table){
                            //get all functions called in meaintime
                            //and create table for every
                            HChar **function_names = (HChar**) VG_(malloc)("debug string", new_function_called*sizeof(HChar*));
                            get_all_new_functions_names(new_function_called, ep, function_names);
                            for (int i=0;i<new_function_called;i++){
                                set_hash_table(function_names[i]);
                            }                     
                        }
                        else{
                            //get all functions returned from in meaintime
                            //and drop table for every
                            for (int i=0;i<(-new_function_called);i++){
                                stack_pop();
                            }
                        }                        
                    }
                    add_event_for_loadG(&sbOut, st);
                    
                }
                break;
            }
            default:
                break;
        }

        /* Always add current statement to output super block */
        addStmtToIRSB( sbOut, st );
    }
    flushEvents(sbOut);

    return sbOut;
}

/* Initialize before processing command line options */
static void vr_pre_clo_init(void) {
    VG_(details_name)            ("Vargrind");
    VG_(details_version)         ("v0.1");
    VG_(details_description)     ("A Valgrind tool that analyses variable usage");
    VG_(details_copyright_author)(
      "");
    VG_(details_bug_reports_to)  (VG_BUGS_TO);
    VG_(details_avg_translation_sizeB) ( 275 );

    VG_(basic_tool_funcs)        (vr_post_clo_init,
                                  vr_instrument,
                                  vr_fini);

    VG_(needs_command_line_options)(vr_process_cmd_line_option,
                                    vr_print_usage,
                                    vr_print_debug_usage);

    VG_(needs_malloc_replacement)(vr_malloc,
                                  vr___builtin_new,
                                  vr___builtin_new_aligned,
                                  vr___builtin_vec_new,
                                  vr___builtin_vec_new_aligned,
                                  vr_memalign,
                                  vr_calloc,
                                  vr_free,
                                  vr___builtin_delete,
                                  vr___builtin_delete_aligned,
                                  vr___builtin_vec_delete,
                                  vr___builtin_vec_delete_aligned,
                                  vr_realloc,
                                  vr_malloc_usable_size, 
                                  0);

}

VG_DETERMINE_INTERFACE_VERSION(vr_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
