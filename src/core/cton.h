/*******************************************************************************
 * "THE BEER-WARE LICENSE" (Revision 43):
 * <yeonji@ieee.org> create this project. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a sweety curry rice in return (cuz
 * I cannot drink alcoholic beverages). Yeonji Lee
 *
 ******************************************************************************/

#ifndef _CTON_HEADER_
#define _CTON_HEADER_ 1

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _INTER_LIBCTON_

typedef struct {
    void *      pool;
    void *    (*palloc)(void *pool, size_t size);
    void *    (*prealloc)(void *pool, void *ptr, size_t size);
    void      (*pfree)(void *pool, void *ptr);
    void      (*pdestroy)(void *pool);
} cton_memhook;

extern cton_memhook cton_std_hook;

#else

typedef void cton_memhook;

#endif

/*
 * CTON object types
 */

#ifdef _INTER_LIBCTON_

typedef struct cton_obj_s       cton_obj;
typedef struct cton_string_s    cton_string;
typedef struct cton_array_s     cton_array;
typedef struct cton_hash_item_s cton_hash_item;
typedef struct cton_hash_s      cton_hash;

#else

typedef void cton_obj;
typedef void cton_string;
typedef void cton_array;
typedef void cton_hash_item;
typedef void cton_hash;

#endif

enum cton_type_e {
    CTON_INVALID = 0,
    CTON_OBJECT  = 1,
    CTON_NULL    = 2,
    CTON_BOOL    = 3,
    CTON_BINARY  = 4,
    CTON_STRING  = 5,
    CTON_ARRAY   = 6,
    CTON_HASH    = 7,
    CTON_INT8    = 8,
    CTON_INT16   = 9,
    CTON_INT32   = 10,
    CTON_INT64   = 11,
    CTON_UINT8   = 12,
    CTON_UINT16  = 13,
    CTON_UINT32  = 14,
    CTON_UINT64  = 15,
    CTON_FLOAT8  = 16,
    CTON_FLOAT16 = 17,
    CTON_FLOAT32 = 18,
    CTON_FLOAT64 = 19
};

#define CTON_TYPE_CNT 20

enum cton_bool_e {
    CTON_FALSE = 0,
    CTON_TRUE  = 1
};

#ifdef _INTER_LIBCTON_

struct cton_string_s {
    size_t   len;
    size_t   used;
    uint8_t *ptr;
};

struct cton_array_s {
    size_t       len;
    size_t       used;
    cton_obj   **ptr;
    enum cton_type_e    sub_type;
};

struct cton_hash_item_s {
    cton_obj        *key;
    cton_obj        *value;
    cton_hash_item  *next;
};

struct cton_hash_s {
    size_t          count;
    cton_hash_item *root;
    cton_hash_item *last;
};

struct cton_obj_s {
    uint64_t  magic;  /* Magic number for debug. */

    union {
        enum cton_bool_e   b;
        cton_string str;
        cton_array  arr;
        cton_hash   hash;
        int8_t      i8;
        int16_t     i16;
        int32_t     i32;
        int64_t     i64;
        uint8_t     u8;
        uint16_t    u16;
        uint32_t    u32;
        uint64_t    u64;
        float       f32;
        double      f64;
    } payload;

    /* For object pool */
    struct cton_obj_s *next;
    struct cton_obj_s *prev;

    enum cton_type_e type;   /* CTON type */
    uint8_t ref;
};

#endif

typedef enum   cton_type_e      cton_type;
typedef enum   cton_bool_e      cton_bool;

#define CTON_STRUCT_MAGIC 0x4E4F5443




/*******************************************************************************
 * CTON context types
 * 用来保存内存池结构体，对应的错误指，节点池，以及全局参数。
 * 同一个上下文结构体不保证写线程安全。
 * TODO 是否保留一个内存池的互斥锁接口，通过锁住整个上下文的方式保证线程安全？
 * 
 * 不同上下文结构体之间应保证线程安全。（避免static变量的使用）
 * 是否要求所有函数都使用上下文结构体作为第一个传入参数？
 ******************************************************************************/
#ifdef _INTER_LIBCTON_

typedef struct cton_ctx_s       cton_ctx;

#else

typedef void cton_ctx;

#endif

enum cton_error_e {
    CTON_OK            = 0,
    CTON_ERROR_INVAL,         /* Invalid CTON object */
    CTON_ERROR_ALLOC,         /* Memory allocation failed
                                 (NULL was returnd from alloc) */
    CTON_ERROR_CREATE,        /* Tring to create objcet with invalid type
                                 (Only: cton_object_create() ) */
    CTON_ERROR_TYPE,          /* Type specific method received object
                                 with wrong type */
    CTON_ERROR_INVSUBTYPE,    /* Tring to set array by invalid element type
                                 (Only: cton_array_settype() ) */
    CTON_ERROR_RSTSUBTYPE,    /* Tring to reset array element type
                                 (Only: cton_array_settype() ) */
    CTON_ERROR_IMPLEM,
    CTON_ERROR_OVF,
    CTON_ERROR_REPLACE,
    CTON_ERROR_SUBTYPE,
    CTON_ERROR_INDEX,
    CTON_ERROR_INPUT,
    CTON_ERROR_BROKEN,        /* Data is not complele (more data except) */
    CTON_ERROR_SYNTAX,        /* Syntax error while parsing data */
    CTON_ERROR_UNKNOWN = 127  /* Unknown error */
};

typedef enum cton_error_e       cton_err;

#ifdef _INTER_LIBCTON_

struct cton_ctx_s {
    cton_memhook memhook;

    cton_obj *root; /* 根节点 */

    /* Object pool */
    cton_obj *nodes;
    cton_obj *nodes_last;

