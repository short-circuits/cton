/*******************************************************************************
 * CTON util algorithms
 *  Base64
 ******************************************************************************/

#include <cton.h>

static cton_obj *cton_base64_encode_internal(cton_ctx *ctx,
    cton_obj* src, const uint8_t *basis, char padding, int wrap)
{
    size_t src_len;
    size_t dst_len;

    cton_obj *dst;
    cton_obj *wrapd;

    uint8_t *s;
    uint8_t *d;

    src_len = cton_string_getlen(ctx, src);
    dst_len = (src_len / 3) * 4;

    /* Get length with padding character */
    if (src_len % 3 > 0) {
        if (padding != 0) {
            dst_len += 4;
        } else {
            dst_len += (1 + src_len % 3);
        }
    }

    dst = cton_object_create(ctx, CTON_STRING);
    if (cton_geterr(ctx) != CTON_OK) {
        return NULL;
    }

    cton_string_setlen(ctx, dst, dst_len + 1);
    if (cton_geterr(ctx) != CTON_OK) {
        cton_object_delete(dst);
        return NULL;
    }


    s = (uint8_t *)cton_string_getptr(ctx, src);
    d = (uint8_t *)cton_string_getptr(ctx, dst);

    while (src_len > 3) {

        *d++ = basis[(s[0] >> 2) & 0x3f];
        *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
        *d++ = basis[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
        *d++ = basis[s[2] & 0x3f];

        s += 3;
        src_len -= 3;
    }

    if (src_len) {
        *d++ = basis[(s[0] >> 2) & 0x3f];

        if (src_len == 1) {
            *d++ = basis[(s[0] & 3) << 4];
            if (padding) {
                *d++ = padding;
            }

        } else { /* src_len == 2 */
            *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
            *d++ = basis[(s[1] & 0x0f) << 2];
        }

        if (padding) {
            *d++ = padding;
        }
    }

    d[dst_len] = '\0';

    if (wrap != 0) {
        wrapd = cton_util_linewrap(ctx, dst, wrap, 0);
        cton_object_delete(dst);
    } else {
        wrapd = dst;
    }

    return wrapd;
}

cton_obj *cton_base64_encode(cton_ctx *ctx, cton_obj* obj, cton_base64_std std)
{
    static uint8_t basis64_std[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static uint8_t basis64_url[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    static uint8_t basis64_imap[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

    uint8_t *basis64;
    char padding;
    int wrap;

    basis64 = basis64_std;
    padding = '=';
    wrap    = 0;

    switch (std) {

        case CTON_BASE64_RFC1421:
            wrap = 64;
            break;

        case CTON_BASE64_RFC2045:
        case CTON_BASE64_RFC4880:
            wrap = 76;
            break;

        case CTON_BASE64_RFC3501:
            basis64 = basis64_imap;
        case CTON_BASE64_RFC2152:
            padding = 0;
            break;

        case CTON_BASE64URL:
        case CTON_BASE64_RFC4868S5:
            basis64 = basis64_url;
        case CTON_BASE64:
        case CTON_BASE64_RFC3548:
        case CTON_BASE64_RFC4868S4:
        default:
            /* Do nothing */;
    }

    return cton_base64_encode_internal(ctx, obj, basis64, padding, wrap);
}

cton_obj * cton_base64_decode(cton_ctx *ctx, cton_obj* obj)
{
    uint8_t * s;
    uint8_t * d;
    uint8_t p_buf[4];
    size_t src_len;
    size_t dst_len;
    size_t code_len;
    size_t src_index;
    size_t dst_index;
    size_t code_index;

    cton_obj *dst;

    static int8_t basis64[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

    /*  SP   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, 63, 62, -1, 63,

    /*   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,

    /*   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
        -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,

    /*   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63,

    /*   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
        -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,

    /*   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~   DEL */
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,

        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };

    if (cton_object_gettype(obj) != CTON_STRING) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    s = (uint8_t *)cton_string_getptr(ctx, obj);
    src_len = cton_string_getlen(ctx, obj);
    code_len = 0;

    for (src_index = 0; src_index < src_len; src_index ++) {
        if (s[src_index] == '=') {
            break;
        }

        if (basis64[s[src_index]] != -1) {
            code_len += 1;
        }
    }

    if (code_len % 4 == 1) {
        cton_seterr(ctx, CTON_ERROR_INPUT);
        return NULL;
    }

    dst_len = (code_len / 4) * 3;

    if (code_len % 4 > 0) {
        dst_len += code_len % 4 - 1;
    }

    dst = cton_object_create(ctx, CTON_BINARY);
    cton_string_setlen(ctx, dst, dst_len);
    d = cton_binary_getptr(ctx, dst);

    dst_index = 0;
    code_index = 0;

    for (src_index = 0; src_index < src_len; src_index ++) {
        if (s[src_index] == '=') {
            break;
        }

        if (basis64[s[src_index]] == -1) {
            continue;
        }

        p_buf[code_index] = s[src_index];
        code_index ++;

        if (code_index >= 4) {
            code_index = 0;
            d[dst_index]     = (basis64[p_buf[0]] << 2 | basis64[p_buf[1]] >> 4);
            d[dst_index + 1] = (basis64[p_buf[1]] << 4 | basis64[p_buf[2]] >> 2);
            d[dst_index + 2] = (basis64[p_buf[2]] << 6 | basis64[p_buf[3]]);

            dst_index += 3;
        }

    }

    if (code_index >= 1) {
        d[dst_index] = (basis64[p_buf[0]] << 2 | basis64[p_buf[1]] >> 4);
    }

    if (code_index >= 2) {
        d[dst_index + 1] = (basis64[p_buf[1]] << 4 | basis64[p_buf[2]] >> 2);
    }

    return dst;
}
