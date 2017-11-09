#include "Python.h"
#include "hashtable.h"
#include "frameobject.h"
#include "pythread.h"
#include "osdefs.h"

/* Trace memory blocks allocated by PyMem_RawMalloc() */
#define TRACE_RAW_MALLOC

/* Forward declaration */
static void memlimit_stop(void);

#ifdef Py_DEBUG
#  define TRACE_DEBUG
#endif

/* Protected by the GIL */
static struct {
    PyMemAllocatorEx mem;
    PyMemAllocatorEx raw;
    PyMemAllocatorEx obj;
} allocators;

static struct {
    /* Module initialized?
       Variable protected by the GIL */
    enum {
        memlimit_NOT_INITIALIZED,
        memlimit_INITIALIZED,
        memlimit_FINALIZED
    } initialized;

    /* Is memlimit tracing memory allocations?
       Variable protected by the GIL */
    int tracing;

    /* limit of the number of frames in a traceback, 1 by default.
       Variable protected by the GIL. */
    int max_nframe;

    /* use domain in trace key?
       Variable protected by the GIL. */
    int use_domain;

    /* max alloc memory. */
    int max_malloc_size;

    /* max execution time in microsecond. */
    int max_execution_time;

} memlimit_config = {memlimit_NOT_INITIALIZED, 0, 1, 0, 1024*1024 ,100*1000};

#if defined(TRACE_RAW_MALLOC) && defined(WITH_THREAD)
/* This lock is needed because memlimit_free() is called without
   the GIL held from PyMem_RawFree(). It cannot acquire the lock because it
   would introduce a deadlock in PyThreadState_DeleteCurrent(). */
static PyThread_type_lock tables_lock;
#  define TABLES_LOCK() PyThread_acquire_lock(tables_lock, 1)
#  define TABLES_UNLOCK() PyThread_release_lock(tables_lock)
#else
   /* variables are protected by the GIL */
#  define TABLES_LOCK()
#  define TABLES_UNLOCK()
#endif


#define DEFAULT_DOMAIN 0

/* Pack the frame_t structure to reduce the memory footprint. */
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
    uintptr_t ptr;
    _PyTraceMalloc_domain_t domain;
} pointer_t;

/* Pack the frame_t structure to reduce the memory footprint on 64-bit
   architectures: 12 bytes instead of 16. */
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#elif defined(_MSC_VER)
#pragma pack(push, 4)
#endif
{
    /* filename cannot be NULL: "<unknown>" is used if the Python frame
       filename is NULL */
    PyObject *filename;
    unsigned int lineno;
} frame_t;
#ifdef _MSC_VER
#pragma pack(pop)
#endif


typedef struct {
    Py_uhash_t hash;
    int nframe;
    frame_t frames[1];
} traceback_t;

#define TRACEBACK_SIZE(NFRAME) \
        (sizeof(traceback_t) + sizeof(frame_t) * (NFRAME - 1))

#define MAX_NFRAME \
        ((INT_MAX - (int)sizeof(traceback_t)) / (int)sizeof(frame_t) + 1)


static PyObject *unknown_filename = NULL;

/* Trace of a memory block */
typedef struct {
    /* Size of the memory block in bytes */
    size_t size;

    /* Traceback where the memory block was allocated */
    traceback_t *traceback;
} trace_t;


/* Size in bytes of currently traced memory.
   Protected by TABLES_LOCK(). */
static size_t memlimit_traced_memory = 0;

static int memlimit_is_out_off_memory = 0;

/* Peak size in bytes of traced memory.
   Protected by TABLES_LOCK(). */
static size_t memlimit_peak_traced_memory = 0;

/* pointer (void*) => trace (trace_t).
   Protected by TABLES_LOCK(). */
static _Py_hashtable_t *memlimit_traces = NULL;


#ifdef TRACE_DEBUG
static void
memlimit_error(const char *format, ...)
{
    va_list ap;
    fprintf(stderr, "memlimit: ");
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    fflush(stderr);
}
#endif


#if defined(WITH_THREAD) && defined(TRACE_RAW_MALLOC)
#define REENTRANT_THREADLOCAL

/* If your OS does not provide native thread local storage, you can implement
   it manually using a lock. Functions of thread.c cannot be used because
   they use PyMem_RawMalloc() which leads to a reentrant call. */
