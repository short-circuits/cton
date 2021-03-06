
#include <core/cton_core.h>

/*******************************************************************************
 * CTON util functions
 * 
 ******************************************************************************/


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

int cton_util_writefile(cton_obj* obj, const char *path)
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

        if (src_index == src_len) {
            break;
        }

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

