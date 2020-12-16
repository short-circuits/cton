#include <limits.h>
/*******************************************************************************
 * init
 ******************************************************************************/

void cton_int8_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.i8 = 0;
}

void cton_int16_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.i16 = 0;
}

void cton_int32_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.i32 = 0;
}

void cton_int64_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.i64 = 0;
}

void cton_uint8_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.u8 = 0;
}

void cton_uint16_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.u16 = 0;
}

void cton_uint32_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.u32 = 0;
}

void cton_uint64_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.u64 = 0;
}

void cton_float8_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) obj;
    cton_seterr(ctx, CTON_ERROR_IMPLEM);
}

void cton_float16_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) obj;
    cton_seterr(ctx, CTON_ERROR_IMPLEM);
}

void cton_float32_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.f32 = 0.0;
}

void cton_float64_init(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    obj->payload.f64 = 0.0;
}

/*******************************************************************************
 * getptr
 ******************************************************************************/

void * cton_int8_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.i8;
}

void * cton_int16_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.i16;
}

void * cton_int32_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.i32;
}

void * cton_int64_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.i64;
}

void * cton_uint8_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.u8;
}

void * cton_uint16_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.u16;
}

void * cton_uint32_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.u32;
}

void * cton_uint64_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.u64;
}

void * cton_float8_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) obj;
    cton_seterr(ctx, CTON_ERROR_IMPLEM);
    return NULL;
}

void * cton_float16_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) obj;
    cton_seterr(ctx, CTON_ERROR_IMPLEM);
    return NULL;
}

void * cton_float32_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.f32;
}

void * cton_float64_getptr(cton_ctx *ctx, cton_obj *obj)
{
    (void) ctx;
    return &obj->payload.f64;
}

int64_t cton_numeric_getint(cton_ctx *ctx, cton_obj *obj)
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
        cton_seterr(ctx, CTON_ERROR_TYPE);
    }

    return val;
}

int64_t cton_numeric_setint(cton_ctx *ctx, cton_obj *obj, int64_t val)
{
    if (obj->type == CTON_INT8) {
        if (val > INT8_MAX || val < INT8_MIN) {
            cton_seterr(ctx, CTON_ERROR_OVF);
        }
        obj->payload.i8 = (int8_t)val;

    } else if (obj->type == CTON_INT16) {
        if (val > INT16_MAX || val < INT16_MIN) {
            cton_seterr(ctx, CTON_ERROR_OVF);
        }
        obj->payload.i16 = (int16_t)val;

    } else if (obj->type == CTON_INT32) {
        if (val > INT32_MAX || val < INT32_MIN) {
            cton_seterr(ctx, CTON_ERROR_OVF);
        }
        obj->payload.i32 = (int32_t)val;

    } else if (obj->type == CTON_INT64) {
        obj->payload.i64 = (int64_t)val;

    } else {
        cton_seterr(ctx, CTON_ERROR_TYPE);
    }

    return val;
}

uint64_t cton_numeric_getuint(cton_ctx *ctx, cton_obj *obj)
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
        cton_seterr(ctx, CTON_ERROR_TYPE);
    }

    return val;
}

double cton_numeric_getfloat(cton_ctx *ctx, cton_obj *obj)
{
    double val = 0.0;

    if (obj->type == CTON_FLOAT8) {
        cton_seterr(ctx, CTON_ERROR_IMPLEM);

    } else if (obj->type == CTON_FLOAT16) {
        cton_seterr(ctx, CTON_ERROR_IMPLEM);

    } else if (obj->type == CTON_FLOAT32) {
        val = obj->payload.f32;

    } else if (obj->type == CTON_FLOAT64) {
        val = obj->payload.f64;

    } else {
        cton_seterr(ctx, CTON_ERROR_TYPE);
    }

    return val;
}

int cton_bool_set(cton_ctx *ctx, cton_obj *obj, cton_bool val)
{
    if (cton_object_gettype(ctx, obj) != CTON_BOOL) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return -1;
    }

    obj->payload.b = val;
    return 0;
}

cton_bool cton_bool_get(cton_ctx *ctx, cton_obj *obj)
{
    if (cton_object_gettype(ctx, obj) != CTON_BOOL) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return -1;
    }

    return obj->payload.b;
}