#if !(defined(_POSIX_THREADS) || defined(NT_THREADS))
#  error "need native thread local storage (TLS)"
#endif

static int memlimit_reentrant_key = -1;

/* Any non-NULL pointer can be used */
#define REENTRANT Py_True

static int
get_reentrant(void)
{
    void *ptr;

    assert(memlimit_reentrant_key != -1);
    ptr = PyThread_get_key_value(memlimit_reentrant_key);
    if (ptr != NULL) {
        assert(ptr == REENTRANT);
        return 1;
    }
    else
        return 0;
}

static void
set_reentrant(int reentrant)
{
    assert(reentrant == 0 || reentrant == 1);
    assert(memlimit_reentrant_key != -1);

    if (reentrant) {
        assert(!get_reentrant());
        PyThread_set_key_value(memlimit_reentrant_key, REENTRANT);
    }
    else {
        assert(get_reentrant());
        PyThread_set_key_value(memlimit_reentrant_key, NULL);
    }
}

#else

/* WITH_THREAD not defined: Python compiled without threads,
   or TRACE_RAW_MALLOC not defined: variable protected by the GIL */
static int memlimit_reentrant = 0;

static int
get_reentrant(void)
{
    return memlimit_reentrant;
}

static void
set_reentrant(int reentrant)
{
    assert(reentrant != memlimit_reentrant);
    memlimit_reentrant = reentrant;
}
#endif


static Py_uhash_t
hashtable_hash_pointer_t(_Py_hashtable_t *ht, const void *pkey)
{
    pointer_t ptr;
    Py_uhash_t hash;

    _Py_HASHTABLE_READ_KEY(ht, pkey, ptr);

    hash = (Py_uhash_t)_Py_HashPointer((void*)ptr.ptr);
    hash ^= ptr.domain;
    return hash;
}


static int
hashtable_compare_pointer_t(_Py_hashtable_t *ht, const void *pkey,
                            const _Py_hashtable_entry_t *entry)
{
    pointer_t ptr1, ptr2;

    _Py_HASHTABLE_READ_KEY(ht, pkey, ptr1);
    _Py_HASHTABLE_ENTRY_READ_KEY(ht, entry, ptr2);

    /* compare pointer before domain, because pointer is more likely to be
       different */
    return (ptr1.ptr == ptr2.ptr && ptr1.domain == ptr2.domain);

}


static _Py_hashtable_t *
hashtable_new(size_t key_size, size_t data_size,
              _Py_hashtable_hash_func hash_func,
              _Py_hashtable_compare_func compare_func)
{
    _Py_hashtable_allocator_t hashtable_alloc = {malloc, free};
    return _Py_hashtable_new_full(key_size, data_size, 0,
                                  hash_func, compare_func,
                                  &hashtable_alloc);
}

static int
memlimit_use_domain_cb(_Py_hashtable_t *old_traces,
                           _Py_hashtable_entry_t *entry, void *user_data)
{
    uintptr_t ptr;
    pointer_t key;
    _Py_hashtable_t *new_traces = (_Py_hashtable_t *)user_data;
    const void *pdata = _Py_HASHTABLE_ENTRY_PDATA(old_traces, entry);

    _Py_HASHTABLE_ENTRY_READ_KEY(old_traces, entry, ptr);
    key.ptr = ptr;
    key.domain = DEFAULT_DOMAIN;

    return _Py_hashtable_set(new_traces,
                             sizeof(key), &key,
                             old_traces->data_size, pdata);
}


/* Convert memlimit_traces from compact key (uintptr_t) to pointer_t key.
 * Return 0 on success, -1 on error. */
static int
memlimit_use_domain(void)
{
    _Py_hashtable_t *new_traces = NULL;

    assert(!memlimit_config.use_domain);

    new_traces = hashtable_new(sizeof(pointer_t),
                               sizeof(trace_t),
                               hashtable_hash_pointer_t,
                               hashtable_compare_pointer_t);
    if (new_traces == NULL) {
        return -1;
    }

    if (_Py_hashtable_foreach(memlimit_traces, memlimit_use_domain_cb,
                              new_traces) < 0)
    {
        _Py_hashtable_destroy(new_traces);
        return -1;
    }

    _Py_hashtable_destroy(memlimit_traces);
    memlimit_traces = new_traces;

    memlimit_config.use_domain = 1;

    return 0;
}


