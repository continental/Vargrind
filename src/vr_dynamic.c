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

#include "pub_tool_replacemalloc.h"
#define SP_UNUSED(arg) (void)arg;

#include "vr_include.h"

// Memory allocation

void* alloc_block(SizeT req_szB, SizeT req_alignB, Bool is_zeroed) {
    SizeT actual_szB;
    void* p;

    //VG_(printf)("Allocated block size: %d\n",req_szB);
    if (((SSizeT) req_szB) < 0)
        return 0;
    
    // Allocate and zero if necessary.
    p = VG_(cli_malloc)(req_alignB, req_szB);
    if (!p)
        return 0;
    
    if (is_zeroed)
        VG_(memset)(p, 0, req_szB);
    
    actual_szB = VG_(cli_malloc_usable_size)(p);
    tl_assert(actual_szB >= req_szB);
    //if(get_dynamic()) VG_(printf)("Allocated memory block at: 0x%x\n",p);
    return p;
}

void dealloc_block(void* p) {
    //VG_(printf)("Deallocated block size: %d\n",sizeof(p));
    //if(get_dynamic()) VG_(printf)("Deallocated memory block at: 0x%x\n",p);

    VG_(cli_free)(p);
}

void* realloc_block(void* p_old, SizeT new_req_szB) {
    SizeT old_req_szB;
    
    void* p_new;
    if (!p_old)
        return alloc_block(new_req_szB, VG_(clo_alignment), /*is_zeroed*/False);
    
    p_new = alloc_block(new_req_szB, VG_(clo_alignment), /*is_zeroed*/False);
    if (p_new) {
        old_req_szB = VG_(cli_malloc_usable_size)(p_old);
    VG_(memcpy)(p_new, p_old,
                (new_req_szB <= old_req_szB ? new_req_szB : old_req_szB));
    dealloc_block(p_old);
    }
    //if(get_dynamic()) VG_(printf)("Reallocated memory block at: 0x%x\n",p_new);
    return p_new;
}

//functions implementation

void* vr_malloc(ThreadId tid, SizeT szB) {
    SP_UNUSED(tid);
    return alloc_block(szB, VG_(clo_alignment), /*is_zeroed*/False);
}

void* vr___builtin_new(ThreadId tid, SizeT szB) {
    SP_UNUSED(tid);
    return alloc_block(szB, VG_(clo_alignment), /*is_zeroed*/False);
}

void* vr___builtin_new_aligned(ThreadId tid, SizeT szB, SizeT align ){
    SP_UNUSED(tid);
    return alloc_block(szB, align, /*is_zeroed*/False);
}
void* vr___builtin_vec_new(ThreadId tid, SizeT szB) {
    SP_UNUSED(tid);
    return alloc_block(szB, VG_(clo_alignment), /*is_zeroed*/False);
}

void* vr___builtin_vec_new_aligned (ThreadId tid, SizeT szB, SizeT align ){
    SP_UNUSED(tid);
    return alloc_block(szB, align, /*is_zeroed*/False);
}

void* vr_calloc(ThreadId tid, SizeT m, SizeT szB) {
    SP_UNUSED(tid);
    return alloc_block(m*szB, VG_(clo_alignment), /*is_zeroed*/True);
}

void* vr_memalign(ThreadId tid, SizeT alignB, SizeT szB) {
    SP_UNUSED(tid);
    return alloc_block(szB, alignB, False);
}
void vr_free(ThreadId tid, void* p) {
    SP_UNUSED(tid);
    dealloc_block(p);
}
void vr___builtin_delete(ThreadId tid, void* p) {
    SP_UNUSED(tid);
    dealloc_block(p);
}

void vr___builtin_delete_aligned(ThreadId tid, void* p, SizeT align ){
    SP_UNUSED(tid);
    dealloc_block(p);
}

void vr___builtin_vec_delete(ThreadId tid, void* p) {
    SP_UNUSED(tid);
    dealloc_block(p);
}

void vr___builtin_vec_delete_aligned(ThreadId tid, void* p, SizeT align ){
    SP_UNUSED(tid);
    dealloc_block(p);
}

void* vr_realloc(ThreadId tid, void* p_old, SizeT new_szB) {
    SP_UNUSED(tid);
    return realloc_block(p_old, new_szB);
}

SizeT vr_malloc_usable_size(ThreadId tid, void* p) {
    SP_UNUSED(tid);
    return VG_(cli_malloc_usable_size)(p);
}