/*******************************************************************************
 * "THE BEER-WARE LICENSE" (Revision 43):
 * <yeonji@ieee.org> create this project. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a sweety curry rice in return (cuz
 * I cannot drink alcoholic beverages). Yeonji Lee
 *
 *******************************************************************************
 * Sorry for my poor English and appreciate it if you could correct my English.
 * 英語の悪かった申し訳ございません。修正してもらえば助かります。
 * Простите за плохой английский.Было бы полезно если бы вы могли это исправить.
 *******************************************************************************
 *         LibCTON
 *   Yet another object notation library for C language. (Toaru Object Notation)
 *   This project aims to provide a strongly typed object manipulation library
 * used by C language. It is expected to include unique binary and plain text
 * storage formats, and hope to be compatible with markup languages such as
 * JSON and YAML and binary formats such as NBT and ProtoBF, and become a
 * unified object operation interface for C language.
 *
 ******************************************************************************/

#include <core/cton_core.h>
#include <core/cton_llib.h>
#include <limits.h>

static void  cton_string_init(cton_obj *str);
static void  cton_string_delete(cton_obj *str);
static void  cton_array_init(cton_obj *obj);
static void  cton_array_delete(cton_obj *obj);
static void *cton_array_getptr(cton_obj *obj);
static void  cton_hash_init(cton_obj *obj);
static void  cton_hash_delete(cton_obj *obj);

static void cton_int8_init(cton_obj *obj);
static void cton_int16_init(cton_obj *obj);
static void cton_int32_init(cton_obj *obj);
static void cton_int64_init(cton_obj *obj);
static void cton_uint8_init(cton_obj *obj);
static void cton_uint16_init(cton_obj *obj);
static void cton_uint32_init(cton_obj *obj);
static void cton_uint64_init(cton_obj *obj);
static void cton_float8_init(cton_obj *obj);
static void cton_float16_init(cton_obj *obj);
static void cton_float32_init(cton_obj *obj);
static void cton_float64_init(cton_obj *obj);

static void * cton_int8_getptr(cton_obj *obj);
static void * cton_int16_getptr(cton_obj *obj);
static void * cton_int32_getptr(cton_obj *obj);
static void * cton_int64_getptr(cton_obj *obj);
static void * cton_uint8_getptr(cton_obj *obj);
static void * cton_uint16_getptr(cton_obj *obj);
static void * cton_uint32_getptr(cton_obj *obj);
static void * cton_uint64_getptr(cton_obj *obj);
static void * cton_float8_getptr(cton_obj *obj);
static void * cton_float16_getptr(cton_obj *obj);
static void * cton_float32_getptr(cton_obj *obj);
static void * cton_float64_getptr(cton_obj *obj);

static int cton_bool_cmp(cton_obj *a, cton_obj *b);
static int cton_binary_cmp(cton_obj *a, cton_obj *b);
static int cton_string_cmp(cton_obj *a, cton_obj *b);
static int cton_int8_cmp(cton_obj *a, cton_obj *b);
static int cton_int16_cmp(cton_obj *a, cton_obj *b);
static int cton_int32_cmp(cton_obj *a, cton_obj *b);
static int cton_int64_cmp(cton_obj *a, cton_obj *b);
static int cton_uint8_cmp(cton_obj *a, cton_obj *b);
static int cton_uint16_cmp(cton_obj *a, cton_obj *b);
static int cton_uint32_cmp(cton_obj *a, cton_obj *b);
static int cton_uint64_cmp(cton_obj *a, cton_obj *b);
static int cton_float32_cmp(cton_obj *a, cton_obj *b);
static int cton_float64_cmp(cton_obj *a, cton_obj *b);


/*******************************************************************************
 * CTON memory hook
 *
 * CTON requests a memory hook to mamage the memory.
 * CTON will try to create the structure of cton_ctx in the memory pool offered.
 * 
 * alloc hook is necessary, all of the alloc requirement will call this hook.
 * realloc hook is not used currently, it just an reserved hook.
 * if free hook is sat, it will be called when destroying an object.
 * if destroy hook is sat, it will be called when destroying the cton_ctx.
 * 
 ******************************************************************************/

static void *cton_alloc(cton_ctx *ctx, size_t size);
static void  cton_free(cton_ctx *ctx, void *ptr);
static void *cton_realloc(cton_ctx *ctx, void *ptr, size_t ori, size_t new);
static void  cton_pdestroy(cton_ctx *ctx);

/*
 * Just proxy the call of malloc() and free()
 */
static void *cton_std_malloc(void *pool, size_t size)
{
    (void) pool;

    return malloc(size);
}

static void  cton_std_free(void *pool, void *ptr)
{
    (void) pool;

    free(ptr);
}

static void *cton_std_realloc(void *pool, void *ptr, size_t size)
{
    (void) pool;
    
    return realloc(ptr, size);
}

cton_memhook cton_std_hook = {
    NULL, cton_std_malloc, cton_std_realloc, cton_std_free, NULL
};

/*
 * Create a memory hook and return it's pointer.
 * it will not allocate new memory for this pool.
 * and it is not thread safe.
 */
cton_memhook* cton_memhook_init (void * pool,
    void * (*palloc)(void *pool, size_t size),
    void * (*prealloc)(void *pool, void *ptr, size_t size),
    void   (*pfree)(void *pool, void *ptr),
    void   (*pdestroy)(void *pool))
{
    static cton_memhook hook;

    hook.pool     = pool;
    hook.palloc   = palloc;
    hook.prealloc = prealloc;
    hook.pfree    = pfree;
    hook.pdestroy = pdestroy;

    return &hook;
}

/*
 * cton_alloc()
 *
 * DESCRIPTION
 *   Allocates size bytes of memory from mem pool and returns a pointer to the
 *  allocated memory.
 *
 * PARAMETER
 *   ctx: cton context
 *   size: the size that will be allocated.
 */
static void * cton_alloc(cton_ctx *ctx, size_t size)
{
    void * ptr;
    ptr = ctx->memhook.palloc(ctx->memhook.pool, size);

    if (ptr == NULL) {
        cton_seterr(ctx, CTON_ERROR_ALLOC);
    }

    return ptr;
}

static void cton_free(cton_ctx *ctx, void *ptr)
{
    if (ctx->memhook.pfree != NULL) {
        ctx->memhook.pfree(ctx->memhook.pool, ptr);
    }
}

static void * cton_realloc(cton_ctx *ctx,
    void *ptr, size_t size_ori, size_t size_new)
{
    void * new_ptr;

    if (size_new <= size_ori) {
        return ptr;
    }

    if (ctx->memhook.prealloc == NULL) {
        
        new_ptr = cton_alloc(ctx, size_new);
        if (new_ptr == NULL) {
            cton_seterr(ctx, CTON_ERROR_ALLOC);
            return NULL;
        }

        cton_llib_memcpy(new_ptr, ptr, size_ori);

        cton_free(ctx, ptr);

    } else {
        new_ptr = ctx->memhook.prealloc(ctx->memhook.pool, ptr, size_new);
        if (new_ptr == NULL) {
            cton_seterr(ctx, CTON_ERROR_ALLOC);
            return NULL;
        }
    }

    return new_ptr;
}

