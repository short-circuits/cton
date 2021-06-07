
#include <core/cton_core.h>

#define CTON_BUFFER_PAGESIZE 4096

cton_buf *cton_buffer_create(cton_ctx *ctx)
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
    (void) ctx;
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

size_t cton_buffer_getlen(cton_buf *buf)
{
    return buf->index;
}

cton_obj *cton_buffer_pack(cton_buf *buf, cton_type type)
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
        buf_seg = *(cton_obj **)cton_array_get(buf->arr, buf_index);

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

int cton_buffer_putchar(cton_buf *buf, int c)
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

    str = *(cton_obj **)cton_array_get(buf->arr, array_len - 1);
    ptr = cton_binary_getptr(str);
    ptr[buf->index % CTON_BUFFER_PAGESIZE] = c;
    buf->index += 1;

    return c;
}

int cton_util_buffer_puts(cton_buf *buf, const char *s)
{

    while (*s != '\0') {
        cton_buffer_putchar(buf, *s);
        s += 1;
    }

    return 0;
}