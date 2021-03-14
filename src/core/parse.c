#include <core/cton_core.h>
#include <ctype.h>


static cton_obj * cton_parse_value(cton_ctx *ctx, struct cton_string_s *ton,
    size_t *index);
static cton_obj * cton_parse_hash(cton_ctx *ctx, struct cton_string_s *ton,
    size_t *index);
static cton_obj * cton_parse_array(cton_ctx *ctx, struct cton_string_s *ton,
    size_t *index, cton_type type);
static cton_obj * cton_parse_string(cton_ctx *ctx, struct cton_string_s *ton,
    size_t *index);
static cton_obj * cton_parse_number(cton_ctx *ctx, struct cton_string_s *ton,
    size_t *index, cton_type type);
static size_t cton_parse_whitespace(cton_ctx *ctx, struct cton_string_s *ton,
    size_t *index);
static size_t cton_parse_sharp_comment(struct cton_string_s *ton,
    size_t *index);
static size_t cton_parse_c_comment(struct cton_string_s *ton,
    size_t *index);
static cton_type cton_parse_type(struct cton_string_s *ton, size_t *index);


cton_obj *
cton_parse(cton_ctx *ctx, cton_obj *ton)
{
    cton_obj   *obj;
    size_t      index;

    index = 0;

    obj = cton_parse_value(ctx, (struct cton_string_s *)ton, &index);

    return obj;
}


static cton_obj *
cton_parse_value(cton_ctx *ctx, struct cton_string_s *ton, size_t *index)
{
    cton_obj   *obj;
    cton_type   pre_type;
    char        ch;

    cton_parse_whitespace(ctx, ton, index);

    if (*index == ton->used) {
        return NULL;
    }

    pre_type = cton_parse_type(ton, index);

    ch = ton->ptr[*index];

    if (ch == '{') {
        if (pre_type != CTON_OBJECT && pre_type != CTON_HASH) {
            return NULL;
        }

        obj = cton_parse_hash(ctx, ton, index);

    } else if (ch == '[') {
        /* Array */
        obj = cton_parse_array(ctx, ton, index, pre_type);

    } else if (ch == '"') {
        /* String */
        obj = cton_parse_string(ctx, ton, index);

    } else if (ch == '-' || ch == '.' || (ch >= '0' && ch <= '9')) {
        if (pre_type == CTON_OBJECT) {
            obj = cton_parse_number(ctx, ton, index, CTON_FLOAT64);
        } else {
            obj = cton_parse_number(ctx, ton, index, pre_type);
        }

    } else if (strncmp((char *)&ton->ptr[*index], "true", 4) == 0) {
        obj = cton_object_create(ctx, CTON_BOOL);
        cton_bool_set(obj, CTON_TRUE);
        *index += 4;

    } else if (strncmp((char *)&ton->ptr[*index], "false", 5) == 0) {
        obj = cton_object_create(ctx, CTON_BOOL);
        cton_bool_set(obj, CTON_FALSE);
        *index += 5;

    } else if (strncmp((char *)&ton->ptr[*index], "null", 4) == 0) {
        obj = cton_object_create(ctx, CTON_NULL);
        *index += 4;

    } else {
        return NULL;
    }

    cton_parse_whitespace(ctx, ton, index);

    return obj;
}

static cton_obj *
cton_parse_string(cton_ctx *ctx, struct cton_string_s *ton, size_t *index)
{
    cton_obj   *obj;
    size_t      str_len;
    size_t      str_index;
    size_t      dst_index;
    char       *str;
    char       *dst;

    str = (char *)&(ton->ptr[*index + 1]);

    /* Get string length */
    str_index = 0;
    str_len   = 0;
    while (str_index + *index < ton->used) {

        if (str[str_index] == '\\') {
            /* quote */
            str_index += 1;
            switch(str[str_index]) {
                case '\"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                    break;
                default:
                    cton_seterr(ctx, CTON_ERROR_IMPLEM);
                    return NULL;
            }

        } else if (str[str_index] == '\"') {
            break;
        }

        str_index += 1;
        str_len   += 1;
    }

    obj = cton_object_create(ctx, CTON_STRING);
    cton_string_setlen(obj, str_len + 1);

    dst = (char *)cton_string_getptr(obj);
    str_index = 0;
    dst_index = 0;

    while (str_index + *index < ton->used) {

        if (str[str_index] == '\\') {
            /* quote */
            str_index += 1;
            switch(str[str_index]) {
                case '\"': dst[dst_index] = '\"'; break;
                case '\\': dst[dst_index] = '\\'; break;
                case '/':  dst[dst_index] = '/';  break;
                case 'b':  dst[dst_index] = '\b';  break;
                case 'f':  dst[dst_index] = '\f';  break;
                case 'n':  dst[dst_index] = '\n';  break;
                case 'r':  dst[dst_index] = '\r';  break;
                case 't':  dst[dst_index] = '\t';  break;
                default:
                    cton_seterr(ctx, CTON_ERROR_IMPLEM);
                    return NULL;
            }

        } else if (str[str_index] == '\"') {
            break;
        } else {
            dst[dst_index] = str[str_index];
        }

        str_index += 1;
        dst_index += 1;
    }

    *index += str_index + 2; /* 2 for the commas */

    return obj;
}

