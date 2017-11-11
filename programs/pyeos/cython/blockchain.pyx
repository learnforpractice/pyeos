from eostypes_ cimport *

cdef extern from "blockchain_.hpp" namespace "python":
    void* new_apply_context_(char* code, void* trx, void* message);
    int release_apply_context_(void* apply_ctx);
    void set_current_context_(void* context_ptr);
    void apply_message_(void* context_ptr)

cdef class apply_context:
    cdef void* _thisptr
    def __cinit__(self,code: bytes, trx, message):
        cdef uint64_t trx_ptr
        cdef uint64_t msg_ptr
        trx_ptr = trx()
        msg_ptr = message()
        self._thisptr = new_apply_context_(code, <void*>trx_ptr, <void*>msg_ptr);

    def __call__(self):
        return <uint64_t>self._thisptr

    def __dealloc__(self):
#        print('__dealloc__ apply_context', <uint64_t>self._thisptr)

        if self._thisptr:
            release_apply_context_(self._thisptr)
            self._thisptr = NULL

def set_current_context(ctx: apply_context):
    cdef uint64_t ptr
    ptr = ctx()
    set_current_context_(<void*>ptr)

def apply_message(ctx: apply_context):
    cdef uint64_t ptr
    ptr = ctx()
    apply_message_(<void*>ptr)



