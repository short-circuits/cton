
#include "cton.h"

/*
 * Init context structure.
 * Setup memory manage hook by parameter;
 * Default hook will be setup if pass the NULL value.
 * the cton_ctx structure will also allocated by memory hook.
 * This function won't initlize the root object.
 *
 * *This function is not thread-safe*
 */
cton_ctx *cton_init(cton_memhook *hook)
{
    cton_ctx *ctx;
    extern cton_memhook cton_std_hook;

    if (hook == NULL) {
        hook = &cton_std_hook;
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

/*
 * Destory context structure.
 * This function will check the memory hook.
 * If destroy hook is not NULL, it will call destroy hook directly.
 * If destroy hook is NULL, but free hook is not NULL, it will call free hook
 * to free all of the sub-object recurslly.
 * If both destroy and free hook is NULL, this function will do nothing.
 */
int cton_destory(cton_ctx *ctx)
{
    cton_pdestroy(ctx);
    return 0;
}

void cton_seterr(cton_ctx *ctx, cton_err err)
{
    ctx->err = err;
}

/*
 * Get last error for specific context
 */
cton_err cton_geterr(cton_ctx *ctx)
{
    return ctx->err;
}

/*
 * Get error string for spedific error code.
 */
char * cton_strerr(cton_err err);

int cton_tree_setroot(cton_ctx *ctx, cton_obj *obj)
{
    cton_type type;

    type = cton_object_gettype(ctx, obj);

    if (cton_geterr(ctx) != CTON_OK) {
        return -1;
    }

    if (type != CTON_HASH && type != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return -1;
    }

    if (cton_tree_getroot(ctx) != NULL) {
        cton_seterr(ctx, CTON_ERROR_REPLACE);

        ctx->root = obj;
        return -2;
    }

    ctx->root = obj;
    return 0;
}

cton_obj *cton_tree_getroot(cton_ctx *ctx)
{
    return ctx->root;
}

cton_obj *cton_tree_get_by_path(cton_ctx *ctx, cton_obj *path);