static cton_obj *
cton_parse_hash(cton_ctx *ctx, struct cton_string_s *ton, size_t *index)
{
    cton_obj *hash;
    cton_obj *key;
    cton_obj *value;

    size_t    parse_index;

    hash = cton_object_create(ctx, CTON_HASH);

    parse_index = *index;
    parse_index += 1;

    cton_parse_whitespace(ctx, ton, index);

    if (ton->ptr[parse_index] == '}') {
        /* empty hash */
        *index = parse_index + 1;
        return hash;
    }


    while (parse_index < ton->used) {

        cton_parse_whitespace(ctx, ton, &parse_index);
        key = cton_parse_value(ctx, ton, &parse_index);
        cton_parse_whitespace(ctx, ton, &parse_index);

        if (ton->ptr[parse_index] != ':') {
            fprintf(stderr, "Get '%c', exceped ':'\n", ton->ptr[parse_index]);
            cton_seterr(ctx, CTON_ERROR_IMPLEM);
            *index = parse_index;
            return hash;
        }
        parse_index ++;

        value = cton_parse_value(ctx, ton, &parse_index);

        cton_hash_set(hash, key, value);

        if (ton->ptr[parse_index] == '}') {
            break;
        } else if (ton->ptr[parse_index] != ',') {
            break;
        }

        parse_index++;
    }

    *index = parse_index + 1;

    return hash;
}

static cton_obj *
cton_parse_array(cton_ctx *ctx, struct cton_string_s *ton, size_t *index,
    cton_type type)
{
    cton_obj *arr;
    cton_obj *obj;
    int cnt;
    int keep_obj;

    *index += 1;

    arr = cton_object_create(ctx, CTON_ARRAY);
    cton_array_settype(arr, type);

    keep_obj = cton_array_complex(arr);

    cnt = 0;
    cton_array_setlen(arr, cnt);

    cton_parse_whitespace(ctx, ton, index);

    if (ton->ptr[*index] == ']') {
        *index += 1;
        return arr;
    }

    while (*index < ton->used) {
        cnt ++;
        obj = cton_parse_value(ctx, ton, index);

        cton_array_setlen(arr, cnt);
        cton_array_set(arr, obj, cnt - 1);

        if (keep_obj == 1) {
            cton_object_delete(obj);
        }

        if (ton->ptr[*index] == ']') {
            break;
        } else if (ton->ptr[*index] == ',') {
            *index += 1;
        } else {
            cton_seterr(ctx, CTON_ERROR_IMPLEM);
            break;
        }
    }

    *index += 1;

    return arr;
}

