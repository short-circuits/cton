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
    void      (*init)(cton_obj *obj);
    void      (*delete)(cton_obj *obj);
    void *    (*getptr)(cton_obj *obj);
} cton_class_hook_s;

cton_class_hook_s cton_class_hook[CTON_TYPE_CNT] = {
    {
        CTON_INVALID, NULL, NULL, NULL
    },{
        CTON_OBJECT, NULL, NULL, NULL
    },{
        CTON_NULL, NULL, NULL, NULL
    },{
        CTON_BOOL, NULL, NULL, NULL
    },{
        CTON_BINARY, cton_string_init, cton_string_delete,
        (void *(*)(cton_obj *))cton_string_getptr
    },{
        CTON_STRING, cton_string_init, cton_string_delete,
        (void *(*)(cton_obj *))cton_string_getptr
    },{
        CTON_ARRAY, cton_array_init, cton_array_delete, cton_array_getptr
    },{
        CTON_HASH, cton_hash_init, cton_hash_delete, NULL
    },{
        CTON_INT8, cton_int8_init, NULL, cton_int8_getptr
    },{
        CTON_INT16, cton_int16_init, NULL, cton_int16_getptr
    },{
        CTON_INT32, cton_int32_init, NULL, cton_int32_getptr
    },{
        CTON_INT64, cton_int64_init, NULL, cton_int64_getptr
    },{
        CTON_UINT8, cton_uint8_init, NULL, cton_uint8_getptr
    },{
        CTON_UINT16, cton_uint16_init, NULL, cton_uint16_getptr
    },{
        CTON_UINT32, cton_uint32_init, NULL, cton_uint32_getptr
    },{
        CTON_UINT64, cton_uint64_init, NULL, cton_uint64_getptr
    },{
        CTON_FLOAT8, cton_float8_init, NULL, cton_float8_getptr
    },{
        CTON_FLOAT16, cton_float16_init, NULL, cton_float16_getptr
    },{
        CTON_FLOAT32, cton_float32_init, NULL, cton_float32_getptr
    },{
        CTON_FLOAT64, cton_float64_init, NULL, cton_float64_getptr
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

    extern cton_class_hook_s cton_class_hook[CTON_TYPE_CNT];

    if (type == CTON_INVALID || type == CTON_OBJECT) {
        /* These types are not valid for stand alone object */
        cton_seterr(ctx, CTON_ERROR_CREATE);
        return NULL;
    }

    obj = cton_alloc(ctx, sizeof(cton_obj));
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

    if (cton_class_hook[type].init != NULL) {
        cton_class_hook[type].init(obj);
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
    extern cton_class_hook_s cton_class_hook[CTON_TYPE_CNT];
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
    if (cton_class_hook[type].delete != NULL) {
        cton_class_hook[type].delete(obj);
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
    extern cton_class_hook_s cton_class_hook[CTON_TYPE_CNT];

    if (obj->type >= CTON_TYPE_CNT || obj->type == CTON_INVALID) {
        cton_seterr(obj->ctx, CTON_ERROR_INVAL);
        return NULL;
    }

    if (obj->type == CTON_NULL) {
        return NULL;
    }

    if (cton_class_hook[obj->type].getptr == NULL) {
        return &obj->payload;
    }

    return cton_class_hook[obj->type].getptr(obj);
}


/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 *         Bool
 ******************************************************************************/

int cton_bool_set(cton_obj *obj, cton_bool val)
{
    int ret = 0;

    if (val == CTON_TRUE) {
        
        /*
         * Only CTON_TRUE is treated as true, any other value or invalidate
         * values are all treated as false.
         */

        obj->payload.b = CTON_TRUE;

    } else {

        obj->payload.b = CTON_FALSE;

        if (val != CTON_FALSE) {
            ret = 1;
        }

    }

    return ret;
}

cton_bool cton_bool_get(cton_obj *obj)
{
    if (cton_objtype(obj) != CTON_BOOL) {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
        return CTON_FALSE;
    }

    return obj->payload.b;
}


/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 * String
 ******************************************************************************/

static void cton_string_init(cton_obj *str)
{
    str->payload.str.ptr  = NULL;
    str->payload.str.len  = 0;
    str->payload.str.used = 0;
}

static void cton_string_delete(cton_obj *str)
{
    cton_free(str->ctx, str->payload.str.ptr);
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
    return (void *)obj->payload.str.ptr;
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
    return obj->payload.str.used;
}

size_t cton_string_getlen(cton_obj *obj)
{
    return cton_binary_getlen(obj) - 1;
}

int cton_binary_setlen(cton_obj *obj, size_t len)
{
    size_t aligned;
    void * new_ptr;

    if (obj->payload.str.len == 0) {
        obj->payload.str.ptr = cton_alloc(obj->ctx, len);
        if (obj->payload.str.ptr == NULL) {
            return -1;
        }
        obj->payload.str.len = len;
        obj->payload.str.used = len;

    } else if (obj->payload.str.len >= len) {
        obj->payload.str.used = len;

    } else {
        aligned = cton_llib_align(len, 128);
        new_ptr = cton_realloc(obj->ctx, \
            obj->payload.str.ptr, obj->payload.str.len, aligned);
        if (new_ptr == NULL) {
            return -1;
        }

        obj->payload.str.ptr  = new_ptr;
        obj->payload.str.len  = aligned;
        obj->payload.str.used = len;
    }

    return len;
}

int cton_string_setlen(cton_obj *obj, size_t len)
{
    return cton_binary_setlen(obj, len + 1);
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

static void cton_array_init(cton_obj *arr)
{
    arr->payload.arr.ptr  = NULL;
    arr->payload.arr.len  = 0;
    arr->payload.arr.used = 0;
    arr->payload.arr.sub_type = CTON_INVALID;

}

static void cton_array_delete(cton_obj *arr)
{
    cton_free(arr->ctx, arr->payload.arr.ptr);
}

static void * cton_array_getptr(cton_obj *arr)
{
    return arr->payload.arr.ptr;
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
int cton_array_settype(cton_obj *arr, cton_type type)
{
    if (arr->payload.arr.sub_type != CTON_INVALID) {
        /* Element type is already set */
        cton_seterr(arr->ctx, CTON_ERROR_RSTSUBTYPE);
        return -1;
    }

    if (type >= CTON_TYPE_CNT || type == CTON_INVALID) {
        cton_seterr(arr->ctx, CTON_ERROR_INVSUBTYPE);
        return -1;
    }

    arr->payload.arr.sub_type = type;

    return 0;
}


cton_type cton_array_gettype(cton_obj *arr)
{
    return arr->payload.arr.sub_type;
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
size_t cton_array_getlen(cton_obj *arr)
{
    return arr->payload.arr.used;
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
size_t cton_array_setlen(cton_obj *arr, size_t len)
{
    size_t index;
    cton_obj **ptr;

    if (cton_array_gettype(arr) == CTON_INVALID) {
        cton_seterr(arr->ctx, CTON_ERROR_SUBTYPE);
        return 0;
    }

    if (arr->payload.arr.ptr == NULL) {
        /* Space have not been allocated yet */
        ptr  = cton_alloc(arr->ctx, len * sizeof(cton_obj *));

        if ( ptr == NULL ) {
            return 0;
        }

        arr->payload.arr.ptr  = ptr;
        arr->payload.arr.len  = len;

        for (index = 0; index < len; index ++) {
            arr->payload.arr.ptr[index] = NULL;
        }

        arr->payload.arr.used = len;

    } else if (arr->payload.arr.len < len) {

        ptr = cton_realloc(arr->ctx,
            arr->payload.arr.ptr, arr->payload.arr.len * sizeof(cton_obj *),
            len * sizeof(cton_obj *));

        if ( ptr == NULL ) {
            return 0;
        }

        arr->payload.arr.ptr  = ptr;
        arr->payload.arr.len  = len;

        for (index = arr->payload.arr.used; index < len; index ++) {
            arr->payload.arr.ptr[index] = NULL;
        }

        arr->payload.arr.used = len;

    } else {
        /* Shink this array */
        arr->payload.arr.used = len;
        
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
cton_obj * cton_array_get(cton_obj *arr, size_t index)
{
    if (cton_array_getlen(arr) <= index) {
        cton_seterr(arr->ctx, CTON_ERROR_INDEX);
        return NULL;
    }

    return arr->payload.arr.ptr[index];
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
int cton_array_set(cton_obj *arr, cton_obj *obj, size_t index)
{
    if (cton_array_gettype(arr) != CTON_OBJECT && \
        cton_array_gettype(arr) != cton_objtype(obj)) {
        cton_seterr(arr->ctx, CTON_ERROR_SUBTYPE);
        return -1;
    }

    if (cton_array_getlen(arr) <= index) {
        cton_seterr(arr->ctx, CTON_ERROR_INDEX);
        return -1;
    }

    arr->payload.arr.ptr[index] = obj;
    return 0;
}

int cton_array_foreach(cton_obj *arr, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, size_t, void*))
{
    size_t len;
    size_t index;
    int    ret;
    cton_obj **ptr;

    ret = 0;
    len = cton_array_getlen(arr);
    ptr = cton_array_getptr(arr);

    for (index = 0; index < len; index ++) {
        ret = func(arr->ctx, ptr[index], index, rctx);

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

        p = cton_util_strcmp(node->key, temp->key) > 0 ? &temp->left : &temp->right;

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

static cton_rbtree_node_t *cton_hash_search(cton_obj *h, cton_obj *k)
{
    cton_rbtree_node_t *current;
    int diff;

    current = h->payload.hash.root;

    while (1) {
        if (current == h->payload.hash.sentinel) {
            return NULL;
        }

        diff = cton_util_strcmp(k, current->key);

        if (diff == 0) {
            break;
        }

        current = diff > 0 ? current->left : current->right;
    }

    return current;
}

static void cton_hash_remove_item(cton_ctx *ctx,
    cton_obj *hash, cton_rbtree_node_t *item)
{
    cton_rbtree_delete(&hash->payload.hash, item);

    cton_free(ctx, item);

    hash->payload.hash.count -= 1;
}

static void cton_hash_insert_item(cton_ctx *ctx,
    cton_obj *hash, cton_rbtree_node_t *item)
{
    (void) ctx;

    cton_rbtree_insert(&hash->payload.hash, item);

    hash->payload.hash.count += 1;
}

static void cton_hash_init(cton_obj *obj)
{
    obj->payload.hash.root = &cton_rbtree_sentinel;
    obj->payload.hash.sentinel = &cton_rbtree_sentinel;
    obj->payload.hash.count = 0;
}

static void cton_hash_delete(cton_obj *obj)
{
    while (obj->payload.hash.root != &cton_rbtree_sentinel) {
        cton_hash_remove_item(obj->ctx, obj, obj->payload.hash.root);
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

cton_obj * cton_hash_get_s(cton_obj *h, const char *ks)
{
    cton_obj *key;
    cton_obj *val;

    key = cton_util_strcstr(h->ctx, ks);

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

size_t cton_hash_getlen(cton_obj *h)
{
    return h->payload.hash.count;
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

int cton_hash_foreach(cton_obj *hash, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, cton_obj *, size_t, void*))
{
    size_t index;

    index = 0;

    cton_rbtree_foreach(hash->ctx, hash->payload.hash.root, &index, rctx, func);

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

    cton_object_delete(h->ctx, key);

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

        func(h->ctx, now->key, now->value, index, rctx);

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
    obj->payload.i8 = 0;
}

static void cton_int16_init(cton_obj *obj)
{
    obj->payload.i16 = 0;
}

static void cton_int32_init(cton_obj *obj)
{
    obj->payload.i32 = 0;
}

static void cton_int64_init(cton_obj *obj)
{
    obj->payload.i64 = 0;
}

static void cton_uint8_init(cton_obj *obj)
{
    obj->payload.u8 = 0;
}

static void cton_uint16_init(cton_obj *obj)
{
    obj->payload.u16 = 0;
}

static void cton_uint32_init(cton_obj *obj)
{
    obj->payload.u32 = 0;
}

static void cton_uint64_init(cton_obj *obj)
{
    obj->payload.u64 = 0;
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
    obj->payload.f32 = 0.0;
}

static void cton_float64_init(cton_obj *obj)
{
    obj->payload.f64 = 0.0;
}

/*******************************************************************************
 * getptr
 ******************************************************************************/

static void * cton_int8_getptr(cton_obj *obj)
{
    return &obj->payload.i8;
}

static void * cton_int16_getptr(cton_obj *obj)
{
    return &obj->payload.i16;
}

static void * cton_int32_getptr(cton_obj *obj)
{
    return &obj->payload.i32;
}

static void * cton_int64_getptr(cton_obj *obj)
{
    return &obj->payload.i64;
}

static void * cton_uint8_getptr(cton_obj *obj)
{
    return &obj->payload.u8;
}

static void * cton_uint16_getptr(cton_obj *obj)
{
    return &obj->payload.u16;
}

static void * cton_uint32_getptr(cton_obj *obj)
{
    return &obj->payload.u32;
}

static void * cton_uint64_getptr(cton_obj *obj)
{
    return &obj->payload.u64;
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
    return &obj->payload.f32;
}

static void * cton_float64_getptr(cton_obj *obj)
{
    return &obj->payload.f64;
}

int64_t cton_numeric_getint(cton_obj *obj)
{
    int64_t val = 0;

    if (obj->type == CTON_INT8) {
        val = obj->payload.i8;

    } else if (obj->type == CTON_INT16) {
        val = obj->payload.i16;

    } else if (obj->type == CTON_INT32) {
        val = obj->payload.i32;

    } else if (obj->type == CTON_INT64) {
        val = obj->payload.i64;

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
        obj->payload.i8 = (int8_t)val;

    } else if (obj->type == CTON_INT16) {
        if (val > INT16_MAX || val < INT16_MIN) {
            cton_seterr(obj->ctx, CTON_ERROR_OVF);
        }
        obj->payload.i16 = (int16_t)val;

    } else if (obj->type == CTON_INT32) {
        if (val > INT32_MAX || val < INT32_MIN) {
            cton_seterr(obj->ctx, CTON_ERROR_OVF);
        }
        obj->payload.i32 = (int32_t)val;

    } else if (obj->type == CTON_INT64) {
        obj->payload.i64 = (int64_t)val;

    } else {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
    }

    return val;
}

uint64_t cton_numeric_getuint(cton_obj *obj)
{
    uint64_t val = 0;

    if (obj->type == CTON_UINT8) {
        val = obj->payload.u8;

    } else if (obj->type == CTON_UINT16) {
        val = obj->payload.u16;

    } else if (obj->type == CTON_UINT32) {
        val = obj->payload.u32;

    } else if (obj->type == CTON_UINT64) {
        val = obj->payload.u64;

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
        val = obj->payload.f32;

    } else if (obj->type == CTON_FLOAT64) {
        val = obj->payload.f64;

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
        obj->payload.f32 = val;

    } else if (obj->type == CTON_FLOAT64) {
        obj->payload.f64 = val;

    } else {
        cton_seterr(obj->ctx, CTON_ERROR_TYPE);
    }

    return val;
}


/*******************************************************************************
 * CTON util functions
 * 
 ******************************************************************************/


/*
 * cton_util_strcmp()
 *
 * DESCRIPTION
 *   CTON用比较字符串，
 *   先当作一般字符串进行strncmp比较，如果两个字符串相等，则比较空间大小.
 *
 * PARAMETER
 *   s1: 一个CTON结构
 *   s2: 另一个CTON结构
 *
 * RETURN
 *   与strcmp定义一致（如果不等则返回非零值）.
 *
 * ERRORS
 *   好像没有这种东西.
 */
int cton_util_strcmp(cton_obj *s1, cton_obj *s2)
{
    size_t len_s1;
    size_t len_s2;
    size_t len_cmp;

    volatile int ret;

    len_s1 = s1->payload.str.used;
    len_s2 = s2->payload.str.used;

    len_cmp = len_s1 < len_s2 ? len_s1 : len_s2;

    ret = 0;
    ret = cton_llib_strncmp((char *)s1->payload.str.ptr, 
                  (char *)s2->payload.str.ptr, len_cmp - 1);

    if (ret != 0) {
        return ret;
    }

    if (len_s1 == len_s2) {
        return 0;
    } else if (len_s1 < len_s2) {
        return s2->payload.str.ptr[len_cmp];
    } else {
        return s1->payload.str.ptr[len_cmp];
    }
}


cton_obj * cton_util_create_str(cton_ctx *ctx,
    const char *str, char end, char quote)
{
    cton_obj *obj;
    char     *ptr;
    size_t    index;

    for (index = 0; ; index ++) {
        if (str[index] == end) {
            if (index == 0) {
                break;
            } else if (str[index - 1] != quote) {
                break;
            }
        }
    }

    obj = cton_object_create(ctx, CTON_STRING);

    cton_string_setlen(obj, index + 1);

    ptr = cton_string_getptr(obj);

    cton_llib_memcpy(ptr, str, index);

    ptr[index] = 0;

    return obj;
}

cton_obj * cton_util_strcstr(cton_ctx *ctx, const char *cstr)
{
    return cton_util_create_str(ctx, cstr, '\0', '\\');
}


#define CTON_BUFFER_PAGESIZE 4096

cton_buf *cton_util_buffer_create(cton_ctx *ctx)
{
    cton_buf *buf;

    cton_obj *container;

    container = cton_object_create(ctx, CTON_BINARY);
    cton_string_setlen(container, sizeof(cton_buf));

    buf = cton_binary_getptr(container);

    buf->container = container;
    buf->ctx = ctx;
    buf->index = 0;

    buf->arr = cton_object_create(ctx, CTON_ARRAY);

    cton_array_settype(buf->arr, CTON_STRING);
    cton_array_setlen(buf->arr, 0);

    return buf;
}

static int 
cton_util_buffer_destroy_arr(cton_ctx *ctx, cton_obj *obj, size_t i, void *c)
{
    (void) i;
    (void) c;
    cton_object_delete(obj);
    return 0;
}

void cton_util_buffer_destroy(cton_buf *buf)
{
    cton_array_foreach(buf->arr, NULL, cton_util_buffer_destroy_arr);
    cton_object_delete(buf->arr);
    cton_object_delete(buf->container);
}

size_t cton_util_buffer_getlen(cton_buf *buf)
{
    return buf->index;
}

cton_obj *cton_util_buffer_pack(cton_buf *buf, cton_type type)
{
    cton_obj *pack;
    cton_obj *buf_seg;

    size_t buf_cnt;
    size_t buf_index;
    size_t buf_len;
    size_t ch_index;

    char  *o_ptr;
    char  *buf_ptr;

    if (type != CTON_STRING && type != CTON_BINARY) {
        return NULL;
    }

    pack = cton_object_create(buf->ctx, type);
    if (type == CTON_STRING) {
        /* String type need a space for ending '\0' */
        cton_string_setlen(pack, buf->index + 1);

    } else {
        cton_string_setlen(pack, buf->index);

    }


    o_ptr = cton_string_getptr(pack);

    buf_len = CTON_BUFFER_PAGESIZE;
    buf_cnt = cton_array_getlen(buf->arr);

    for (buf_index = 0; buf_index < buf_cnt; buf_index ++) {
        buf_seg = cton_array_get(buf->arr, buf_index);

        buf_ptr = cton_string_getptr(buf_seg);

        if (buf_index == buf_cnt - 1) {
            buf_len = buf->index % CTON_BUFFER_PAGESIZE;
        }

        for (ch_index = 0; ch_index < buf_len; ch_index ++) {
            *o_ptr = buf_ptr[ch_index];
            o_ptr ++;
        }

    }

    if (type == CTON_STRING) {
        *o_ptr = '\0';
    }

    return pack;
}

int cton_util_buffer_putchar(cton_buf *buf, int c)
{
    int array_len;
    cton_obj *str;
    char     *ptr;

    array_len = cton_array_getlen(buf->arr);

    if (buf->index % CTON_BUFFER_PAGESIZE == 0) {
        /* expand buffer first */
        array_len = cton_array_getlen(buf->arr);
        array_len += 1;
        cton_array_setlen(buf->arr, array_len);

        str = cton_object_create(buf->ctx, CTON_STRING);
        cton_string_setlen(str, CTON_BUFFER_PAGESIZE);
        cton_array_set(buf->arr, str, array_len - 1);
    }

    str = cton_array_get(buf->arr, array_len - 1);
    ptr = cton_binary_getptr(str);
    ptr[buf->index % CTON_BUFFER_PAGESIZE] = c;
    buf->index += 1;

    return c;
}

cton_obj *cton_util_readfile(cton_ctx *ctx, const char *path)
{
    cton_obj *data;
    FILE     *fp;
    size_t    len;
    char     *ptr;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = cton_object_create(ctx, CTON_BINARY);
    cton_string_setlen(data, len);

    ptr = cton_string_getptr(data);
    fread(ptr, len, 1, fp);
    fclose(fp);

    return data;
}

int cton_util_writefile(cton_ctx *ctx, cton_obj* obj, const char *path)
{
    FILE     *fp;
    size_t    len;
    cton_type type;
    char     *ptr;

    type = cton_objtype(obj);
    if (type != CTON_STRING && type != CTON_BINARY) {
        return -1;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }

    len = cton_string_getlen(obj);
    ptr = cton_string_getptr(obj);

    if (type == CTON_STRING) {
        len -= 1; /* Dismiss the ending '\0' */
    }

    fwrite(ptr, len, 1, fp);
    fclose(fp);

    return 0;
}

cton_obj *cton_util_linesplit(cton_ctx *ctx, cton_obj *src_obj)
{
    cton_obj *lines;
    cton_obj *line;

    size_t src_len;
    size_t src_index;
    size_t dst_index;
    size_t arr_index;
    size_t line_length;

    uint8_t *src;
    uint8_t *dst;
    uint8_t ch;

    lines = cton_object_create(ctx, CTON_ARRAY);
    cton_array_settype(lines, CTON_STRING);

    arr_index = 0;
    cton_array_setlen(lines, arr_index);

    src = (uint8_t *)cton_string_getptr(src_obj);
    src_len = cton_string_getlen(src_obj);
    src_index = 0;

    while (src_index < src_len) {
        line_length = 0;

        while (src_index + line_length < src_len) {
            /* Get line length */
            ch = src[src_index + line_length];
            if (ch == '\r' || ch == '\n') {
                break;
            }

            line_length += 1;
        }

        line = cton_object_create(ctx, CTON_STRING);
        cton_string_setlen(line, line_length + 1);
        dst = (uint8_t *)cton_string_getptr(line);

        for (dst_index = 0; dst_index < line_length; dst_index ++) {
            dst[dst_index] = src[src_index + dst_index];
        }

        dst[line_length] = '\0';
        cton_array_setlen(lines, arr_index + 1);
        cton_array_set(lines, line, arr_index);

        src_index += line_length;

        if (src[src_index] == '\r') {
            if ((src_index + 1 < src_len) && src[src_index + 1] == '\n') {
                src_index += 1;
            }
        }

        src_index += 1;
        arr_index += 1;
    }

    return lines;
}

cton_obj *cton_util_linewrap(cton_ctx *ctx, cton_obj *src, size_t col, char w)
{
    cton_obj *dst;
    size_t src_len;
    size_t dst_len;

    size_t wrap_cnt;

    size_t src_index;
    size_t dst_index;

    char *s;
    char *d;

    if (cton_objtype(src) != CTON_STRING) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    src_len = cton_string_getlen(src);

    wrap_cnt = src_len / col;

    if (w == '\0') {
        dst_len = src_len + wrap_cnt * 2;
    } else {
        dst_len = src_len + wrap_cnt;
    }

    dst = cton_object_create(ctx, CTON_STRING);
    cton_string_setlen(dst, dst_len);

    s = cton_string_getptr(src);
    d = cton_string_getptr(dst);
    dst_index = 0;

    if (w == '\0') {
        for (src_index = 0; src_index < src_len; src_index ++) {
            d[dst_index] = s[src_index];
            dst_index ++;

            if ((src_index + 1) % col == 0) {
                d[dst_index] = '\r';
                dst_index ++;
                d[dst_index] = '\n';
                dst_index ++;
            }
        }

    } else {
        
        for (src_index = 0; src_index < src_len; src_index ++) {
            d[dst_index] = s[src_index];
            dst_index ++;

            if ((src_index + 1) % col == 0) {
                d[dst_index] = w;
                dst_index ++;
            }
        }
    }

    return dst;
}


/*******************************************************************************
 * CTON util algorithms
 *  Base16
 ******************************************************************************/

cton_obj *cton_util_encode16(cton_ctx *ctx, cton_obj* obj, int option)
{
    static uint8_t basis16_std[] = "0123456789ABCDEF";
    static uint8_t basis16_low[] = "0123456789abcdef";

    cton_obj *dst_obj;

    uint8_t *src;
    uint8_t *dst;
    size_t   len;

    uint8_t *basis16;

    if (option == 1) {
        basis16 = basis16_low;
    } else {
        basis16 = basis16_std;
    }

    dst_obj = cton_object_create(ctx, CTON_STRING);
    cton_string_setlen(dst_obj, cton_string_getlen(obj) * 2 + 1);

    len = cton_string_getlen(obj);
    src = cton_binary_getptr(obj);
    dst = cton_binary_getptr(dst_obj);

    while (len > 0) {
        *dst++ = basis16[((*src) & 0xF0) >> 4];
        *dst++ = basis16[((*src) & 0x0F)];
        len -= 1;
        src ++;
    }

    *dst++ = '\0';

    return dst_obj;
}

/*******************************************************************************
 * CTON Garbage Collection
 ******************************************************************************/

static int
cton_gc_mark_array(cton_ctx *ctx, cton_obj *obj, size_t i, void* r);
static int
cton_gc_mark_hash(cton_ctx *ctx, cton_obj *k, cton_obj *v, size_t i, void* r);

static int cton_gc_collect(cton_ctx *ctx);

int cton_gc(cton_ctx *ctx)
{
    (void) ctx;
    cton_obj *root;
    int cnt;

    cnt = 0;
    root = cton_tree_getroot(ctx);

    if (root != NULL) {
        cton_gc_mark(ctx, root);
    }

    cnt = cton_gc_collect(ctx);

    return cnt;
}

static int
cton_gc_mark_array(cton_ctx *ctx, cton_obj *obj, size_t i, void* r)
{
    (void) i;
    (void) r;

    if (obj != NULL) {
        cton_gc_mark(ctx, obj);
    }

    return 0;
}

static int
cton_gc_mark_hash(cton_ctx *ctx, cton_obj *k, cton_obj *v, size_t i, void* r)
{

    (void) i;
    (void) r;

    cton_gc_mark(ctx, k);
    cton_gc_mark(ctx, v);
    
    return 0;
}


void cton_gc_mark(cton_ctx *ctx, cton_obj *obj)
{
    cton_type type;

    if (obj->ref == 0) {
        type = cton_objtype(obj);

        if (type == CTON_ARRAY) {
            obj->ref = 1;
            cton_array_foreach(obj, NULL, cton_gc_mark_array);
        } else if (type == CTON_HASH) {
            obj->ref = 1;
            cton_hash_foreach(obj, NULL, cton_gc_mark_hash);
        } else {
            obj->ref = 1;
        }
    }
}

static int cton_gc_collect(cton_ctx *ctx)
{
    cton_obj *obj;
    cton_obj *next;
    int cnt;

    cnt = 0;
    obj = ctx->nodes;

    while (obj != NULL) {
        next = obj->next;

        if (obj->ref == 0) {
            cton_object_delete(obj);
            cnt += 1;
        } else {
            obj->ref = 0;
        }

        obj = next;
    }

    return cnt;
}
