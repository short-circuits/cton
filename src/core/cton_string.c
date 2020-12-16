/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 * String
 ******************************************************************************/

#define cton_object_confirm_str(ctx, obj, ret) \
    if (cton_object_gettype(ctx, obj) != CTON_STRING && \
        cton_object_gettype(ctx, obj) != CTON_BINARY) { \
        cton_seterr(ctx, CTON_ERROR_TYPE);         \
        return ret;                                \
    }

static void cton_string_init(cton_ctx *ctx, cton_obj *str)
{
    (void) ctx;
    str->payload.str.ptr  = NULL;
    str->payload.str.len  = 0;
    str->payload.str.used = 0;
}

/*
 * cton_str_getptr()
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
uint8_t * cton_str_getptr(cton_ctx *ctx, cton_obj *obj)
{
    cton_object_confirm_str(ctx, obj, NULL);

    return obj->payload.str.ptr;
}

void * cton_binary_getptr(cton_ctx *ctx, cton_obj *obj)
{
    cton_object_confirm_str(ctx, obj, NULL);

    return obj->payload.str.ptr;
}

/*
 * cton_str_getptr()
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
size_t cton_str_getlen(cton_ctx *ctx, cton_obj *obj)
{
    cton_object_confirm_str(ctx, obj, 0);

    return obj->payload.str.used;
}

int cton_str_setlen(cton_ctx *ctx, cton_obj *obj, size_t len)
{
    size_t aligned;
    void * new_ptr;

    cton_object_confirm_str(ctx, obj, 0);

    if (obj->payload.str.len == 0) {
        obj->payload.str.ptr = cton_alloc(ctx, len);
        obj->payload.str.len = len;
        obj->payload.str.used = len;

    } else if (obj->payload.str.len >= len) {
        obj->payload.str.used = len;

    } else {
        aligned = cton_util_align(len, 4096);
        new_ptr = cton_realloc(ctx, \
            obj->payload.str.ptr, obj->payload.str.len, aligned);
        if (new_ptr != NULL) {
            obj->payload.str.ptr  = new_ptr;
            obj->payload.str.len  = aligned;
            obj->payload.str.used = len;
        }
    }

    return len;
}

cton_obj * cton_str_fromcstr(cton_ctx *ctx,
    const char *str, char end, char quote)
{
    cton_obj *obj;
    uint8_t  *ptr;
    size_t index;

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

    cton_str_setlen(ctx, obj, index + 1);

    ptr = cton_str_getptr(ctx, obj);

    cton_util_memcpy(ptr, str, index);

    ptr[index] = 0;

    return obj;
}

cton_obj * cton_str_new_cstr(cton_ctx *ctx, const char *cstr)
{
    return cton_str_fromcstr(ctx, cstr, '\0', '\\');
}
