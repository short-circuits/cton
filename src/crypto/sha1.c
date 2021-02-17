/*******************************************************************************
 * CTON util algorithms
 *  SHA1
 ******************************************************************************/

#include <cton.h>

static void * cton_llib_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

static void * cton_llib_memset(void *b, int c, size_t len)
{
    return memset(b, c, len);
}

typedef struct {
    uint64_t  bytes;
    uint32_t  a, b, c, d, e, f;
    uint8_t   buffer[64];
} cton_sha1_ctx;

static void
cton_digest_sha1_init(cton_sha1_ctx *ctx);
static void
cton_digest_sha1_update(cton_sha1_ctx *ctx, const void *data, size_t size);
static void
cton_digest_sha1_final(cton_sha1_ctx *ctx, uint8_t *result);
static const uint8_t *
cton_digest_sha1_body(cton_sha1_ctx *ctx, const uint8_t *data, size_t size);

cton_obj * cton_digest_sha1(cton_ctx *ctx, cton_obj *obj)
{
    cton_obj *sha1;
    cton_obj *sha1ctx;

    cton_sha1_ctx *sha1ctx_ptr;

    if (cton_object_gettype(obj) != CTON_STRING &&
        cton_object_gettype(obj) != CTON_BINARY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    sha1 = cton_object_create(ctx, CTON_BINARY);
    cton_string_setlen(ctx, sha1, 20);

    sha1ctx = cton_object_create(ctx, CTON_BINARY);
    cton_string_setlen(ctx, sha1ctx, sizeof(cton_sha1_ctx));
    sha1ctx_ptr = cton_binary_getptr(ctx, sha1ctx);

    cton_digest_sha1_init(sha1ctx_ptr);
    cton_digest_sha1_update(sha1ctx_ptr,
        cton_string_getptr(ctx, obj), cton_string_getlen(ctx, obj));
    cton_digest_sha1_final(sha1ctx_ptr, cton_binary_getptr(ctx, sha1));

    cton_object_delete(sha1ctx);

    return sha1;
}

static void 
cton_digest_sha1_init(cton_sha1_ctx *ctx)
{
    ctx->a = 0x67452301;
    ctx->b = 0xefcdab89;
    ctx->c = 0x98badcfe;
    ctx->d = 0x10325476;
    ctx->e = 0xc3d2e1f0;

    ctx->bytes = 0;
}

static void
cton_digest_sha1_update(cton_sha1_ctx *ctx, const void *data, size_t size)
{
    size_t  used, free;

    used = (size_t) (ctx->bytes & 0x3f);
    ctx->bytes += size;

    if (used) {
        free = 64 - used;

        if (size < free) {
            cton_llib_memcpy(&ctx->buffer[used], data, size);
            return;
        }

        cton_llib_memcpy(&ctx->buffer[used], data, free);
        data = (uint8_t *) data + free;
        size -= free;
        (void) cton_digest_sha1_body(ctx, ctx->buffer, 64);
    }

    if (size >= 64) {
        data = cton_digest_sha1_body(ctx, data, size & ~(size_t) 0x3f);
        size &= 0x3f;
    }

    cton_llib_memcpy(ctx->buffer, data, size);
}

static void
cton_digest_sha1_final(cton_sha1_ctx *ctx, uint8_t *result)
{
    size_t  used, free;

    used = (size_t) (ctx->bytes & 0x3f);

    ctx->buffer[used++] = 0x80;

    free = 64 - used;

    if (free < 8) {
        cton_llib_memset(&ctx->buffer[used], 0, free);
        (void) cton_digest_sha1_body(ctx, ctx->buffer, 64);
        used = 0;
        free = 64;
    }

    cton_llib_memset(&ctx->buffer[used], 0, free - 8);

    ctx->bytes <<= 3;
    ctx->buffer[56] = (uint8_t) (ctx->bytes >> 56);
    ctx->buffer[57] = (uint8_t) (ctx->bytes >> 48);
    ctx->buffer[58] = (uint8_t) (ctx->bytes >> 40);
    ctx->buffer[59] = (uint8_t) (ctx->bytes >> 32);
    ctx->buffer[60] = (uint8_t) (ctx->bytes >> 24);
    ctx->buffer[61] = (uint8_t) (ctx->bytes >> 16);
    ctx->buffer[62] = (uint8_t) (ctx->bytes >> 8);
    ctx->buffer[63] = (uint8_t) ctx->bytes;

    (void) cton_digest_sha1_body(ctx, ctx->buffer, 64);

    result[0] = (uint8_t) (ctx->a >> 24);
    result[1] = (uint8_t) (ctx->a >> 16);
    result[2] = (uint8_t) (ctx->a >> 8);
    result[3] = (uint8_t) ctx->a;
    result[4] = (uint8_t) (ctx->b >> 24);
    result[5] = (uint8_t) (ctx->b >> 16);
    result[6] = (uint8_t) (ctx->b >> 8);
    result[7] = (uint8_t) ctx->b;
    result[8] = (uint8_t) (ctx->c >> 24);
    result[9] = (uint8_t) (ctx->c >> 16);
    result[10] = (uint8_t) (ctx->c >> 8);
    result[11] = (uint8_t) ctx->c;
    result[12] = (uint8_t) (ctx->d >> 24);
    result[13] = (uint8_t) (ctx->d >> 16);
    result[14] = (uint8_t) (ctx->d >> 8);
    result[15] = (uint8_t) ctx->d;
    result[16] = (uint8_t) (ctx->e >> 24);
    result[17] = (uint8_t) (ctx->e >> 16);
    result[18] = (uint8_t) (ctx->e >> 8);
    result[19] = (uint8_t) ctx->e;

    cton_llib_memset(ctx, 0, sizeof(*ctx));
}

/*
 * Helper functions.
 */

#define CTON_SHA1_ROTATE(bits, word)                                          \
    (((word) << (bits)) | ((word) >> (32 - (bits))))

#define CTON_SHA1_F1(b, c, d)  (((b) & (c)) | ((~(b)) & (d)))
#define CTON_SHA1_F2(b, c, d)  ((b) ^ (c) ^ (d))
#define CTON_SHA1_F3(b, c, d)  (((b) & (c)) | ((b) & (d)) | ((c) & (d)))

#define CTON_SHA1_STEP(f, a, b, c, d, e, w, t)                                \
    temp = CTON_SHA1_ROTATE(5, (a)) + f((b), (c), (d)) + (e) + (w) + (t);     \
    (e) = (d);                                                                \
    (d) = (c);                                                                \
    (c) = CTON_SHA1_ROTATE(30, (b));                                          \
    (b) = (a);                                                                \
    (a) = temp;

#define CTON_SHA1_GET(n)                                                      \
    ((uint32_t) p[n * 4 + 3] |                                                \
    ((uint32_t) p[n * 4 + 2] << 8) |                                          \
    ((uint32_t) p[n * 4 + 1] << 16) |                                         \
    ((uint32_t) p[n * 4] << 24))

static const uint8_t *
cton_digest_sha1_body(cton_sha1_ctx *ctx, const uint8_t *data, size_t size)
{
    uint32_t       a, b, c, d, e, temp;
    uint32_t       saved_a, saved_b, saved_c, saved_d, saved_e;
    uint32_t       words[80];
    uint64_t       i;
    const uint8_t  *p;

    p = data;

    a = ctx->a;
    b = ctx->b;
    c = ctx->c;
    d = ctx->d;
    e = ctx->e;

    do {
        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;
        saved_e = e;

        /* Load data block into the words array */

        for (i = 0; i < 16; i++) {
            words[i] = CTON_SHA1_GET(i);
        }

        for (i = 16; i < 80; i++) {
            words[i] = CTON_SHA1_ROTATE(1, words[i - 3] ^ words[i - 8]
                ^ words[i - 14] ^ words[i - 16]);
        }

        /* Transformations */

        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[0],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[1],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[2],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[3],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[4],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[5],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[6],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[7],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[8],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[9],  0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[10], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[11], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[12], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[13], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[14], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[15], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[16], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[17], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[18], 0x5a827999);
        CTON_SHA1_STEP(CTON_SHA1_F1, a, b, c, d, e, words[19], 0x5a827999);

        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[20], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[21], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[22], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[23], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[24], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[25], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[26], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[27], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[28], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[29], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[30], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[31], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[32], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[33], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[34], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[35], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[36], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[37], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[38], 0x6ed9eba1);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[39], 0x6ed9eba1);

        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[40], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[41], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[42], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[43], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[44], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[45], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[46], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[47], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[48], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[49], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[50], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[51], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[52], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[53], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[54], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[55], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[56], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[57], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[58], 0x8f1bbcdc);
        CTON_SHA1_STEP(CTON_SHA1_F3, a, b, c, d, e, words[59], 0x8f1bbcdc);

        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[60], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[61], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[62], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[63], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[64], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[65], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[66], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[67], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[68], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[69], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[70], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[71], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[72], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[73], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[74], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[75], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[76], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[77], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[78], 0xca62c1d6);
        CTON_SHA1_STEP(CTON_SHA1_F2, a, b, c, d, e, words[79], 0xca62c1d6);

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;
        e += saved_e;

        p += 64;

    } while (size -= 64);

    ctx->a = a;
    ctx->b = b;
    ctx->c = c;
    ctx->d = d;
    ctx->e = e;

    return p;
}