cton_obj *
cton_parse_number(cton_ctx *ctx, struct cton_string_s *ton, size_t *index,
    cton_type type)
{
    cton_obj * num;

    int   dump_index;
    char  ch;
    char  c_string_buffer[128];
    char *endptr = NULL;

    double  number_f;
    int64_t number_i;

    void *nptr;

    for (dump_index = 0; dump_index < 128; dump_index ++) {
        ch = ton->ptr[*index + dump_index];
        if ((ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || \
            ch == 'e' || ch == 'E' || ch == '.') {

            c_string_buffer[dump_index] = ch;

        } else {
            break;
        }

        if (*index + dump_index >= ton->used) {
            break;
        }
    }

    c_string_buffer[dump_index] = '\0';

    if (type == CTON_FLOAT32 || type == CTON_FLOAT64) {
        number_f = strtod(c_string_buffer, &endptr);
        if (c_string_buffer == endptr) {
            return NULL;
        }
    } else {
        number_i = strtol(c_string_buffer, &endptr, 10);
        if (c_string_buffer == endptr) {
            return NULL;
        }
    }

    *index += dump_index;

    num  = cton_object_create(ctx, type);

    nptr = cton_object_getvalue(num);

    switch (type) {
        case CTON_INT8:    *(int8_t *)nptr   = number_i; break;
        case CTON_INT16:   *(int16_t *)nptr  = number_i; break;
        case CTON_INT32:   *(int32_t *)nptr  = number_i; break;
        case CTON_INT64:   *(int64_t *)nptr  = number_i; break;
        case CTON_UINT8:   *(uint8_t *)nptr  = number_i; break;
        case CTON_UINT16:  *(uint16_t *)nptr = number_i; break;
        case CTON_UINT32:  *(uint32_t *)nptr = number_i; break;
        case CTON_UINT64:  *(uint64_t *)nptr = number_i; break;
        case CTON_FLOAT32: *(float *)nptr    = number_f; break;
        case CTON_FLOAT64: *(double *)nptr   = number_f; break;
        default:
            *(int8_t *)nptr = number_f;
    }

    return num;
}

static size_t
cton_parse_whitespace(cton_ctx *ctx, struct cton_string_s *ton, size_t *index)
{
    size_t len;
    size_t now;

    len = ton->used;
    now = *index;

    if (now >= len) {
        cton_seterr(ctx, CTON_ERROR_INDEX);
        return len;
    }
    
    while ( now < len ) {

        if (isspace(ton->ptr[now])) {
            now += 1;
            continue;

        } else if (ton->ptr[now] == '#') {
            cton_parse_sharp_comment(ton, &now);

        } else if ((len - now > 2) && \
                    (ton->ptr[now] == '/' && ton->ptr[now + 1] == '*')) {
            now += 1;
            cton_parse_c_comment(ton, &now);

        } else {
            break;
        }
    }

    *index = now;

    return now;
}

static size_t
cton_parse_sharp_comment(struct cton_string_s *ton, size_t *index)
{
    size_t len;

    len = ton->len;

    while (*index < len) {
        if (ton->ptr[*index] == '\n') {
            break;
        }

        *index += 1;
    }

    return *index;
}

static size_t
cton_parse_c_comment(struct cton_string_s *ton, size_t *index)
{
    size_t len;

    len = ton->len;

    while (*index < len) {
        if ( ton->ptr[*index] == '/' && ton->ptr[*index - 1] == '*') {
            break;
        }

        *index += 1;
    }

    return *index;
}

static cton_type
cton_parse_type(struct cton_string_s *ton, size_t *index)
{
    cton_type type;
    size_t    pos;

    type = CTON_OBJECT;
    pos  = *index;

    if (ton->ptr[pos] == 'a') {
        /* Array type */
        type = CTON_ARRAY;

    } else if (ton->ptr[pos] == 'n') {
        /* Array type */
        type = CTON_NULL;

    } else if (ton->ptr[pos] == 'b') {
        /* Bool */
        type = CTON_BOOL;
        
    } else if (ton->ptr[pos] == 'c') {
        /* Binary */
        type = CTON_BINARY;
        
    } else if (ton->ptr[pos] == 'h') {
        /* Hash */
        type = CTON_HASH;
        
    } else if (ton->ptr[pos] == 's') {
        /* String */
        type = CTON_STRING;
        
    } else if (ton->ptr[pos] == 'i') {
        /* Interger */

        if (ton->ptr[pos + 1] == '8') {
            type = CTON_INT8;
            pos += 1;

        } else if (ton->ptr[pos + 1] == '1' && ton->ptr[pos + 2] == '6') {
            type = CTON_INT16;
            pos += 2;

        } else if (ton->ptr[pos + 1] == '3' && ton->ptr[pos + 2] == '2') {
            type = CTON_INT32;
            pos += 2;

        } else if (ton->ptr[pos + 1] == '6' && ton->ptr[pos + 2] == '4') {
            type = CTON_INT64;
            pos += 2;

        }

    } else if (ton->ptr[pos] == 'u') {
        /* Unsigned */

        if (ton->ptr[pos + 1] == '8') {
            type = CTON_UINT8;
            pos += 1;

        } else if (ton->ptr[pos + 1] == '1' && ton->ptr[pos + 2] == '6') {
            type = CTON_UINT16;
            pos += 2;

        } else if (ton->ptr[pos + 1] == '3' && ton->ptr[pos + 2] == '2') {
            type = CTON_UINT32;
            pos += 2;

        } else if (ton->ptr[pos + 1] == '6' && ton->ptr[pos + 2] == '4') {
            type = CTON_UINT64;
            pos += 2;

        }

    } else if (ton->ptr[pos] == 'f') {
        /* Float */

        if (ton->ptr[pos + 1] == '8') {
            type = CTON_FLOAT8;
            pos += 1;

        } else if (ton->ptr[pos + 1] == '1' && ton->ptr[pos + 2] == '6') {
            type = CTON_FLOAT16;
            pos += 2;

        } else if (ton->ptr[pos + 1] == '3' && ton->ptr[pos + 2] == '2') {
            type = CTON_FLOAT32;
            pos += 2;

        } else if (ton->ptr[pos + 1] == '6' && ton->ptr[pos + 2] == '4') {
            type = CTON_FLOAT64;
            pos += 2;

        }
    }

    if (type != CTON_OBJECT && ton->ptr[pos + 1] == ':') {
        *index = pos + 1;
    }

    return type;
}

#if 0
/*******************************************************************************
 * JSON parse
 ******************************************************************************/

static cton_obj *cton_json_parse_value(cton_ctx *ctx,
    const char *json, size_t *index, size_t len);
static cton_obj *cton_json_parse_number(cton_ctx *ctx, 
    const char *json, size_t *index, size_t len);
static cton_obj *cton_json_parse_array(cton_ctx *ctx, 
    const char *json, size_t *index, size_t len);
static cton_obj *cton_json_parse_hash(cton_ctx *ctx, 
    const char *json, size_t *index, size_t len);
static cton_obj *cton_json_parse_string(cton_ctx *ctx, 
    const char *json, size_t *index, size_t len);
static size_t cton_json_skip_whitespace(cton_ctx *ctx,
    const char *json, size_t *start, size_t len);


cton_obj *cton_json_parse(cton_ctx *ctx, cton_obj *json)
{
    cton_obj *obj;
    size_t index;

    index = 0;
    obj = cton_json_parse_value(ctx, 
        (char *)cton_string_getptr(json), &index, cton_string_getlen(json));

    return obj;
}

static cton_obj * cton_json_parse_value(cton_ctx *ctx,
    const char *json, size_t *index, size_t len)
{
    cton_obj * obj;
    char ch;

    cton_json_skip_whitespace(ctx, json, index, len);

    if (*index == len) {
        return NULL;
    }

    ch = json[*index];

    if (ch == '{') {
        /* Hash (json:object) */
        obj = cton_json_parse_hash(ctx, json, index, len);

    } else if (ch == '[') {
        /* Array (json:array) */
        obj = cton_json_parse_array(ctx, json, index, len);

    } else if (ch == '\"') {
        /* String (json:string) */
        obj = cton_json_parse_string(ctx, json, index, len);

    } else if (ch == '-' || ch == '.' || (ch >= '0' && ch <= '9')) {
        obj = cton_json_parse_number(ctx, json, index, len);

    } else if (strncmp(&json[*index], "true", 4) == 0) {
        obj = cton_object_create(ctx, CTON_BOOL);
        cton_bool_set(obj, CTON_TRUE);
        *index += 4;

    } else if (strncmp(&json[*index], "false", 5) == 0) {
        obj = cton_object_create(ctx, CTON_BOOL);
        cton_bool_set(obj, CTON_FALSE);
        *index += 5;

    } else if (strncmp(&json[*index], "null", 4) == 0) {
        obj = cton_object_create(ctx, CTON_NULL);
        *index += 4;

    } else {
        return NULL;
    }

    cton_json_skip_whitespace(ctx, json, index, len);

    return obj;
}

static cton_obj *
cton_json_parse_number(cton_ctx *ctx, 
    const char *json, size_t *index, size_t len)
{
    cton_obj * num;

    int   dump_index;
    char  ch;
    char  c_string_buffer[128];
    char *endptr = NULL;

    double number;

    (void)len;

    for (dump_index = 0; dump_index < 128; dump_index ++) {
        ch = json[*index + dump_index];
        switch (ch) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
            case '.':
                c_string_buffer[dump_index] = ch;
                break;
            default:
                goto loop_end;
        }
    }
loop_end:
    c_string_buffer[dump_index] = '\0';

    number = strtod(c_string_buffer, &endptr);
    if (c_string_buffer == endptr) {
        return NULL;
    }

    *index += dump_index;

    /* JSON treated all of the number as signed float64 */
    num = cton_object_create(ctx, CTON_FLOAT64);

    *(double *)cton_object_getvalue(num) = number;
    return num;
}