static void
memlimit_remove_trace(_PyTraceMalloc_domain_t domain, uintptr_t ptr)
{
    trace_t trace;
    int removed;

    assert(memlimit_config.tracing);

    if (memlimit_config.use_domain) {
        pointer_t key = {ptr, domain};
        removed = _Py_HASHTABLE_POP(memlimit_traces, key, trace);
    }
    else {
        removed = _Py_HASHTABLE_POP(memlimit_traces, ptr, trace);
    }
    if (!removed) {
        return;
    }

    assert(memlimit_traced_memory >= trace.size);
    memlimit_traced_memory -= trace.size;
}

#define REMOVE_TRACE(ptr) \
            memlimit_remove_trace(DEFAULT_DOMAIN, (uintptr_t)(ptr))


static int
memlimit_add_trace(_PyTraceMalloc_domain_t domain, uintptr_t ptr,
                      size_t size)
{
    pointer_t key = {ptr, domain};
    trace_t trace;
    _Py_hashtable_entry_t* entry;
    int res;

    assert(memlimit_config.tracing);


    if (!memlimit_config.use_domain && domain != DEFAULT_DOMAIN) {
        /* first trace using a non-zero domain whereas traces use compact
           (uintptr_t) keys: switch to pointer_t keys. */
        if (memlimit_use_domain() < 0) {
            return -1;
        }
    }

    if (memlimit_config.use_domain) {
        entry = _Py_HASHTABLE_GET_ENTRY(memlimit_traces, key);
    }
    else {
        entry = _Py_HASHTABLE_GET_ENTRY(memlimit_traces, ptr);
    }

    if (entry != NULL) {
        /* the memory block is already tracked */
        _Py_HASHTABLE_ENTRY_READ_DATA(memlimit_traces, entry, trace);
        assert(memlimit_traced_memory >= trace.size);
        memlimit_traced_memory -= trace.size;

        trace.size = size;
//        trace.traceback = traceback;
        _Py_HASHTABLE_ENTRY_WRITE_DATA(memlimit_traces, entry, trace);
    }
    else {
        trace.size = size;
//        trace.traceback = traceback;

        if (memlimit_config.use_domain) {
            res = _Py_HASHTABLE_SET(memlimit_traces, key, trace);
        }
        else {
            res = _Py_HASHTABLE_SET(memlimit_traces, ptr, trace);
        }
        if (res != 0) {
            return res;
        }
    }

    assert(memlimit_traced_memory <= SIZE_MAX - size);

    memlimit_traced_memory += size;
    if (memlimit_traced_memory > memlimit_peak_traced_memory)
        memlimit_peak_traced_memory = memlimit_traced_memory;
    return 0;
}

#define ADD_TRACE(ptr, size) \
            memlimit_add_trace(DEFAULT_DOMAIN, (uintptr_t)(ptr), size)


static int notice = 0;

static int memory_run_out(void) {
   if (memlimit_is_out_off_memory) {
      if (!notice) {
         notice = 1;
         return 1;
      }
   } else {
      notice = 0;
   }
   return 0;
}

static long long start_time = 0;

static long long get_milliseconds() {
   struct timeval  tv;
   gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000LL + tv.tv_usec * 1LL ;
}

static int time_out(void) {
   struct timeval  tv;
   gettimeofday(&tv, NULL);
   long long time_now = tv.tv_sec * 1000000LL + tv.tv_usec ;

   if (!start_time) {
      return 0;
   }

   if (time_now - start_time >= memlimit_config.max_execution_time ) {
      start_time = 0;
      return 1;
   }
   return 0;
}

static int exit_eval_frame_check(void) {
   if (time_out()) {
      printf("++++++++++time out! exit_eval_frame\n");
      PyErr_Format(PyExc_RuntimeError, "time out!!!");
      return 1;
   }
   if (memory_run_out()) {
      printf("++++++++++memory out! exit_eval_frame\n");
      PyErr_Format(PyExc_RuntimeError, "memory run out!!!");
      return 1;
   }
   return 0;
}

