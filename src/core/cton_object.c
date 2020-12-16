/*******************************************************************************
 * CTON Object methods
 ******************************************************************************/

typedef struct {
    cton_type   type;
    void      (*init)(cton_ctx *ctx, cton_obj *obj);
    void      (*destroy)(cton_ctx *ctx, cton_obj *obj);
    void *    (*getptr)(cton_ctx *ctx, cton_obj *obj);
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
        CTON_BINARY, cton_string_init, NULL,
        (void *(*)(cton_ctx *, cton_obj *))cton_str_getptr
    },{
        CTON_STRING, cton_string_init, NULL,
        (void *(*)(cton_ctx *, cton_obj *))cton_str_getptr
    },{
        CTON_ARRAY, cton_array_init, NULL, cton_array_getptr
    },{
        CTON_HASH, NULL, NULL, NULL
    },{
#if 0
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
#else
        CTON_INT8,  NULL, NULL, NULL
    },{
        CTON_INT16, NULL, NULL, NULL
    },{
        CTON_INT32, NULL, NULL, NULL
    },{
        CTON_INT64, NULL, NULL, NULL
    },{
        CTON_UINT8,  NULL, NULL, NULL
    },{
        CTON_UINT16, NULL, NULL, NULL
    },{
        CTON_UINT32, NULL, NULL, NULL
    },{
        CTON_UINT64, NULL, NULL, NULL
    },{
        CTON_FLOAT8,  NULL, NULL, NULL
    },{
        CTON_FLOAT16, NULL, NULL, NULL
    },{
        CTON_FLOAT32, NULL, NULL, NULL
    },{
        CTON_FLOAT64, NULL, NULL, NULL
#endif
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

    obj = cton_alloc(ctx, sizeof(cton_obj));

    cton_util_memset(obj, 0, sizeof(cton_obj));

    obj->magic = CTON_STRUCT_MAGIC;
    obj->type  = type;
    obj->prev  = NULL;
    obj->next  = NULL;

    /* Insert object into object pool linked list */
    if (ctx->nodes == NULL) {
        ctx->nodes = obj;

    } else {
        obj->prev = ctx->nodes_last;
        ctx->nodes_last->next = obj;

    }

    ctx->nodes_last = obj;

    if (cton_class_hook[type].init != NULL) {
        cton_class_hook[type].init(ctx, obj);
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
void cton_object_delete(cton_ctx *ctx, cton_obj *obj);

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
cton_type cton_object_gettype(cton_ctx *ctx, cton_obj *obj)
{
    if (obj->type >= CTON_TYPE_CNT) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
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
void * cton_object_getvalue(cton_ctx *ctx, cton_obj *obj)
{
    extern cton_class_hook_s cton_class_hook[CTON_TYPE_CNT];

    if (obj->type >= CTON_TYPE_CNT || obj->type == CTON_INVALID) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    if (obj->type == CTON_NULL) {
        return NULL;
    }

    if (cton_class_hook[obj->type].getptr == NULL) {
        return &obj->payload;
    }

    return cton_class_hook[obj->type].getptr(ctx, obj);
}