static void cton_pdestroy(cton_ctx *ctx)
{
    if (ctx->memhook.pdestroy != NULL) {
        ctx->memhook.pdestroy(ctx->memhook.pool);
    }
}



/*******************************************************************************
 *
 *    CTON Common methods
 *
 *******************************************************************************
 *         CTON Context
 *******************************************************************************
 *   CTON context is an object purposed to hold most important information for a
 * context. The same context object does not guarantee thread safety, but you
 * can safely manipulate different objects in different threads.
 ******************************************************************************/


/**
 * cton_init()
 *   - Create cton context object.
 *
 * PARAMETER
 *   hook: the cton memory hook.

 * RETURN VALUE
 *   Handle of CTON context object created or NULL for any exception.
 *
 * DESCRIPTION
 *   cton_init() creates a cton context object and initlize it by memory hook
 * defined by parameter. If NULL pointer is passed, this function will init the
 * default memory hook as just a proxy for malloc/free in C standard library.
 *   In convenient, this object is also allocated from memory pool passed by
 * parameter, so you may not need to call destroy method of cton context, call
 * destroy method of your memory pool will destroy the cton context as well.
 *   At least palloc handle is needed for the memory hook, if the palloc method
 * is NULL, this function will failed and return NULL.
 *   This method is threads safe. But the method to create memory hook is not
 * thread-safe yet, so you may need to treating this method as not thread-safe.
 *
 */
cton_ctx *cton_init(cton_memhook *hook)
{
    cton_ctx *ctx;
    extern cton_memhook cton_std_hook;

    if (hook == NULL) {
        hook = &cton_std_hook;
    }

    if (hook->palloc == NULL) {
        /* palloc handle is necessary for cton_ctx */
        return NULL;
    }

    ctx = hook->palloc(hook->pool, sizeof(cton_ctx));

    if (ctx == NULL) {
        return NULL;
    }

    ctx->memhook.pool     = hook->pool;
    ctx->memhook.palloc   = hook->palloc;
    ctx->memhook.prealloc = hook->prealloc;
    ctx->memhook.pfree    = hook->pfree;
    ctx->memhook.pdestroy = hook->pdestroy;

    ctx->err = CTON_OK;

    ctx->root = NULL;
    ctx->nodes = NULL;
    ctx->nodes_last = NULL;

    return ctx;
}


/**
 * cton_destroy()
 *   - Destroy a cton context object.
 *
 * PARAMETER
 *   ctx: the cton context to be destroied.

 * RETURN VALUE
 *   0 for success or other value for any errors (?)
 *
 * DESCRIPTION
 *   The cton_destroy() function will try to destroy a cton context object. If
 * pdestroy is not NULL in he memory hook, it will just call this handle and
 * destroy the whole memory pool. but if this handle is not set in the memory
 * hook, this function will try to call pfree method for every object created
 * from this cton context. If pfree handle is also not set, this function will
 * return -1 and set the cton_err as INVALID_MHOOK.
 *   
 */
int cton_destory(cton_ctx *ctx)
{
    /** TODO */
    cton_pdestroy(ctx);
    return 0;
}


/**
 * cton_seterr()
 *   - Set error for cton context.
 *
 * PARAMETER
 *   ctx: the cton context.
 *   err: The error that happened.
 *   
 */
void cton_seterr(cton_ctx *ctx, cton_err err)
{
    ctx->err = err;
}


/**
 * cton_geterr()
 *   - Get error for cton context.
 *
 * PARAMETER
 *   ctx: the cton context that holds an error.
 *
 * RETURN VALUE
 *   The error that was set by cton_seterr() in this context.
 *
 * NOTE
 *   You can use `cton_geterr(ctx) == CTON_OK` in condition sentence to confirm
 * no error has occured.
 *   
 */
cton_err cton_geterr(cton_ctx *ctx)
{
    return ctx->err;
}

/**
 * cton_strerr()
 *   - look up the error message string corresponding to an error number.
 *
 * PARAMETER
 *   err: the error number.
 *
 * RETURN VALUE
 *   Printible error message string pointer.
 *   
 */
char * cton_strerr(cton_err err)
{
    static char str[] = "Not Implemented.\n";

    (void) err;

    return str;
}


/*******************************************************************************
 * CTON Object methods
 ******************************************************************************/

typedef struct {
    cton_type   type;
    size_t      obj_size;
    size_t      arr_size;
    void      (*init)(cton_obj *obj);
    void      (*delete)(cton_obj *obj);
    void *    (*getptr)(cton_obj *obj);
    int       (*cmp)(cton_obj *a, cton_obj *b);
} cton_typehook_s;

cton_typehook_s cton_type_hook[CTON_TYPE_CNT] = {
    {
        CTON_INVALID, 0, 0, NULL, NULL, NULL, NULL
    },{
        CTON_OBJECT, 0, sizeof(cton_obj *), NULL, NULL, NULL, NULL
    },{
        CTON_NULL, sizeof(cton_obj), 0, NULL, NULL, NULL, NULL
    },{
        CTON_BOOL, sizeof(struct cton_bool_s), sizeof(enum cton_bool_e),
        NULL, NULL, NULL, cton_bool_cmp
    },{
        CTON_BINARY, sizeof(struct cton_string_s), sizeof(struct cton_string_s),
        cton_string_init, cton_string_delete,
        (void *(*)(cton_obj *))cton_string_getptr,
        cton_binary_cmp
    },{
        CTON_STRING, sizeof(struct cton_string_s), sizeof(struct cton_string_s),
        cton_string_init, cton_string_delete,
        (void *(*)(cton_obj *))cton_string_getptr, cton_string_cmp
    },{
        CTON_ARRAY, sizeof(struct cton_array_s), sizeof(struct cton_array_s),
        cton_array_init, cton_array_delete, cton_array_getptr, NULL
    },{
        CTON_HASH, sizeof(struct cton_hash_s), sizeof(struct cton_hash_s),
        cton_hash_init, cton_hash_delete, NULL, NULL
    },{
        CTON_INT8, sizeof(cton_obj) + sizeof(int8_t), sizeof(int8_t),
        cton_int8_init, NULL, cton_int8_getptr, cton_int8_cmp
    },{
        CTON_INT16, sizeof(cton_obj) + sizeof(int16_t), sizeof(int16_t),
        cton_int16_init, NULL, cton_int16_getptr, cton_int16_cmp
    },{
        CTON_INT32, sizeof(cton_obj) + sizeof(int32_t), sizeof(int32_t),
        cton_int32_init, NULL, cton_int32_getptr, cton_int32_cmp
    },{
        CTON_INT64, sizeof(cton_obj) + sizeof(int64_t), sizeof(int64_t),
        cton_int64_init, NULL, cton_int64_getptr, cton_int64_cmp
    },{
        CTON_UINT8, sizeof(cton_obj) + sizeof(uint8_t), sizeof(uint8_t),
        cton_uint8_init, NULL, cton_uint8_getptr, cton_uint8_cmp
    },{
        CTON_UINT16, sizeof(cton_obj) + sizeof(uint16_t), sizeof(uint16_t),
        cton_uint16_init, NULL, cton_uint16_getptr, cton_uint16_cmp
    },{
        CTON_UINT32, sizeof(cton_obj) + sizeof(uint32_t), sizeof(uint32_t),
        cton_uint32_init, NULL, cton_uint32_getptr, cton_uint32_cmp
    },{
        CTON_UINT64, sizeof(cton_obj) + sizeof(uint64_t), sizeof(uint64_t),
        cton_uint64_init, NULL, cton_uint64_getptr, cton_uint64_cmp
    },{
        CTON_FLOAT8, sizeof(cton_obj) + sizeof(uint8_t), sizeof(uint8_t),
        cton_float8_init, NULL, cton_float8_getptr, NULL
    },{
        CTON_FLOAT16, sizeof(cton_obj) + sizeof(uint16_t), sizeof(uint16_t),
        cton_float16_init, NULL, cton_float16_getptr, NULL
    },{
        CTON_FLOAT32, sizeof(cton_obj) + sizeof(float), sizeof(float),
        cton_float32_init, NULL, cton_float32_getptr, cton_float32_cmp
    },{
        CTON_FLOAT64, sizeof(cton_obj) + sizeof(double), sizeof(double),
        cton_float64_init, NULL, cton_float64_getptr, cton_float64_cmp
    }
};

