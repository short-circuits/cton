# CTON coreAPI

- [CTONコンテキスト](./context.md)

int cton_destory(cton_ctx *ctx);
void cton_seterr(cton_ctx *ctx, cton_err err);
cton_err cton_geterr(cton_ctx *ctx);
char * cton_strerr(cton_err err);
#define cton_err_clear(ctx) {cton_seterr((ctx), CTON_OK)}


int cton_gc(cton_ctx *ctx);
void cton_gc_mark(cton_obj *obj);

/* cton_obj common methods */
cton_obj * cton_object_create(cton_ctx *ctx, cton_type type);
void cton_object_delete(cton_obj *obj);
cton_type cton_object_gettype(cton_obj *obj);
void * cton_object_getvalue(cton_obj *obj);
cton_ctx *cton_object_getctx(cton_obj *obj);

/* cton bool type specific methods */
int cton_bool_set(cton_obj *obj, cton_bool val);
cton_bool cton_bool_get(cton_obj *obj);

/* cton string type specific methods */
int cton_string_setlen(cton_obj *obj, size_t len);
size_t cton_string_getlen(cton_obj *obj);
char * cton_string_getptr(cton_obj *obj);

int cton_binary_setlen(cton_obj *obj, size_t len);
size_t cton_binary_getlen(cton_obj *obj);
void * cton_binary_getptr(cton_obj *obj);
cton_obj * cton_string_create(cton_ctx *ctx, size_t len, const char *str);
#define cton_string(ctx, str) (cton_string_create((ctx), sizeof(str), (str)))

/* cton array type specific methods */
int cton_array_settype(cton_obj *arr, cton_type type);
cton_type cton_array_gettype(cton_obj *arr);
size_t cton_array_setlen(cton_obj *arr, size_t len);
size_t cton_array_getlen(cton_obj *arr);
int cton_array_set(cton_obj *arr, cton_obj *obj, size_t index);
cton_obj * cton_array_get(cton_obj *arr, size_t index);
int cton_array_foreach(cton_obj *arr, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, size_t, void*));

/* cton hash type specific methods */
cton_obj * cton_hash_set(cton_obj *h, cton_obj *k, cton_obj *v);
cton_obj * cton_hash_get(cton_obj *h, cton_obj *k);
cton_obj * cton_hash_get_s(cton_obj *h, const char *ks);
size_t cton_hash_getlen(cton_obj *h);
int cton_hash_foreach(cton_obj *hash, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, cton_obj *, size_t, void*));

/* cton numeric types specific methods */
int64_t cton_numeric_setint(cton_obj *obj, int64_t val);
int64_t cton_numeric_getint(cton_obj *obj);
uint64_t cton_numeric_setuint(cton_obj *obj, uint64_t val);
uint64_t cton_numeric_getuint(cton_obj *obj);
double cton_numeric_setfloat(cton_obj *obj, double val);
double cton_numeric_getfloat(cton_obj *obj);



/* cton tree methods */
int cton_tree_setroot(cton_ctx *ctx, cton_obj *obj);
cton_obj *cton_tree_getroot(cton_ctx *ctx);
cton_obj *cton_tree_get_by_path(cton_ctx *ctx, cton_obj *path);


cton_obj * cton_stringify(cton_ctx *ctx, cton_obj *obj);
cton_obj * cton_parse(cton_ctx *ctx, cton_obj *ton);
cton_obj * cton_serialize(cton_ctx *ctx, cton_obj *obj);
cton_obj * cton_deserialize(cton_ctx *ctx, cton_obj *tbon);

/* CTON_MODULE_FUNCS */


cton_obj *cton_digest_sha1(cton_ctx *ctx, cton_obj *obj);

typedef enum {
    CTON_BASE64 = 0,
    CTON_BASE64URL,
    CTON_BASE64_RFC1421,
    CTON_BASE64_RFC2045,
    CTON_BASE64_RFC2152,
    CTON_BASE64_RFC3501,
    CTON_BASE64_RFC3548,
    CTON_BASE64_RFC4868S4,
    CTON_BASE64_RFC4868S5,
    CTON_BASE64_RFC4880
} cton_base64_std;

cton_obj *cton_base64_encode(cton_ctx *ctx, cton_obj* obj, cton_base64_std std);
cton_obj *cton_base64_decode(cton_ctx *ctx, cton_obj* obj);

cton_obj * cton_bmp_parse(cton_ctx *ctx, cton_obj *bmp);
cton_obj * cton_bmp_serialize(cton_ctx *ctx, cton_obj *obj);

cton_obj * cton_json_parse(cton_ctx *ctx, cton_obj *json);
cton_obj * cton_json_stringify(cton_ctx *ctx, cton_obj *obj);



cton_memhook* cton_memhook_init (void * pool,
    void *    (*palloc)(void *pool, size_t size),
    void *    (*prealloc)(void *pool, void *ptr, size_t size),
    void      (*pfree)(void *pool, void *ptr),
    void      (*pdestroy)(void *pool));

/* cton context methods */
