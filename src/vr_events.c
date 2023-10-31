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


#include "vr_include.h"
#include "vr_trace_functions.h"


void copy_preceding_first_IMark(Int* i, IRSB** sbOut, IRSB* sbIn){
    while (*i < sbIn->stmts_used && sbIn->stmts[*i]->tag != Ist_IMark) {
        addStmtToIRSB( *sbOut, sbIn->stmts[*i] );
        (*i)++;
    }
}


void check_function_name(IRStmt* st, DiEpoch ep){
    const HChar* fnname;
    if ( VG_(get_fnname_if_entry)(ep, st->Ist.IMark.addr, &fnname) ) {
        /* Start tracing at main function and set offset for counting functions on the stack*/
        if( VG_(strcmp)(fnname, "main") == 0 ){
            if (!get_trace()){
                set_trace(True);
                set_offset();
                set_init_table_flag(True);
            }
        }
            /* Stop tracing at exit function */
        else if( VG_(strcmp)(fnname, "exit") == 0 )
            set_trace(False);
    }
}

void add_event_for_store(IRSB** sbOut, IRStmt* st, IRTypeEnv** tyenv){
    IRExpr* data = st->Ist.Store.data;
    IRType type = typeOfIRExpr(*tyenv, data);
    tl_assert(type != Ity_INVALID);
    addEvent_Dw(*sbOut, st->Ist.Store.addr, sizeofIRType(type));
}

void add_event_for_storeG(IRSB** sbOut, IRStmt* st, IRTypeEnv** tyenv){
    IRStoreG* sg   = st->Ist.StoreG.details;
    IRExpr*   data = sg->data;
    IRType    type = typeOfIRExpr(*tyenv, data);
    tl_assert(type != Ity_INVALID);
    addEvent_Dw_guarded( *sbOut, sg->addr, sizeofIRType(type), sg->guard );
}

void add_event_for_loadG(IRSB** sbOut, IRStmt* st){
    IRLoadG* lg       = st->Ist.LoadG.details;
    IRType   type     = Ity_INVALID; /* loaded type */
    IRType   typeWide = Ity_INVALID; /* after implicit widening */
    typeOfIRLoadGOp(lg->cvt, &typeWide, &type);
    tl_assert(type != Ity_INVALID);
    addEvent_Dr_guarded( *sbOut, lg->addr,
                        sizeofIRType(type), lg->guard );
}