/**
 * cton_object(s) are managed by cton_ctx by Doubly linked list.
 *
 * cton_ctx.
 *
 */

/*
 * cton_object_create()
 *
 * DESCRIPTION
 *   Create an object with the specific type and add it to the object list.
 *
 * PARAMETER
 *   ctx: cton context
 *   type: the type that the new object will hold
 *
 * RETURN
 *     Handle of new created object or NULL for any exception.
 */
cton_obj * cton_object_create(cton_ctx *ctx, cton_type type)
{
    cton_obj *obj;

    extern cton_typehook_s cton_type_hook[CTON_TYPE_CNT];

    if (type == CTON_INVALID || type == CTON_OBJECT) {
        /* These types are not valid for stand alone object */
        cton_seterr(ctx, CTON_ERROR_CREATE);
        return NULL;
    }

    obj = cton_alloc(ctx, cton_type_hook[type].obj_size);
    if (obj == NULL) {
        return NULL;
    }

    cton_llib_memset(obj, 0, sizeof(cton_obj));

    obj->magic = CTON_STRUCT_MAGIC;
    obj->type  = type;
    obj->prev  = NULL;
    obj->next  = NULL;
    obj->ctx   = ctx;

    /* Insert object into object pool linked list */
    if (ctx->nodes == NULL) {
        ctx->nodes = obj;

    } else {
        obj->prev = ctx->nodes_last;
        ctx->nodes_last->next = obj;

    }

    ctx->nodes_last = obj;

    if (cton_type_hook[type].init != NULL) {
        cton_type_hook[type].init(obj);
    }

    return obj;
}

/*
 * cton_object_delete()
 *
 * DESCRIPTION
 *   Destory an object and recollect its memory.
 *   备忘：如果将来添加了引用计数功能，这个API要不要更改为引用减一？
 *
 * PARAMETER
 *   ctx: The cton context
 *   obj: The object that will be deleted.
 */
void cton_object_delete(cton_obj *obj)
{
    extern cton_typehook_s cton_type_hook[CTON_TYPE_CNT];
    cton_type type;

    if (obj->magic != CTON_STRUCT_MAGIC) {
        /* Not CTON structure */
        return;
    }

    /* Remove this object from object link list */
    if (obj->next != NULL) {
        obj->next->prev = obj->prev;
    }

    if (obj->prev != NULL) {
        obj->prev->next = obj->next;
    }

    if (obj->ctx->nodes == obj) {
        obj->ctx->nodes = obj->next;
    }

    if (obj->ctx->nodes_last == obj) {
        obj->ctx->nodes_last = obj->prev;
    }

    type = obj->type;
    /* Delete context by type specificed hook */
    if (cton_type_hook[type].delete != NULL) {
        cton_type_hook[type].delete(obj);
    }

    /* Free obj structure */
    cton_free(obj->ctx, obj);
}

/*
 * cton_object_gettype()
 *
 * DESCRIPTION
 *   Return the type of given object.
 *
 * PARAMETER
 *   obj: The object which you want to get it's type
 *
 * RETURN
 *   The type of give object or invalid type for NULL ptr.
 */
cton_type cton_object_gettype(cton_obj *obj)
{
    if (obj->type >= CTON_TYPE_CNT || obj->type == CTON_OBJECT) {
        cton_seterr(obj->ctx, CTON_ERROR_INVAL);
        return CTON_INVALID;
    }

    return obj->type;
}

/*
 * cton_object_getvalue()
 *
 * DESCRIPTION
 *   Get the payload pointer of an cton object.
 *
 * PARAMETER
 *   obj: The object which you want to get it's type
 *
 * RETURN
 *   The payload pointer of give object or NULL for NULL ptr.
 */
void * cton_object_getvalue(cton_obj *obj)
{
    extern cton_typehook_s cton_type_hook[CTON_TYPE_CNT];

    if (obj->type >= CTON_TYPE_CNT || obj->type == CTON_INVALID) {
        cton_seterr(obj->ctx, CTON_ERROR_INVAL);
        return NULL;
    }

    if (obj->type == CTON_NULL) {
        return NULL;
    }

    if (cton_type_hook[obj->type].getptr == NULL) {
        return &obj[1];
    }

    return cton_type_hook[obj->type].getptr(obj);
}

cton_ctx * cton_object_getctx(cton_obj *obj)
{
    return obj->ctx;
}


int cton_object_cmp(cton_obj *a, cton_obj *b)
{
    if (cton_objtype(a) != cton_objtype(b)) {
        return (cton_objtype(a) - cton_objtype(b));
    }

    if (cton_type_hook[cton_objtype(a)].cmp == NULL) {
        return a - b;
    }

    return cton_type_hook[cton_objtype(a)].cmp(a, b);
}


/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 *         Bool
 ******************************************************************************/

static int cton_bool_cmp(cton_obj *a, cton_obj *b)
{
    return cton_bool_get(a) - cton_bool_get(b);
}

int cton_bool_set(cton_obj *obj, cton_bool val)
{
    int ret = 0;
    struct cton_bool_s *b;

    b = (struct cton_bool_s *)obj;

    if (val == CTON_TRUE) {
        
        /*
         * Only CTON_TRUE is treated as true, any other value or invalidate
         * values are all treated as false.
         */

        b->val = CTON_TRUE;

    } else {

        b->val = CTON_FALSE;

        if (val != CTON_FALSE) {
            ret = 1;
        }

    }

    return ret;
}

cton_bool cton_bool_get(cton_obj *obj)
{
    struct cton_bool_s *b;

    b = (struct cton_bool_s *)obj;

    if (cton_objtype(obj) != CTON_BOOL) {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
        return CTON_FALSE;
    }

    return b->val;
}


/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 * String
 ******************************************************************************/

static void cton_string_init(cton_obj *obj)
{
    struct cton_string_s *str;

    str       = (struct cton_string_s *)obj;

    str->ptr  = NULL;
    str->len  = 0;
    str->used = 0;
}

