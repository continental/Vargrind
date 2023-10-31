#ifndef __VR_INCLUDE_H
#define __VR_INCLUDE_H

typedef
   IRExpr 
   IRAtom;

/*get-set for trace variable*/
Bool get_trace(void);
void set_trace(Bool status);


//For dynamic
Bool get_dynamic(void);

HChar* get_output_file(void);
//void set_output_file();

Bool get_show_local(void);

Bool get_show_global(void);

Bool get_show_dynamic(void);

Bool get_show_name(void);

Bool get_show_address(void);

HChar* get_fname(void);

Bool get_merge(void);

Bool get_show_unknown(void);

Bool get_show_table(void);

Bool get_show_load_store(void);


typedef enum { OpLoad=0, OpStore=1 } Op; 

#define N_OPS (OpStore + 1)

/* Events */
#define MAX_DSIZE    512

#include "pub_tool_hashtable.h"
#include "vr_hash.h"


typedef 
   enum { Event_Ir, Event_Dr, Event_Dw, Event_Dm }
   EventKind;

typedef struct {
      EventKind  ekind;
      IRAtom*    addr;
      Int        size;
      IRAtom*    guard; /* :: Ity_I1, or NULL=="always True" */
} Event;

#define N_EVENTS 4

/* Structures for counting */
typedef enum {
    LoadOp, StoreOp, OthersOp
} OpKind;

#define N_OPERATIONS (OthersOp + 1)

typedef struct {
    ULong executed;
} op_stats;

struct vr_st{
    op_stats operations[N_OPERATIONS];
};


struct _vr_hash_node{
	VgHashNode *next;
    UWord key;
    var_info *data;
};

typedef struct _vr_hash_node vr_hash_node;


void set_init_table_flag(Bool status);

void flushEvents(IRSB* sb);


void copy_preceding_first_IMark(Int* i, IRSB** sbOut, IRSB* sbIn);
void check_function_name(IRStmt* st, DiEpoch ep);
void add_event_for_store(IRSB** sbOut, IRStmt* st, IRTypeEnv** tyenv);
void add_event_for_storeG(IRSB** sbOut, IRStmt* st, IRTypeEnv** tyenv);
void add_event_for_loadG(IRSB** sbOut, IRStmt* st);

void addEvent_Ir ( IRSB* sb, IRAtom* iaddr, UInt isize);
void addEvent_Dw_guarded ( IRSB* sb, IRAtom* daddr, Int dsize, IRAtom* guard );
void addEvent_Dw ( IRSB* sb, IRAtom* daddr, Int dsize );
void addEvent_Dr ( IRSB* sb, IRAtom* daddr, Int dsize );
void addEvent_Dr_guarded ( IRSB* sb, IRAtom* daddr, Int dsize, IRAtom* guard );

void* alloc_block(SizeT req_szB, SizeT req_alignB, Bool is_zeroed);

void dealloc_block(void* p);

void* realloc_block(void* p_old, SizeT new_req_szB);

//functions implementation

void* vr_malloc(ThreadId tid, SizeT szB);
void* vr___builtin_new(ThreadId tid, SizeT szB);
void* vr___builtin_new_aligned(ThreadId tid, SizeT szB, SizeT align );
void* vr___builtin_vec_new(ThreadId tid, SizeT szB);
void* vr___builtin_vec_new_aligned (ThreadId tid, SizeT szB, SizeT align );
void* vr_calloc(ThreadId tid, SizeT m, SizeT szB);
void* vr_memalign(ThreadId tid, SizeT alignB, SizeT szB);
void vr_free(ThreadId tid, void* p);
void vr___builtin_delete(ThreadId tid, void* p);
void vr___builtin_delete_aligned(ThreadId tid, void* p, SizeT align );
void vr___builtin_vec_delete(ThreadId tid, void* p);
void vr___builtin_vec_delete_aligned(ThreadId tid, void* p, SizeT align );
void* vr_realloc(ThreadId tid, void* p_old, SizeT new_szB);
SizeT vr_malloc_usable_size(ThreadId tid, void* p);



//VgHashTable *create_table(const HChar* name);
//UInt count_nodes(table);

//var_info* create_new_node(int type, HChar *variable_name, Addr address, char *declared_at);
//void update_counter(var_info* node);
//void init_counter(var_info* node);
//Addr get_variable_address_gs(var_info *node);

#endif
