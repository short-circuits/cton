/*******************************************************************************
 * CTON Garbage Collection
 ******************************************************************************/

#include <core/cton_core.h>

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
        cton_gc_mark(root);
    }

    cnt = cton_gc_collect(ctx);

    return cnt;
}

static int
cton_gc_mark_array(cton_ctx *ctx, cton_obj *obj, size_t i, void* r)
{
    (void) ctx;
    (void) i;
    (void) r;

    if (obj != NULL) {
        cton_gc_mark(obj);
    }

    return 0;
}

static int
cton_gc_mark_hash(cton_ctx *ctx, cton_obj *k, cton_obj *v, size_t i, void* r)
{
    (void) ctx;
    (void) i;
    (void) r;

    cton_gc_mark(k);
    cton_gc_mark(v);
    
    return 0;
}


void cton_gc_mark(cton_obj *obj)
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