static void cton_string_delete(cton_obj *obj)
{
    struct cton_string_s *str;

    str = (struct cton_string_s *)obj;

    cton_free(obj->ctx, str->ptr);
}

/*
 * cton_string_getptr()
 *
 * DESCRIPTION
 *   Get the string pointer of an cton string object.
 *
 * PARAMETER
 *   ctx: The cton context
 *   obj: The string object that pointer will be returned.
 *
 * RETURN
 *   The data pointer of the string object.
 */
void * cton_binary_getptr(cton_obj *obj)
{
    struct cton_string_s *str;

    str = (struct cton_string_s *)obj;

    return (void *)str->ptr;
}

char * cton_string_getptr(cton_obj *obj)
{
    return cton_binary_getptr(obj);
}

/*
 * cton_string_getptr()
 *
 * DESCRIPTION
 *   Get the string pointer of an cton string object.
 *
 * PARAMETER
 *   ctx: The cton context
 *   obj: The string object that pointer will be returned.
 *
 * RETURN
 *   The data pointer of the string object.
 */
size_t cton_binary_getlen(cton_obj *obj)
{
    struct cton_string_s *str;

    str = (struct cton_string_s *)obj;

    return str->used;
}

size_t cton_string_getlen(cton_obj *obj)
{
    return cton_binary_getlen(obj) - 1;
}

int cton_binary_setlen(cton_obj *obj, size_t len)
{
    size_t aligned;
    void * new_ptr;

    struct cton_string_s *bin;

    bin = (struct cton_string_s *)obj;

    if (bin->len == 0) {
        bin->ptr = cton_alloc(obj->ctx, len);
        if (bin->ptr == NULL) {
            return -1;
        }
        bin->len = len;
        bin->used = len;

    } else if (bin->len >= len) {
        bin->used = len;

    } else {
        aligned = cton_llib_align(len, 128);
        new_ptr = cton_realloc(obj->ctx, bin->ptr, bin->len, aligned);
        if (new_ptr == NULL) {
            return -1;
        }

        bin->ptr  = new_ptr;
        bin->len  = aligned;
        bin->used = len;
    }

    return len;
}

int cton_string_setlen(cton_obj *obj, size_t len)
{
    return cton_binary_setlen(obj, len + 1);
}

static int cton_binary_cmp(cton_obj *obj_a, cton_obj *obj_b)
{
    size_t len_a;
    size_t len_b;
    size_t len_cmp;

    volatile int ret;

    struct cton_string_s *bin_a;
    struct cton_string_s *bin_b;

    bin_a = (struct cton_string_s *)obj_a;
    bin_b = (struct cton_string_s *)obj_b;

    len_a = bin_a->used;
    len_b = bin_b->used;

    len_cmp = len_a < len_b ? len_a : len_b;

    ret = 0;
    ret = cton_llib_strncmp((char *)bin_a->ptr, (char *)bin_b->ptr, len_cmp);

    if (ret != 0) {
        return ret;
    }

    if (len_a == len_b) {
        return 0;
    } else if (len_a < len_b) {
        return bin_b->ptr[len_cmp];
    } else {
        return bin_a->ptr[len_cmp];
    }
}

static int cton_string_cmp(cton_obj *a, cton_obj *b)
{
    return cton_binary_cmp(a, b);
}

cton_obj * cton_string_create(cton_ctx *ctx, size_t len, const char *str)
{
    cton_obj *obj;
    char     *ptr;

    obj = cton_object_create(ctx, CTON_STRING);

    cton_string_setlen(obj, len);

    ptr = cton_string_getptr(obj);

    cton_llib_memcpy(ptr, str, len);

    return obj;
}


/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 *         Array
 *******************************************************************************
 *
 *   Array in cton is defined as an ordered data structure that can be access by
 * it's index. All elements in an array should takes the same type. Though, cton
 * object is an special type can be used, and will cause an array holds data in
 * different types. That means the array can storage elements with different
 * types de facto.
 *   The method to create an array object is not as easy as numeric object. You
 * need to call cton_object_create() firstly to create an object with the type
 * of array. Than call the cton_array_settype() to assign the array with a type.
 * You cannot change the type of an array after this step, and you cannot skip
 * this step unless you won't use this array. After assign the type to the array
 * you maybe want to declear the size of this array. Call cton_array_setlen()
 * will allow to set the size of the array, and the size of the array can be
 * changed during the life cycle of the object.
 *
 *******************************************************************************
 *
 *   In this part,all of the function should start with `cton_array_`, and
 * accepts at least 2 parameter in the order of `cton_ctx *ctx, cton_obj *arr`
 *
 ******************************************************************************/

static void cton_array_init(cton_obj *obj)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    arr->ptr      = NULL;
    arr->len      = 0;
    arr->used     = 0;
    arr->sub_type = CTON_INVALID;

}

static void cton_array_delete(cton_obj *obj)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    cton_free(obj->ctx, arr->ptr);
}

static void * cton_array_getptr(cton_obj *obj)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    return arr->ptr;
}

/*
 * cton_array_settype()
 *
 * DESCRIPTION
 *   Assign a sub-object type to an array object.
 *   This fuction will set the arr->payload.arr.sub_type to the type passed by
 * parameter.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object assign it's sub-type.
 *   type: The type the array will hold.
 *
 * RETURN
 *   Zero for success and other value for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *
 */
int cton_array_settype(cton_obj *obj, cton_type type)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    if (arr->sub_type != CTON_INVALID) {
        /* Element type is already set */
        cton_seterr(obj->ctx, CTON_ERROR_RSTSUBTYPE);
        return -1;
    }

    if (type >= CTON_TYPE_CNT || type == CTON_INVALID) {
        cton_seterr(obj->ctx, CTON_ERROR_INVSUBTYPE);
        return -1;
    }

    arr->sub_type = type;

    return 0;
}


cton_type cton_array_gettype(cton_obj *obj)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    return arr->sub_type;
}

/*
 * cton_array_getlen()
 *
 * DESCRIPTION
 *   Get how many sub-objects the array object can hold.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to get it's volume.
 *
 * RETURN
 *   The volume of this array object or zero for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 */
size_t cton_array_getlen(cton_obj *obj)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    return arr->used;
}

/*
 * cton_array_setlen()
 *
 * DESCRIPTION
 *   Change the number of objects an array can contain.
 *
 *   The behavior of this function is determined by the content of the given
 * array object. If the given array object holds a NULL pointer, this function
 * will do a new allocation for the array, and set all of the allocated object
 * to NULL pointer. Or if the array object is not NULL pointered, and given
 * length is bigger than the length allocated in the array, this function will
 * trying to expand the space of the array. If the given is less than the given
 * array object holds, this function won't shrink the space actually, it just
 * set the used flag to the length given. Also, this step won't trying to delete
 * the object in the space vanished.
 *   
 *   Although the type of the array is not referenced by this function, for
 * safety reason, this function still requests type is pre-seted.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to change it's size.
 *   len: The new size this array will be.
 *
 * RETURN
 *   The new length of the array or 0 for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *   CTON_ERROR_SUBTYPE: Sub-type is not assigned.
 *   CTON_ERROR_ALLOC: memory hook returns NULL pointer.
 */