typedef int (*fn_check)(void);
static fn_check old_check = NULL;
extern fn_check set_exit_eval_frame_check(fn_check func);


static void*
memlimit_alloc(int use_calloc, void *ctx, size_t nelem, size_t elsize)
{
    PyMemAllocatorEx *alloc = (PyMemAllocatorEx *)ctx;
    void *ptr;

    assert(elsize == 0 || nelem <= SIZE_MAX / elsize);

    if (nelem * elsize > 1024*100) {
       printf("large memory malloc detect :%ld\n", nelem * elsize);
       return NULL;
    }

    if (memlimit_traced_memory + nelem * elsize >= memlimit_config.max_malloc_size) {
       memlimit_is_out_off_memory = 1;
    }

    if (use_calloc)
        ptr = alloc->calloc(alloc->ctx, nelem, elsize);
    else
        ptr = alloc->malloc(alloc->ctx, nelem * elsize);
    if (ptr == NULL)
        return NULL;

    TABLES_LOCK();
    if (ADD_TRACE(ptr, nelem * elsize) < 0) {
        /* Failed to allocate a trace for the new memory block */
        TABLES_UNLOCK();
        alloc->free(alloc->ctx, ptr);
        return NULL;
    }
    TABLES_UNLOCK();
    return ptr;
}


static void*
memlimit_realloc(void *ctx, void *ptr, size_t new_size)
{
    PyMemAllocatorEx *alloc = (PyMemAllocatorEx *)ctx;
    void *ptr2;

    ptr2 = alloc->realloc(alloc->ctx, ptr, new_size);
    if (ptr2 == NULL)
        return NULL;

    if (ptr != NULL) {
        /* an existing memory block has been resized */

        TABLES_LOCK();

        /* memlimit_add_trace() updates the trace if there is already
           a trace at address (domain, ptr2) */
        if (ptr2 != ptr) {
            REMOVE_TRACE(ptr);
        }

        if (ADD_TRACE(ptr2, new_size) < 0) {
            /* Memory allocation failed. The error cannot be reported to
               the caller, because realloc() may already have shrunk the
               memory block and so removed bytes.

               This case is very unlikely: a hash entry has just been
               released, so the hash table should have at least one free entry.

               The GIL and the table lock ensures that only one thread is
               allocating memory. */
            assert(0 && "should never happen");
        }
        TABLES_UNLOCK();
    }
    else {
        /* new allocation */

        TABLES_LOCK();
        if (ADD_TRACE(ptr2, new_size) < 0) {
            /* Failed to allocate a trace for the new memory block */
            TABLES_UNLOCK();
            alloc->free(alloc->ctx, ptr2);
            return NULL;
        }
        TABLES_UNLOCK();
    }
    return ptr2;
}


static void
memlimit_free(void *ctx, void *ptr)
{
    PyMemAllocatorEx *alloc = (PyMemAllocatorEx *)ctx;

    if (ptr == NULL)
        return;

     /* GIL cannot be locked in PyMem_RawFree() because it would introduce
        a deadlock in PyThreadState_DeleteCurrent(). */

    alloc->free(alloc->ctx, ptr);

    TABLES_LOCK();
    REMOVE_TRACE(ptr);
    TABLES_UNLOCK();
}


static void*
memlimit_alloc_gil(int use_calloc, void *ctx, size_t nelem, size_t elsize)
{
    void *ptr;

    if (get_reentrant()) {
        PyMemAllocatorEx *alloc = (PyMemAllocatorEx *)ctx;
        if (use_calloc)
            return alloc->calloc(alloc->ctx, nelem, elsize);
        else
            return alloc->malloc(alloc->ctx, nelem * elsize);
    }

    /* Ignore reentrant call. PyObjet_Malloc() calls PyMem_Malloc() for
       allocations larger than 512 bytes, don't trace the same memory
       allocation twice. */
    set_reentrant(1);

    ptr = memlimit_alloc(use_calloc, ctx, nelem, elsize);

    set_reentrant(0);
    return ptr;
}


static void*
memlimit_malloc_gil(void *ctx, size_t size)
{
    return memlimit_alloc_gil(0, ctx, 1, size);
}