static cton_obj *
cton_json_parse_array(cton_ctx *ctx, 
    const char *json, size_t *index, size_t len)
{
    cton_obj *arr;
    cton_obj *obj;
    int cnt;


    if (json[*index] != '[') {
        return NULL;
    }

    *index += 1;

    arr = cton_object_create(ctx, CTON_ARRAY);
    cton_array_settype(arr, CTON_OBJECT);

    cnt = 0;
    cton_array_setlen(arr, cnt);

    cton_json_skip_whitespace(ctx, json, index, len);
    if (json[*index] == ']') {
        *index += 1;
        return arr;
    }

    while (*index < len) {
        cnt ++;
        obj = cton_json_parse_value(ctx, json, index, len);
        cton_array_setlen(arr, cnt);
        cton_array_set(arr, obj, cnt - 1);

        if (json[*index] == ']') {
            break;
        } else if (json[*index] == ',') {
            *index += 1;
        } else {
            cton_seterr(ctx, CTON_ERROR_IMPLEM);
            break;
        }
    }

    *index += 1;

    return arr;
}

static cton_obj *
cton_json_parse_string(cton_ctx *ctx, 
    const char *json, size_t *index, size_t len)
{
    cton_obj   *obj;
    size_t      str_len;
    size_t      str_index;
    size_t      dst_index;
    const char *str;
    char       *dst;

    if (json[*index] != '\"') {
        return NULL;
    }

    str = &json[*index + 1];

    /* Get string length */
    str_index = 0;
    str_len   = 0;
    while (str_index + *index < len) {

        if (str[str_index] == '\\') {
            /* quote */
            str_index += 1;
            switch(str[str_index]) {
                case '\"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                    break;
                default:
                    cton_seterr(ctx, CTON_ERROR_IMPLEM);
                    return NULL;
            }

        } else if (str[str_index] == '\"') {
            break;
        }

        str_index += 1;
        str_len   += 1;
    }

    obj = cton_object_create(ctx, CTON_STRING);
    cton_string_setlen(obj, str_len + 1);

    dst = (char *)cton_string_getptr(obj);
    str_index = 0;
    dst_index = 0;

    while (str_index + *index < len) {

        if (str[str_index] == '\\') {
            /* quote */
            str_index += 1;
            switch(str[str_index]) {
                case '\"': dst[dst_index] = '\"'; break;
                case '\\': dst[dst_index] = '\\'; break;
                case '/':  dst[dst_index] = '/';  break;
                case 'b':  dst[dst_index] = '\b';  break;
                case 'f':  dst[dst_index] = '\f';  break;
                case 'n':  dst[dst_index] = '\n';  break;
                case 'r':  dst[dst_index] = '\r';  break;
                case 't':  dst[dst_index] = '\t';  break;
                default:
                    cton_seterr(ctx, CTON_ERROR_IMPLEM);
                    return NULL;
            }

        } else if (str[str_index] == '\"') {
            break;
        } else {
            dst[dst_index] = str[str_index];
        }

        str_index += 1;
        dst_index += 1;
    }

    *index += str_index + 2; /* 2 for the commas */

    return obj;
}

