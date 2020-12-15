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

static int cton_alloc_debug = 0;

/*
 * Just proxy the call of malloc() and free()
 */
static void *cton_std_malloc(void *pool, size_t size)
{
    extern int cton_alloc_debug;
    (void) pool;

    if (cton_alloc_debug) {
        fprintf(stderr, "Trying to alloc %ld byte(s)\n", size);
    }

    return malloc(size);
}

static void  cton_std_free(void *pool, void *ptr)
{
    (void) pool;
    free(ptr);
}

cton_memhook cton_std_hook = {
    NULL, cton_std_malloc, NULL, cton_std_free, NULL
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
    return ctx->memhook.palloc(ctx->memhook.pool, size);
}

static void cton_free(cton_ctx *ctx, void *ptr)
{
    ctx->memhook.pfree(ctx->memhook.pool, ptr);
}

static void * cton_realloc(cton_ctx *ctx, void *ptr, size_t size_ori, size_t size_new)
{
    void * new_ptr;

    if (size_new <= size_ori) {
        return ptr;
    }

    if (ctx->memhook.prealloc == NULL) {
        
        new_ptr = cton_alloc(ctx, size_new);
        if (new_ptr == NULL) {
            cton_seterr(ctx, CTON_ERROR_EALLOC);
            return NULL;
        }

        memcpy(new_ptr, ptr, size_ori);

        cton_free(ctx, ptr);

    } else {
        new_ptr = ctx->memhook.prealloc(ctx->memhook.pool, ptr, size_new);
        if (new_ptr == NULL) {
            cton_seterr(ctx, CTON_ERROR_EALLOC);
        }
    }

    return new_ptr;
}

static void cton_pdestroy(cton_ctx *ctx)
{
    ctx->memhook.pdestroy(ctx->memhook.pool);
}