static void*
memlimit_calloc_gil(void *ctx, size_t nelem, size_t elsize)
{
    return memlimit_alloc_gil(1, ctx, nelem, elsize);
}


static void*
memlimit_realloc_gil(void *ctx, void *ptr, size_t new_size)
{
    void *ptr2;

    if (get_reentrant()) {
        /* Reentrant call to PyMem_Realloc() and PyMem_RawRealloc().
           Example: PyMem_RawRealloc() is called internally by pymalloc
           (_PyObject_Malloc() and  _PyObject_Realloc()) to allocate a new
           arena (new_arena()). */
        PyMemAllocatorEx *alloc = (PyMemAllocatorEx *)ctx;

        ptr2 = alloc->realloc(alloc->ctx, ptr, new_size);
        if (ptr2 != NULL && ptr != NULL) {
            TABLES_LOCK();
            REMOVE_TRACE(ptr);
            TABLES_UNLOCK();
        }
        return ptr2;
    }

    /* Ignore reentrant call. PyObjet_Realloc() calls PyMem_Realloc() for
       allocations larger than 512 bytes. Don't trace the same memory
       allocation twice. */
    set_reentrant(1);

    ptr2 = memlimit_realloc(ctx, ptr, new_size);

    set_reentrant(0);
    return ptr2;
}


#ifdef TRACE_RAW_MALLOC
static void*
memlimit_raw_alloc(int use_calloc, void *ctx, size_t nelem, size_t elsize)
{
#ifdef WITH_THREAD
    PyGILState_STATE gil_state;
#endif
    void *ptr;

    if (get_reentrant()) {
        PyMemAllocatorEx *alloc = (PyMemAllocatorEx *)ctx;
        if (use_calloc)
            return alloc->calloc(alloc->ctx, nelem, elsize);
        else
            return alloc->malloc(alloc->ctx, nelem * elsize);
    }

    /* Ignore reentrant call. PyGILState_Ensure() may call PyMem_RawMalloc()
       indirectly which would call PyGILState_Ensure() if reentrant are not
       disabled. */
    set_reentrant(1);

#ifdef WITH_THREAD
    gil_state = PyGILState_Ensure();
    ptr = memlimit_alloc(use_calloc, ctx, nelem, elsize);
    PyGILState_Release(gil_state);
#else
    ptr = memlimit_alloc(use_calloc, ctx, nelem, elsize);
#endif

    set_reentrant(0);
    return ptr;
}


static void*
memlimit_raw_malloc(void *ctx, size_t size)
{
    return memlimit_raw_alloc(0, ctx, 1, size);
}


static void*
memlimit_raw_calloc(void *ctx, size_t nelem, size_t elsize)
{
    return memlimit_raw_alloc(1, ctx, nelem, elsize);
}


static void*
memlimit_raw_realloc(void *ctx, void *ptr, size_t new_size)
{
#ifdef WITH_THREAD
    PyGILState_STATE gil_state;
#endif
    void *ptr2;

    if (get_reentrant()) {
        /* Reentrant call to PyMem_RawRealloc(). */
        PyMemAllocatorEx *alloc = (PyMemAllocatorEx *)ctx;

        ptr2 = alloc->realloc(alloc->ctx, ptr, new_size);

        if (ptr2 != NULL && ptr != NULL) {
            TABLES_LOCK();
            REMOVE_TRACE(ptr);
            TABLES_UNLOCK();
        }
        return ptr2;
    }

    /* Ignore reentrant call. PyGILState_Ensure() may call PyMem_RawMalloc()
       indirectly which would call PyGILState_Ensure() if reentrant calls are
       not disabled. */
    set_reentrant(1);

#ifdef WITH_THREAD
    gil_state = PyGILState_Ensure();
    ptr2 = memlimit_realloc(ctx, ptr, new_size);
    PyGILState_Release(gil_state);
#else
    ptr2 = memlimit_realloc(ctx, ptr, new_size);
#endif

    set_reentrant(0);
    return ptr2;
}
#endif   /* TRACE_RAW_MALLOC */


