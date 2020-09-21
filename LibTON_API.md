# libTON API introduction

## APIs

### Types

CTON_Invalid
CTON_NULL
CTON_TRUE
CTON_FALSE
CTON_BINARY
CTON_STRING
CTON_ARRAY
CTON_HASH
CTON_INT8
CTON_INT16
CTON_INT32
CTON_INT64
CTON_UINT8
CTON_UINT16
CTON_UINT32
CTON_UINT64
CTON_FLOAT8
CTON_FLOAT16
CTON_FLOAT32
CTON_FLOAT64
CTON_OBJECT
CTON_RAW

### Stringify and Serialize

cton_t cton_parse(char * str, size_t len);
size_t cton_stringify(cton_t obj, char * buf, size_t buf_len);
size_t cton_serialize(cton_t obj, char * buf, size_t buf_len);

### CTON struct common call

cton_type_t cton_typeof(cton_t obj);
size_t    cton_sizeof(cton_t obj);

### CTON NULL

cton_t cton_null_new(void);
void   cton_null_del(cton_t obj, int flags);

### CTON Boolean

cton_t cton_bool_new(int value);
void   cton_bool_del(cton_t obj, int flags);

int    cton_bool_get_value(cton_t obj);
void   cton_bool_set_value(cton_t obj, int value);

### CTON Binary String

cton_t cton_string_new(size_t length, uint8_t *data);
void   cton_string_del(cton_t str);
size_t cton_string_get_size(cton_t str);
void*  cton_string_get_value(cton_t str);
void   cton_string_resize(size_t length);

### CTON Array

cton_t cton_array_new(cton_t templete, size_t length);
void   cton_array_del(cton_t arr, int flags)

cton_type_t cton_array_get_type(cton_t arr);

size_t cton_array_get_length(cton_t arr);
size_t cton_array_set_length(cton_t arr, size_t length);

cton_t cton_array_get_element(cton_t arr, size_t index);
int    cton_array_set_element(cton_t arr, size_t index, cton_t obj);

int cton_array_foreach(cton_t arr, int(*call)(size_t, void*));

### CTON Hash

cton_t cton_hash_new();
void   cton_hash_del();

int    cton_hash_insert(cton_t hash, cton_t key, cton_t value);
cton_t cton_hash_search(cton_t hash, cton_t key);
int    cton_hash_delete(cton_t hash, cton_t key);

int cton_hash_foreach(cton_t hash, int(*call)(cton_t, cton_t, void*));

### CTON Numeric

cton_t cton_num_new(cton_type_t type, ...);
void   cton_num_del(cton_t obj);

void*  cton_num_getval(cton_t obj);