size_t cton_array_setlen(cton_obj *obj, size_t len)
{
    extern cton_typehook_s cton_type_hook[CTON_TYPE_CNT];
    struct cton_array_s *arr;
    void   *ptr;

    arr = (struct cton_array_s *)obj;

    if (arr->sub_type == CTON_INVALID) {
        cton_seterr(obj->ctx, CTON_ERROR_SUBTYPE);
        return 0;
    }

    if (arr->ptr == NULL) {
        /* Space have not been allocated yet */
        ptr = cton_alloc(obj->ctx,
            len * cton_type_hook[arr->sub_type].arr_size);

        if ( ptr == NULL ) {
            return 0;
        }

        arr->ptr  = ptr;
        arr->len  = len;

#if 0
        for (index = 0; index < len; index ++) {
            arr->ptr[index] = NULL;
        }
#endif

        arr->used = len;

    } else if (arr->len < len) {

        ptr = cton_realloc(obj->ctx,
            arr->ptr, arr->len * cton_type_hook[arr->sub_type].arr_size,
            len * cton_type_hook[arr->sub_type].arr_size);

        if ( ptr == NULL ) {
            return 0;
        }

        arr->ptr  = ptr;
        arr->len  = len;

#if 0
        for (index = arr->used; index < len; index ++) {
            arr->ptr[index] = NULL;
        }
#endif

        arr->used = len;

    } else {
        /* Shink this array */
        arr->used = len;
        
    }

    return len;
}

/*
 * cton_array_get()
 *
 * DESCRIPTION
 *   Get the sub object in array container by index.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to get it's sub-object.
 *   index: The index of the object you want to get.
 *
 * RETURN
 *   The pointer to sub-object or NULL for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *   CTON_ERROR_INDEX: The requested index is out of the range.
 */
cton_obj * cton_array_get(cton_obj *obj, size_t index)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    if (arr->used <= index) {
        cton_seterr(obj->ctx, CTON_ERROR_INDEX);
        return NULL;
    }

    return ((cton_obj **)arr->ptr)[index];
}

/*
 * cton_array_set()
 *
 * DESCRIPTION
 *   Put a object into an array container at the position by index.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to get it's sub-object.
 *   index: The index of the object you want to put.
 *
 * RETURN
 *   Zero for success or other for any kinds of error
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *   CTON_ERROR_SUBTYPE: Given object is not match the type condition of array.
 *   CTON_ERROR_INDEX: The requested index is out of the range.
 */
int cton_array_set(cton_obj *obj, cton_obj *item, size_t index)
{
    struct cton_array_s *arr;

    arr = (struct cton_array_s *)obj;

    if (arr->sub_type != CTON_OBJECT && \
        arr->sub_type != cton_objtype(item)) {
        cton_seterr(obj->ctx, CTON_ERROR_SUBTYPE);
        return -1;
    }

    if (arr->used <= index) {
        cton_seterr(obj->ctx, CTON_ERROR_INDEX);
        return -1;
    }

    ((cton_obj **)arr->ptr)[index] = obj;
    return 0;
}

int cton_array_foreach(cton_obj *obj, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, size_t, void*))
{
    struct cton_array_s *arr;
    size_t len;
    size_t index;
    int    ret;
    void  *ptr;

    arr = (struct cton_array_s *)obj;

    ret = 0;
    len = arr->used;
    ptr = arr->ptr;

    for (index = 0; index < len; index ++) {
        ret = func(obj->ctx, ((cton_obj **)ptr)[index], index, rctx);

        if (ret != 0) {
            break;
        }
    }

    return ret;
}


/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 *         Hash
 *******************************************************************************
 *   Hash in cton is defined as a key-value pair structure. It allow's getting
 * value object by key object. Only string object can stands as key object.
 *
 *******************************************************************************
 *
 *   In this part, all of the function should start with `cton_hash_`.
 *
 ******************************************************************************/

#ifndef CTON_HASH_LIST

static void cton_rbtree_left_rotate(cton_rbtree_node_t **root,
    cton_rbtree_node_t *sentinel, cton_rbtree_node_t *node);
static void cton_rbtree_right_rotate(cton_rbtree_node_t **root,
    cton_rbtree_node_t *sentinel, cton_rbtree_node_t *node);

#define cton_rbtree_red(node)               ((node)->color = 1)
#define cton_rbtree_black(node)             ((node)->color = 0)
#define cton_rbtree_is_red(node)            ((node)->color)
#define cton_rbtree_is_black(node)          (!cton_rbtree_is_red(node))
#define cton_rbtree_copy_color(n1, n2)      (n1->color = n2->color)

static cton_rbtree_node_t cton_rbtree_sentinel = {
    NULL, NULL, NULL, NULL, NULL, 0
};

static cton_rbtree_node_t *
cton_rbtree_min(cton_rbtree_node_t *node, cton_rbtree_node_t *sentinel)
{
    while (node->left != sentinel) {
        node = node->left;
    }

    return node;
}