/* reentrant flag must be set to call this function and GIL must be held */
static void
memlimit_clear_traces(void)
{
#ifdef WITH_THREAD
    /* The GIL protects variables againt concurrent access */
    assert(PyGILState_Check());
#endif

    TABLES_LOCK();
    _Py_hashtable_clear(memlimit_traces);
    memlimit_traced_memory = 0;
    memlimit_peak_traced_memory = 0;
    TABLES_UNLOCK();

}


static int
memlimit_init(void)
{
    if (memlimit_config.initialized == memlimit_FINALIZED) {
        PyErr_SetString(PyExc_RuntimeError,
                        "the memlimit module has been unloaded");
        return -1;
    }

    if (memlimit_config.initialized == memlimit_INITIALIZED)
        return 0;

    PyMem_GetAllocator(PYMEM_DOMAIN_RAW, &allocators.raw);

#ifdef REENTRANT_THREADLOCAL
    memlimit_reentrant_key = PyThread_create_key();
    if (memlimit_reentrant_key == -1) {
#ifdef MS_WINDOWS
        PyErr_SetFromWindowsErr(0);
#else
        PyErr_SetFromErrno(PyExc_OSError);
#endif
        return -1;
    }
#endif

#if defined(WITH_THREAD) && defined(TRACE_RAW_MALLOC)
    if (tables_lock == NULL) {
        tables_lock = PyThread_allocate_lock();
        if (tables_lock == NULL) {
            PyErr_SetString(PyExc_RuntimeError, "cannot allocate lock");
            return -1;
        }
    }
#endif

    if (memlimit_config.use_domain) {
        memlimit_traces = hashtable_new(sizeof(pointer_t),
                                           sizeof(trace_t),
                                           hashtable_hash_pointer_t,
                                           hashtable_compare_pointer_t);
    }
    else {
        memlimit_traces = hashtable_new(sizeof(uintptr_t),
                                           sizeof(trace_t),
                                           _Py_hashtable_hash_ptr,
                                           _Py_hashtable_compare_direct);
    }

    if (memlimit_traces == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    unknown_filename = PyUnicode_FromString("<unknown>");
    if (unknown_filename == NULL)
        return -1;
    PyUnicode_InternInPlace(&unknown_filename);

    memlimit_config.initialized = memlimit_INITIALIZED;
    return 0;
}


static int
memlimit_start(int max_nframe)
{
    PyMemAllocatorEx alloc;
    size_t size;

    if (memlimit_init() < 0)
        return -1;

    if (memlimit_config.tracing) {
        /* hook already installed: do nothing */
        return 0;
    }

    memlimit_is_out_off_memory = 0;

    assert(1 <= max_nframe && max_nframe <= MAX_NFRAME);
    memlimit_config.max_nframe = max_nframe;

#ifdef TRACE_RAW_MALLOC
    alloc.malloc = memlimit_raw_malloc;
    alloc.calloc = memlimit_raw_calloc;
    alloc.realloc = memlimit_raw_realloc;
    alloc.free = memlimit_free;

    alloc.ctx = &allocators.raw;
    PyMem_GetAllocator(PYMEM_DOMAIN_RAW, &allocators.raw);
    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &alloc);
#endif

    alloc.malloc = memlimit_malloc_gil;
    alloc.calloc = memlimit_calloc_gil;
    alloc.realloc = memlimit_realloc_gil;
    alloc.free = memlimit_free;

    alloc.ctx = &allocators.mem;
    PyMem_GetAllocator(PYMEM_DOMAIN_MEM, &allocators.mem);
    PyMem_SetAllocator(PYMEM_DOMAIN_MEM, &alloc);

    alloc.ctx = &allocators.obj;
    PyMem_GetAllocator(PYMEM_DOMAIN_OBJ, &allocators.obj);
    PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &alloc);

    /* everything is ready: start tracing Python memory allocations */
    memlimit_config.tracing = 1;

    old_check = set_exit_eval_frame_check((void*)exit_eval_frame_check);
    return 0;
}


static void
memlimit_stop(void)
{
    if (!memlimit_config.tracing)
        return;

    /* stop tracing Python memory allocations */
    memlimit_config.tracing = 0;

    /* unregister the hook on memory allocators */
#ifdef TRACE_RAW_MALLOC
    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &allocators.raw);
#endif
    PyMem_SetAllocator(PYMEM_DOMAIN_MEM, &allocators.mem);
    PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &allocators.obj);

    memlimit_clear_traces();

    start_time = 0;

    set_exit_eval_frame_check(old_check);
}