static cton_obj * cton_json_parse_hash(cton_ctx *ctx,
    const char *json, size_t *index, size_t len)
{
    cton_obj *hash;
    cton_obj *key;
    cton_obj *value;

    size_t    parse_index;

    if (json[*index] != '{') {
        return NULL;
    }

    hash = cton_object_create(ctx, CTON_HASH);

    parse_index = *index;
    parse_index ++;

    cton_json_skip_whitespace(ctx, json, &parse_index, len);

    if (json[parse_index] == '}') {
        *index = parse_index + 1;
        return hash;
    }


    while (parse_index < len) {

        cton_json_skip_whitespace(ctx, json, &parse_index, len);
        key = cton_json_parse_string(ctx, json, &parse_index, len);
        cton_json_skip_whitespace(ctx, json, &parse_index, len);

        if (json[parse_index] != ':') {
            fprintf(stderr, "Get '%c', exceped ':'\n", json[parse_index]);
            cton_seterr(ctx, CTON_ERROR_IMPLEM);
            *index = parse_index;
            return hash;
        }
        parse_index ++;

        value = cton_json_parse_value(ctx, json, &parse_index, len);

        cton_hash_set(hash, key, value);

        if (json[parse_index] == '}') {
            break;
        } else if (json[parse_index] != ',') {
            break;
        }

        parse_index++;
    }

    *index = parse_index + 1;

    return hash;
}

static size_t cton_json_skip_whitespace(cton_ctx *ctx,
    const char *json, size_t *start, size_t len)
{
    size_t index;

    (void) ctx;

    index = *start;

    if (index >= len) {
        return len;
    }
    
    while (isspace(json[index])) {
        index ++;
        if (index == len) {
            break;
        }
    }

    *start = index;

    return index;
}
#endif