void
cton_rbtree_insert(cton_rbtree_t *tree, cton_rbtree_node_t *node)
{
    cton_rbtree_node_t  **root, *temp, *sentinel, **p;

    /* a binary tree insert */

    root = &tree->root;
    sentinel = tree->sentinel;

    if (*root == sentinel) {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        cton_rbtree_black(node);
        *root = node;

        return;
    }

    temp = *root;

    for ( ;; ) {

        p = cton_object_cmp(node->key, temp->key) > 0 ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    cton_rbtree_red(node);

    /* re-balance tree */

    while (node != *root && cton_rbtree_is_red(node->parent)) {

        if (node->parent == node->parent->parent->left) {
            temp = node->parent->parent->right;

            if (cton_rbtree_is_red(temp)) {
                cton_rbtree_black(node->parent);
                cton_rbtree_black(temp);
                cton_rbtree_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    cton_rbtree_left_rotate(root, sentinel, node);
                }

                cton_rbtree_black(node->parent);
                cton_rbtree_red(node->parent->parent);
                cton_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            temp = node->parent->parent->left;

            if (cton_rbtree_is_red(temp)) {
                cton_rbtree_black(node->parent);
                cton_rbtree_black(temp);
                cton_rbtree_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    cton_rbtree_right_rotate(root, sentinel, node);
                }

                cton_rbtree_black(node->parent);
                cton_rbtree_red(node->parent->parent);
                cton_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }

    cton_rbtree_black(*root);
}

void
cton_rbtree_delete(cton_rbtree_t *tree, cton_rbtree_node_t *node)
{
    uint8_t           red;
    cton_rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    /* a binary tree delete */

    root = &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        temp = node->left;
        subst = node;

    } else {
        subst = cton_rbtree_min(node->right, sentinel);
        temp = subst->right;
    }

    if (subst == *root) {
        *root = temp;
        cton_rbtree_black(temp);

        /* DEBUG stuff */
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->key = 0;

        return;
    }

    red = cton_rbtree_is_red(subst);

    if (subst == subst->parent->left) {
        subst->parent->left = temp;

    } else {
        subst->parent->right = temp;
    }

    if (subst == node) {

        temp->parent = subst->parent;

    } else {

        if (subst->parent == node) {
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        cton_rbtree_copy_color(subst, node);

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key = 0;

    if (red) {
        return;
    }

    /* a delete fixup */

    while (temp != *root && cton_rbtree_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;

            if (cton_rbtree_is_red(w)) {
                cton_rbtree_black(w);
                cton_rbtree_red(temp->parent);
                cton_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (cton_rbtree_is_black(w->left) && cton_rbtree_is_black(w->right)) {
                cton_rbtree_red(w);
                temp = temp->parent;

            } else {
                if (cton_rbtree_is_black(w->right)) {
                    cton_rbtree_black(w->left);
                    cton_rbtree_red(w);
                    cton_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                cton_rbtree_copy_color(w, temp->parent);
                cton_rbtree_black(temp->parent);
                cton_rbtree_black(w->right);
                cton_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (cton_rbtree_is_red(w)) {
                cton_rbtree_black(w);
                cton_rbtree_red(temp->parent);
                cton_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (cton_rbtree_is_black(w->left) && cton_rbtree_is_black(w->right)) {
                cton_rbtree_red(w);
                temp = temp->parent;

            } else {
                if (cton_rbtree_is_black(w->left)) {
                    cton_rbtree_black(w->right);
                    cton_rbtree_red(w);
                    cton_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                cton_rbtree_copy_color(w, temp->parent);
                cton_rbtree_black(temp->parent);
                cton_rbtree_black(w->left);
                cton_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    cton_rbtree_black(temp);
}


static void
cton_rbtree_left_rotate(cton_rbtree_node_t **root, cton_rbtree_node_t *sentinel,
    cton_rbtree_node_t *node)
{
    cton_rbtree_node_t  *temp;

    temp = node->right;
    node->right = temp->left;

    if (temp->left != sentinel) {
        temp->left->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->left) {
        node->parent->left = temp;

    } else {
        node->parent->right = temp;
    }

    temp->left = node;
    node->parent = temp;
}


static void
cton_rbtree_right_rotate(cton_rbtree_node_t **root, cton_rbtree_node_t *sentinel,
    cton_rbtree_node_t *node)
{
    cton_rbtree_node_t  *temp;

    temp = node->left;
    node->left = temp->right;

    if (temp->right != sentinel) {
        temp->right->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->right) {
        node->parent->right = temp;

    } else {
        node->parent->left = temp;
    }

    temp->right = node;
    node->parent = temp;
}

static cton_rbtree_node_t *cton_hash_ssearch(cton_obj *h, const char *k)
{
    struct cton_hash_s *hash;
    cton_rbtree_node_t *current;
    int diff;

    hash = (struct cton_hash_s *)h;

    current = hash->tree.root;

    while (1) {
        if (current == hash->tree.sentinel) {
            return NULL;
        }

        if (cton_objtype(current->key) != CTON_STRING) {
            diff = CTON_STRING - cton_objtype(current->key);
        } else {
            diff = cton_llib_strncmp(k,
                cton_string_getptr(current->key),
                cton_string_getlen(current->key));
        }

        if (diff == 0) {
            break;
        }

        current = diff > 0 ? current->left : current->right;
    }

    return current;
}

static cton_rbtree_node_t *cton_hash_search(cton_obj *h, cton_obj *k)
{
    struct cton_hash_s *hash;
    cton_rbtree_node_t *current;
    int diff;

    hash = (struct cton_hash_s *)h;

    current = hash->tree.root;

    while (1) {
        if (current == hash->tree.sentinel) {
            return NULL;
        }

        diff = cton_object_cmp(k, current->key);

        if (diff == 0) {
            break;
        }

        current = diff > 0 ? current->left : current->right;
    }

    return current;
}

static void cton_hash_remove_item(cton_ctx *ctx,
    cton_obj *obj, cton_rbtree_node_t *item)
{
    struct cton_hash_s *hash;

    hash = (struct cton_hash_s *)obj;

    cton_rbtree_delete(&hash->tree, item);

    cton_free(ctx, item);

    hash->count -= 1;
}

static void cton_hash_insert_item(cton_ctx *ctx,
    cton_obj *obj, cton_rbtree_node_t *item)
{
    struct cton_hash_s *hash;

    (void) ctx;
    hash = (struct cton_hash_s *)obj;

    cton_rbtree_insert(&hash->tree, item);

    hash->count += 1;
}

static void cton_hash_init(cton_obj *obj)
{
    struct cton_hash_s *hash;

    hash = (struct cton_hash_s *)obj;

    hash->tree.root = &cton_rbtree_sentinel;
    hash->tree.sentinel = &cton_rbtree_sentinel;
    hash->count = 0;
}

static void cton_hash_delete(cton_obj *obj)
{
    struct cton_hash_s *hash;

    hash = (struct cton_hash_s *)obj;

    while (hash->tree.root != &cton_rbtree_sentinel) {
        cton_hash_remove_item(obj->ctx, obj, hash->tree.root);
    }
}

cton_obj * cton_hash_get(cton_obj *h, cton_obj *k)
{
    cton_rbtree_node_t *result;

    if (cton_objtype(k) != CTON_STRING) {
        cton_seterr(h->ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    result = cton_hash_search(h,k);

    if (result == NULL) {
        return NULL;
    }

    return result->value;
}

cton_obj * cton_hash_sget(cton_obj *h, const char *k)
{
    cton_rbtree_node_t *result;

    result = cton_hash_ssearch(h,k);

    if (result == NULL) {
        return NULL;
    }

    return result->value;
}

cton_obj * cton_hash_get_s(cton_obj *h, const char *ks)
{
    cton_obj *key;
    cton_obj *val;

    key = cton_string_create(h->ctx, cton_llib_strlen(ks), ks);

    val = cton_hash_get(h, key);

    cton_object_delete(key);

    return val;
}

cton_obj * cton_hash_set(cton_obj *h, cton_obj *k, cton_obj *v)
{
    cton_rbtree_node_t *pos;
    cton_obj *k_ori;

    if (cton_objtype(k) != CTON_STRING) {
        cton_seterr(h->ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    if (cton_objtype(v) == CTON_INVALID) {
        cton_seterr(h->ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    pos = cton_hash_search(h,k);

    if (pos != NULL) {

        if (v == NULL) {
            k_ori = pos->key;
            cton_hash_remove_item(h->ctx, h, pos);
            cton_free(h->ctx, k_ori);
            return v;
        }

        pos->value = v;

    } else {   

        pos = cton_alloc(h->ctx, sizeof(cton_rbtree_node_t));
        if (pos == NULL) {
            cton_seterr(h->ctx, CTON_ERROR_ALLOC);
            return NULL;
        }

        pos->key   = k;
        pos->value = v;

        cton_hash_insert_item(h->ctx, h, pos);
    }

    return v;
}

cton_obj * cton_hash_sset(cton_obj *h, const char *ks, cton_obj *v)
{
    cton_obj *key;
    cton_obj *ret;

    key = cton_string_create(h->ctx, cton_llib_strlen(ks), ks);

    ret = cton_hash_set(h, key, v);

    if (ret == NULL) {
        cton_object_delete(ret);
    }

    return ret;
}

size_t cton_hash_getlen(cton_obj *obj)
{
    struct cton_hash_s *hash;

    hash = (struct cton_hash_s *)obj;

    return hash->count;
}

void cton_rbtree_foreach(cton_ctx *ctx, cton_rbtree_node_t *node, size_t *index,
    void *rctx, int (*func)(cton_ctx *, cton_obj *, cton_obj *, size_t, void*))
{
    if (node != &cton_rbtree_sentinel) {
        cton_rbtree_foreach(ctx, node->left, index, rctx, func);
        func(ctx, node->key, node->value, *index, rctx);
        *index += 1;
        cton_rbtree_foreach(ctx, node->right, index, rctx, func);
    }
}

int cton_hash_foreach(cton_obj *obj, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, cton_obj *, size_t, void*))
{
    struct cton_hash_s *hash;
    size_t index;

    hash = (struct cton_hash_s *)obj;

    index = 0;

    cton_rbtree_foreach(obj->ctx, hash->tree.root, &index, rctx, func);

    return 0;
}

#else

/*
 * cton_hash_search()
 *
 * DESCRIPTION
 *   CTON用hash查找，
 *   在一个哈希表里面寻找一个值.
 *
 * PARAMETER
 *   h: 一个CTON哈希表
 *   k: 一个CTON值
 *
 * RETURN
 *   哈希表的项目（cton_hash_item）.
 *
 * ERRORS
 *   找不到返回NULL.
 */
static cton_hash_item *cton_hash_search(cton_obj *h, cton_obj *k)
{
	cton_hash_item *current;

	current = h->payload.hash.root;

    while (current != NULL) {
    	if (cton_util_strcmp(k, current->key) == 0) {
    		break;
    	}

    	current = current->next;
    }

    return current;
}

/*
 * cton_hash_remove_item()
 *
 * DESCRIPTION
 *   CTON用hash表项目删除，
 *   在一个哈希表里面删除一个值.
 *
 * PARAMETER
 *   hash: 一个CTON哈希表
 *   item: 一个CTON值
 */
static void cton_hash_remove_item(cton_ctx *ctx,
    cton_obj *hash, cton_hash_item *item)
{
    cton_hash_item *current;

    current = hash->payload.hash.root;

    if (item == hash->payload.hash.root) {
        hash->payload.hash.root = item->next;

        if (item == hash->payload.hash.last) {
            hash->payload.hash.last = NULL;
        }

        cton_free(ctx, item);

        return;
    }

    while (current != NULL) {
        if (current->next == item) {
            break;
        }
    }

    if (current != NULL) {

        current->next = item->next;

        if (item == hash->payload.hash.last) {
            hash->payload.hash.last = current;
        }

        cton_free(ctx, item);
    }

    hash->payload.hash.count -= 1;
}

/*
 * cton_hash_insert_item()
 *
 * DESCRIPTION
 *   CTON用hash表项目插入，
 *   在一个哈希表里面插入一个值.
 *
 * PARAMETER
 *   hash: 一个CTON哈希表
 *   item: 一个CTON值
 */
static void cton_hash_insert_item(cton_ctx *ctx,
    cton_obj *hash, cton_hash_item *item)
{
    (void) ctx;

    if (hash->payload.hash.root == NULL) {
        hash->payload.hash.root = item;

    } else {
        hash->payload.hash.last->next = item;
    }

    hash->payload.hash.last = item;
    item->next = NULL;

    hash->payload.hash.count += 1;
}

/*
 * cton_hash_init()
 *
 * DESCRIPTION
 *   CTON用hash表初始化，
 *
 * PARAMETER
 *   obj: 一个CTON哈希表
 */
static void cton_hash_init(cton_obj *obj)
{
    obj->payload.hash.root = NULL;
    obj->payload.hash.last = NULL;
    obj->payload.hash.count = 0;
}

/*
 * cton_hash_delete()
 *
 * DESCRIPTION
 *	 删除整个hash表，
 *   但实际上cton_hash_remove_item()套娃，
 *
 * PARAMETER
 *   obj: 一个CTON哈希表
 */
static void cton_hash_delete(cton_obj *obj)
{
    while (obj->payload.hash.root != NULL) {
        cton_hash_remove_item(obj->ctx, obj, obj->payload.hash.root);
    }
}

/*
 * cton_hash_get()
 *
 * DESCRIPTION
 *	 从hash表里取值.
 *
 * PARAMETER
 *   h: 一个CTON哈希表
 *   k: 一个CTON对象，是个key
 * 
 * RETURN
 *   一个CTON对象，是key对应的值.
 *
 */
cton_obj * cton_hash_get(cton_obj *h, cton_obj *k)
{
	cton_hash_item *result;

    if (cton_objtype(k) != CTON_STRING) {
        cton_seterr(h->ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    result = cton_hash_search(h,k);

    if (result == NULL) {
    	return NULL;
    }

    return result->value;
}

/*
 *  cton_hash_get_s()
 *
 * DESCRIPTION
 *	 从hash表里取值.
 *
 * PARAMETER
 *   h: 一个CTON哈希表
 *   ks: 一个字符串，是个key
 * 
 * RETURN
 *   成功返回一个CTON对象，是key对应的值，失败返回NULL.
 */
cton_obj * cton_hash_get_s(cton_obj *h, const char *ks)
{
    cton_obj *key;
    cton_obj *val;

    key = cton_util_strcstr(h->ctx, ks);

    val = cton_hash_get(h, key);

    cton_object_delete(key);

    return val;
}

/*
 *  cton_hash_set()
 *
 * DESCRIPTION
 *	 在hash表里设定值.
 *
 * PARAMETER
 *   h: 一个CTON哈希表
 *   k: 一个CTON对象，是个key
 *	 v: 一个CTON对象，是key对应的值.
 * 
 * RETURN
 *   成功返回v，失败返回NULL.
 *
 */
cton_obj * cton_hash_set(cton_obj *h, cton_obj *k, cton_obj *v)
{
	cton_hash_item *pos;
	cton_obj *k_ori;

    if (cton_objtype(k) != CTON_STRING) {
        cton_seterr(h->ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    if (cton_objtype(v) == CTON_INVALID) {
        cton_seterr(h->ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    pos = cton_hash_search(h,k);

    if (pos != NULL) {

        if (v == NULL) {
            k_ori = pos->key;
            cton_hash_remove_item(h->ctx, h, pos);
            cton_free(h->ctx, k_ori);
            return v;
        }

    	pos->value = v;

    } else {   

    	pos = cton_alloc(h->ctx, sizeof(cton_hash_item));
    	if (pos == NULL) {
    		cton_seterr(h->ctx, CTON_ERROR_ALLOC);
    		return NULL;
    	}

    	pos->key   = k;
    	pos->value = v;
    	pos->next  = NULL;

        cton_hash_insert_item(h->ctx, h, pos);
    }

	return v;
}


size_t cton_hash_getlen(cton_obj *h)
{
    return h->payload.hash.count;
}


int cton_hash_foreach(cton_obj *hash, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, cton_obj *, size_t, void*))
{
    cton_hash_item *now;
    cton_hash_item *next;
    size_t          index;

    now = hash->payload.hash.root;
    index = 0;

    while (now != NULL) {

        /* Prevent error when `now` is removed by user */
        next = now->next; 

        func(hash->ctx, now->key, now->value, index, rctx);

        now = next;
        index += 1;

    }

    return 0;
}


#endif


/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 *         Numeric
 *******************************************************************************
 *
 *   TODO: CTON holds totally 12 types of numeric. Both of them holds the almost
 * same API as init, get and set methods. So is it possible or is it necessary
 * to create APIs for every type?
 *
 ******************************************************************************/

/*******************************************************************************
 * init
 ******************************************************************************/

static void cton_int8_init(cton_obj *obj)
{
    *(int8_t *)&obj[1] = 0;
}

static void cton_int16_init(cton_obj *obj)
{
    *(int16_t *)&obj[1] = 0;
}

static void cton_int32_init(cton_obj *obj)
{
    *(int32_t *)&obj[1] = 0;
}

static void cton_int64_init(cton_obj *obj)
{
    *(int64_t *)&obj[1] = 0;
}

static void cton_uint8_init(cton_obj *obj)
{
    *(uint8_t *)&obj[1] = 0;
}

static void cton_uint16_init(cton_obj *obj)
{
    *(uint16_t *)&obj[1] = 0;
}

static void cton_uint32_init(cton_obj *obj)
{
    *(uint32_t *)&obj[1] = 0;
}

static void cton_uint64_init(cton_obj *obj)
{
    *(uint64_t *)&obj[1] = 0;
}

static void cton_float8_init(cton_obj *obj)
{
    cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);
}

static void cton_float16_init(cton_obj *obj)
{
    cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);
}

static void cton_float32_init(cton_obj *obj)
{
    *(float *)&obj[1] = 0.0;
}

static void cton_float64_init(cton_obj *obj)
{
    *(double *)&obj[1] = 0.0;
}

/*******************************************************************************
 * getptr
 ******************************************************************************/

static void * cton_int8_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_int16_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_int32_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_int64_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_uint8_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_uint16_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_uint32_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_uint64_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_float8_getptr(cton_obj *obj)
{
    cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);
    return NULL;
}

static void * cton_float16_getptr(cton_obj *obj)
{
    cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);
    return NULL;
}

static void * cton_float32_getptr(cton_obj *obj)
{
    return &obj[1];
}

static void * cton_float64_getptr(cton_obj *obj)
{
    return &obj[1];
}


static int cton_int8_cmp(cton_obj *a, cton_obj *b)
{
    return *(int8_t *)&a[1] - *(int8_t *)&b[1];
}

static int cton_int16_cmp(cton_obj *a, cton_obj *b)
{
    return *(int16_t *)&a[1] - *(int16_t *)&b[1];
}

static int cton_int32_cmp(cton_obj *a, cton_obj *b)
{
    return *(int32_t *)&a[1] - *(int32_t *)&b[1];
}

static int cton_int64_cmp(cton_obj *a, cton_obj *b)
{
    return *(int64_t *)&a[1] - *(int64_t *)&b[1];
}

static int cton_uint8_cmp(cton_obj *a, cton_obj *b)
{
    return *(uint8_t *)&a[1] - *(uint8_t *)&b[1];
}

static int cton_uint16_cmp(cton_obj *a, cton_obj *b)
{
    return *(uint16_t *)&a[1] - *(uint16_t *)&b[1];
}

static int cton_uint32_cmp(cton_obj *a, cton_obj *b)
{
    return *(uint32_t *)&a[1] - *(uint32_t *)&b[1];
}

static int cton_uint64_cmp(cton_obj *a, cton_obj *b)
{
    return *(uint64_t *)&a[1] - *(uint64_t *)&b[1];
}

static int cton_float32_cmp(cton_obj *a, cton_obj *b)
{
    return *(float *)&a[1] - *(float *)&b[1];
}

static int cton_float64_cmp(cton_obj *a, cton_obj *b)
{
    return *(double *)&a[1] - *(double *)&b[1];
}



int64_t cton_numeric_getint(cton_obj *obj)
{
    int64_t val = 0;

    if (obj->type == CTON_INT8) {
        val = *(int8_t *)&obj[1];

    } else if (obj->type == CTON_INT16) {
        val = *(int16_t *)&obj[1];

    } else if (obj->type == CTON_INT32) {
        val = *(int32_t *)&obj[1];

    } else if (obj->type == CTON_INT64) {
        val = *(int64_t *)&obj[1];

    } else {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
    }

    return val;
}

int64_t cton_numeric_setint(cton_obj *obj, int64_t val)
{
    if (obj->type == CTON_INT8) {
        if (val > INT8_MAX || val < INT8_MIN) {
            cton_seterr(obj->ctx, CTON_ERROR_OVF);
        }
        *(int8_t *)&obj[1] = (int8_t)val;

    } else if (obj->type == CTON_INT16) {
        if (val > INT16_MAX || val < INT16_MIN) {
            cton_seterr(obj->ctx, CTON_ERROR_OVF);
        }
        *(int16_t *)&obj[1] = (int16_t)val;

    } else if (obj->type == CTON_INT32) {
        if (val > INT32_MAX || val < INT32_MIN) {
            cton_seterr(obj->ctx, CTON_ERROR_OVF);
        }
        *(int32_t *)&obj[1] = (int32_t)val;

    } else if (obj->type == CTON_INT64) {
        *(int64_t *)&obj[1] = (int64_t)val;

    } else {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
    }

    return val;
}

uint64_t cton_numeric_getuint(cton_obj *obj)
{
    uint64_t val = 0;

    if (obj->type == CTON_UINT8) {
        val = *(uint8_t *)&obj[1];

    } else if (obj->type == CTON_UINT16) {
        val = *(uint16_t *)&obj[1];

    } else if (obj->type == CTON_UINT32) {
        val = *(uint32_t *)&obj[1];

    } else if (obj->type == CTON_UINT64) {
        val = *(uint64_t *)&obj[1];

    } else {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
    }

    return val;
}

double cton_numeric_getfloat(cton_obj *obj)
{
    double val = 0.0;

    if (obj->type == CTON_FLOAT8) {
        cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);

    } else if (obj->type == CTON_FLOAT16) {
        cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);

    } else if (obj->type == CTON_FLOAT32) {
        val = *(float *)&obj[1];

    } else if (obj->type == CTON_FLOAT64) {
        val = *(double *)&obj[1];

    } else {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
    }

    return val;
}

double cton_numeric_setfloat(cton_obj *obj, double val)
{
    if (obj->type == CTON_FLOAT8) {
        cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);

    } else if (obj->type == CTON_FLOAT16) {
        cton_seterr(obj->ctx, CTON_ERROR_IMPLEM);

    } else if (obj->type == CTON_FLOAT32) {
        *(float *)&obj[1] = val;

    } else if (obj->type == CTON_FLOAT64) {
        *(double *)&obj[1] = val;

    } else {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
    }

    return val;
}