PyDoc_STRVAR(memlimit_is_tracing_doc,
    "is_tracing()->bool\n"
    "\n"
    "True if the memlimit module is tracing Python memory allocations,\n"
    "False otherwise.");


static PyObject*
py_memlimit_is_tracing(PyObject *self)
{
    return PyBool_FromLong(memlimit_config.tracing);
}

PyDoc_STRVAR(memlimit_clear_traces_doc,
    "clear_traces()\n"
    "\n"
    "Clear traces of memory blocks allocated by Python.");


static PyObject*
py_memlimit_clear_traces(PyObject *self)
{
    if (!memlimit_config.tracing)
        Py_RETURN_NONE;

    set_reentrant(1);
    memlimit_clear_traces();
    set_reentrant(0);

    Py_RETURN_NONE;
}



typedef struct {
    _Py_hashtable_t *traces;
    _Py_hashtable_t *tracebacks;
    PyObject *list;
} get_traces_t;


PyDoc_STRVAR(memlimit_start_doc,
    "start(nframe: int=1)\n"
    "\n"
    "Start tracing Python memory allocations. Set also the maximum number \n"
    "of frames stored in the traceback of a trace to nframe.");

static PyObject*
py_memlimit_start(PyObject *self, PyObject *args)
{
    Py_ssize_t nframe = 1;
    int nframe_int;

    if (!PyArg_ParseTuple(args, "|n:start", &nframe))
        return NULL;

    if (nframe < 1 || nframe > MAX_NFRAME) {
        PyErr_Format(PyExc_ValueError,
                     "the number of frames must be in range [1; %i]",
                     MAX_NFRAME);
        return NULL;
    }
    nframe_int = Py_SAFE_DOWNCAST(nframe, Py_ssize_t, int);

    if (memlimit_start(nframe_int) < 0)
        return NULL;

    start_time = get_milliseconds();

    Py_RETURN_NONE;
}

PyDoc_STRVAR(memlimit_stop_doc,
    "stop()\n"
    "\n"
    "Stop tracing Python memory allocations and clear traces\n"
    "of memory blocks allocated by Python.");


static PyObject*
py_memlimit_stop(PyObject *self)
{
    memlimit_stop();
    Py_RETURN_NONE;
}


PyDoc_STRVAR(memlimit_is_out_off_memory_doc,
    "is_out_off_memory() -> int\n"
    "\n"
    "Check whether memory is out off use\n");

static PyObject*
py_memlimit_is_out_off_memory(PyObject *self)
{
    return PyLong_FromLong(memlimit_is_out_off_memory);
}



PyDoc_STRVAR(memlimit_set_max_malloc_size_doc,
    "set_max_malloc_size() -> int\n"
    "\n"
    "Set the maximum size of memory allow to malloc\n" );

static PyObject*
py_memlimit_set_max_malloc_size(PyObject *self, PyObject *args)
{
    Py_ssize_t nsize = 1;

    if (!PyArg_ParseTuple(args, "|n:set_max_malloc_size", &nsize))
        return NULL;

    if (nsize < 0) {
        PyErr_Format(PyExc_ValueError, "memory size must > 0");
        return NULL;
    }

    memlimit_config.max_malloc_size  = nsize;

    Py_RETURN_NONE;
}


PyDoc_STRVAR(memlimit_get_max_malloc_size_doc,
    "get_max_malloc_size() -> int\n"
    "\n"
    "Get the maximum size of memory allow to malloc\n" );

static PyObject*
py_memlimit_get_max_malloc_size(PyObject *self)
{
    return PyLong_FromLong(memlimit_config.max_malloc_size);
}



PyDoc_STRVAR(memlimit_set_max_execution_time_doc,
    "set_max_execution_time() -> int\n"
    "\n"
    "Set the maximum execution time for smart contracts\n" );

static PyObject*
py_memlimit_set_max_execution_time(PyObject *self, PyObject *args)
{
    Py_ssize_t nsize = 1;

    if (!PyArg_ParseTuple(args, "|n:set_max_execution_time", &nsize))
        return NULL;

    if (nsize < 0) {
        PyErr_Format(PyExc_ValueError, "execution time must > 0");
        return NULL;
    }

    memlimit_config.max_execution_time  = nsize;

    Py_RETURN_NONE;
}