    cton_err  err;
};

#endif

/* cton memhook methods */
cton_memhook* cton_memhook_init (void * pool,
    void *    (*palloc)(void *pool, size_t size),
    void *    (*prealloc)(void *pool, void *ptr, size_t size),
    void      (*pfree)(void *pool, void *ptr),
    void      (*pdestroy)(void *pool));

/* cton context methods */
cton_ctx *cton_init(cton_memhook *hook);
int cton_destory(cton_ctx *ctx);
void cton_seterr(cton_ctx *ctx, cton_err err);
cton_err cton_geterr(cton_ctx *ctx);
char * cton_strerr(cton_err err);
#define cton_err_clear(ctx) {cton_seterr((ctx), CTON_OK)}

int cton_gc(cton_ctx *ctx);

/* cton_obj common methods */
cton_obj * cton_object_create(cton_ctx *ctx, cton_type type);
void cton_object_delete(cton_ctx *ctx, cton_obj *obj);
cton_type cton_object_gettype(cton_ctx *ctx, cton_obj *obj);
void * cton_object_getvalue(cton_ctx *ctx, cton_obj *obj);

/* cton bool type specific methods */
int cton_bool_set(cton_ctx *ctx, cton_obj *obj, cton_bool val);
cton_bool cton_bool_get(cton_ctx *ctx, cton_obj *obj);

/* cton string type specific methods */
int cton_string_setlen(cton_ctx *ctx, cton_obj *obj, size_t len);
size_t cton_string_getlen(cton_ctx *ctx, cton_obj *obj);
char * cton_string_getptr(cton_ctx *ctx, cton_obj *obj);
void * cton_binary_getptr(cton_ctx *ctx, cton_obj *obj);

/* cton array type specific methods */
int cton_array_settype(cton_ctx *ctx, cton_obj *arr, cton_type type);
cton_type cton_array_gettype(cton_ctx *ctx, cton_obj *arr);
size_t cton_array_setlen(cton_ctx *ctx, cton_obj *arr, size_t len);
size_t cton_array_getlen(cton_ctx *ctx, cton_obj *arr);
int cton_array_set(cton_ctx *ctx, cton_obj *arr, cton_obj *obj, size_t index);
cton_obj * cton_array_get(cton_ctx *ctx, cton_obj *arr, size_t index);
int cton_array_foreach(cton_ctx *ctx, cton_obj *arr, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, size_t, void*));

/* cton hash type specific methods */
cton_obj * cton_hash_set(cton_ctx *ctx, cton_obj *h, cton_obj *k, cton_obj *v);
cton_obj * cton_hash_get(cton_ctx *ctx, cton_obj *h, cton_obj *k);
cton_obj * cton_hash_get_s(cton_ctx *ctx, cton_obj *h, const char *ks);
size_t cton_hash_getlen(cton_ctx *ctx, cton_obj *h);
int cton_hash_foreach(cton_ctx *ctx, cton_obj *hash, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, cton_obj *, size_t, void*));

/* cton numeric types specific methods */
int64_t cton_numeric_setint(cton_ctx *ctx, cton_obj *obj, int64_t val);
int64_t cton_numeric_getint(cton_ctx *ctx, cton_obj *obj);
uint64_t cton_numeric_setuint(cton_ctx *ctx, cton_obj *obj, uint64_t val);
uint64_t cton_numeric_getuint(cton_ctx *ctx, cton_obj *obj);
double cton_numeric_setfloat(cton_ctx *ctx, cton_obj *obj, double val);
double cton_numeric_getfloat(cton_ctx *ctx, cton_obj *obj);

/* cton_util.c */
cton_obj * cton_util_create_str(cton_ctx *ctx,
    const char *str, char end, char quote);
cton_obj * cton_util_strcstr(cton_ctx *ctx, const char *cstr);

#ifdef _INTER_LIBCTON_
typedef struct cton_buf_s cton_buf;
struct cton_buf_s {
    cton_ctx *ctx;
    cton_obj *arr;
    cton_obj *container;
    size_t   index;
};
#else
typedef void cton_buf;
#endif

cton_buf *cton_util_buffer_create(cton_ctx *ctx);
void cton_util_buffer_destroy(cton_buf *buf);
size_t cton_util_buffer_getlen(cton_buf *buf);
cton_obj *cton_util_buffer_pack(cton_buf *buf, cton_type type);
int cton_util_buffer_putchar(cton_buf *buf, int c);

int cton_util_strcmp(cton_obj *s1, cton_obj *s2);
cton_obj *cton_util_readfile(cton_ctx *ctx, const char *path);
int cton_util_writefile(cton_ctx *ctx, cton_obj* obj, const char *path);
cton_obj *cton_util_linesplit(cton_ctx *ctx, cton_obj *src_obj);
cton_obj *cton_util_linewrap(cton_ctx *ctx, cton_obj *src, size_t col, char w);
cton_obj *cton_util_encode16(cton_ctx *ctx, cton_obj* obj, int option);


/* cton tree methods */
int cton_tree_setroot(cton_ctx *ctx, cton_obj *obj);
cton_obj *cton_tree_getroot(cton_ctx *ctx);
cton_obj *cton_tree_get_by_path(cton_ctx *ctx, cton_obj *path);


cton_obj * cton_stringify(cton_ctx *ctx, cton_obj *obj);
cton_obj * cton_parse(cton_ctx *ctx, cton_obj *ton);
cton_obj * cton_serialize(cton_ctx *ctx, cton_obj *obj);
cton_obj * cton_deserialize(cton_ctx *ctx, cton_obj *tbon);

/* CTON_MODULE_FUNCS */

#endif /* _CTON_HEADER_ */