PyDoc_STRVAR(memlimit_get_max_execution_time_doc,
    "get_max_execution_time() -> int\n"
    "\n"
    "Get the max execution time\n" );

static PyObject*
py_memlimit_get_max_execution_time(PyObject *self)
{
    return PyLong_FromLong(memlimit_config.max_execution_time);
}


PyDoc_STRVAR(memlimit_get_memlimit_memory_doc,
    "get_memlimit_memory() -> int\n"
    "\n"
    "Get the memory usage in bytes of the memlimit module\n"
    "used internally to trace memory allocations.");

static PyObject*
memlimit_get_memlimit_memory(PyObject *self)
{
    size_t size;
    PyObject *size_obj;

    TABLES_LOCK();
    size += _Py_hashtable_size(memlimit_traces);
    TABLES_UNLOCK();

    size_obj = PyLong_FromSize_t(size);
    return Py_BuildValue("N", size_obj);
}


PyDoc_STRVAR(memlimit_get_traced_memory_doc,
    "get_traced_memory() -> (int, int)\n"
    "\n"
    "Get the current size and peak size of memory blocks traced\n"
    "by the memlimit module as a tuple: (current: int, peak: int).");

static PyObject*
memlimit_get_traced_memory(PyObject *self)
{
    Py_ssize_t size, peak_size;
    PyObject *size_obj, *peak_size_obj;

    if (!memlimit_config.tracing)
        return Py_BuildValue("ii", 0, 0);

    TABLES_LOCK();
    size = memlimit_traced_memory;
    peak_size = memlimit_peak_traced_memory;
    TABLES_UNLOCK();

    size_obj = PyLong_FromSize_t(size);
    peak_size_obj = PyLong_FromSize_t(peak_size);
    return Py_BuildValue("NN", size_obj, peak_size_obj);
}

static PyMethodDef module_methods[] = {
    {"is_tracing", (PyCFunction)py_memlimit_is_tracing,
     METH_NOARGS, memlimit_is_tracing_doc},
    {"clear_traces", (PyCFunction)py_memlimit_clear_traces,
     METH_NOARGS, memlimit_clear_traces_doc},
    {"start", (PyCFunction)py_memlimit_start,
      METH_VARARGS, memlimit_start_doc},
    {"stop", (PyCFunction)py_memlimit_stop,
      METH_NOARGS, memlimit_stop_doc},
     {"set_max_malloc_size", (PyCFunction)py_memlimit_set_max_malloc_size,
     METH_VARARGS, memlimit_set_max_malloc_size_doc},
     {"get_max_malloc_size", (PyCFunction)py_memlimit_get_max_malloc_size,
      METH_NOARGS, memlimit_get_max_malloc_size_doc},
      {"set_max_execution_time", (PyCFunction)py_memlimit_set_max_execution_time,
      METH_VARARGS, memlimit_set_max_execution_time_doc},
      {"get_max_execution_time", (PyCFunction)py_memlimit_get_max_execution_time,
       METH_NOARGS, memlimit_get_max_execution_time_doc},
      {"is_out_off_memory", (PyCFunction)py_memlimit_is_out_off_memory,
       METH_NOARGS, memlimit_is_out_off_memory_doc},
      {"get_memlimit_memory", (PyCFunction)memlimit_get_memlimit_memory,
     METH_NOARGS, memlimit_get_memlimit_memory_doc},
    {"get_traced_memory", (PyCFunction)memlimit_get_traced_memory,
     METH_NOARGS, memlimit_get_traced_memory_doc},

    /* sentinel */
    {NULL, NULL}
};

PyDoc_STRVAR(module_doc,
"Debug module to trace memory blocks allocated by Python.");

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "_memlimit",
    module_doc,
    0, /* non-negative size to be able to unload the module */
    module_methods,
    NULL,
};

PyMODINIT_FUNC
PyInit__memlimit(void)
{
    PyObject *m;
    m = PyModule_Create(&module_def);
    if (m == NULL)
        return NULL;

    if (memlimit_init() < 0)
        return NULL;

    return m;
}

