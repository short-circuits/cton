#include <ctype.h>
#include <cton.h>

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
	cton_array_settype(ctx, arr, CTON_OBJECT);

	cnt = 0;
	cton_array_setlen(ctx, arr, cnt);

	cton_json_skip_whitespace(ctx, json, index, len);
	if (json[*index] == ']') {
		*index += 1;
		return arr;
	}

	while (*index < len) {
		cnt ++;
		obj = cton_json_parse_value(ctx, json, index, len);
		cton_array_setlen(ctx, arr, cnt);
		cton_array_set(ctx, arr, obj, cnt - 1);

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

		cton_hash_set(ctx, hash, key, value);

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


/*******************************************************************************
 * JSON stringify
 ******************************************************************************/

#if 0
struct cton_json_ctx_s {
	cton_ctx *ctx;
	cton_obj *jctx;
	cton_obj *buf;
	size_t    index;
};

typedef struct cton_json_ctx_s cton_json_ctx;

#define CTON_JSON_BUFPAGE 4096

static cton_json_ctx *cton_json_stringify_create_ctx(cton_ctx *ctx);
static void cton_json_stringify_destroy_ctx(cton_json_ctx *jctx);
static int cton_json_stringify_bufputchar(cton_json_ctx *jctx, int c);
static cton_obj *cton_json_stringify_buf2str(cton_json_ctx *jctx);
#endif

static int
cton_json_stringify_obj(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int
cton_json_stringify_hash(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int
cton_json_stringify_array(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int
cton_json_stringify_string(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int
cton_json_stringify_binary(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int
cton_json_stringify_number(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);

cton_obj * cton_json_stringify(cton_ctx *ctx, cton_obj *obj)
{
	cton_buf *buf;
	cton_obj *output;

	buf = cton_util_buffer_create(ctx);

	cton_json_stringify_obj(ctx, buf, obj);

	output = cton_util_buffer_pack(buf, CTON_STRING);

	cton_util_buffer_destroy(buf);

	return output;
}


static int cton_json_stringify_obj(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	cton_bool *b;

	switch (cton_object_gettype(obj)) {

		case CTON_NULL:
			cton_util_buffer_putchar(buf, 'n');
			cton_util_buffer_putchar(buf, 'u');
			cton_util_buffer_putchar(buf, 'l');
			cton_util_buffer_putchar(buf, 'l');
			break;

		case CTON_BOOL:
			b = (cton_bool *)cton_object_getvalue(obj);

			if (*b == CTON_TRUE) {
				cton_util_buffer_putchar(buf, 't');
				cton_util_buffer_putchar(buf, 'r');
				cton_util_buffer_putchar(buf, 'u');
				cton_util_buffer_putchar(buf, 'e');
			} else {
				cton_util_buffer_putchar(buf, 'f');
				cton_util_buffer_putchar(buf, 'a');
				cton_util_buffer_putchar(buf, 'l');
				cton_util_buffer_putchar(buf, 's');
				cton_util_buffer_putchar(buf, 'e');
			}
			break;

		case CTON_BINARY: return cton_json_stringify_binary(ctx, buf, obj);
		case CTON_STRING: return cton_json_stringify_string(ctx, buf, obj);
		case CTON_ARRAY: return cton_json_stringify_array(ctx, buf, obj);
		case CTON_HASH: return cton_json_stringify_hash(ctx, buf, obj);
		case CTON_INT8:
		case CTON_INT16:
		case CTON_INT32:
		case CTON_INT64:
		case CTON_UINT8:
		case CTON_UINT16:
		case CTON_UINT32:
		case CTON_UINT64:
		case CTON_FLOAT32:
		case CTON_FLOAT64:
			return cton_json_stringify_number(ctx, buf, obj);
		case CTON_FLOAT8:
		case CTON_FLOAT16:
		case CTON_OBJECT:
		case CTON_INVALID:
		default:
			return -1;
	}
	return 0;
}

static int cton_json_stringify_number(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	char itoa_buf[32] = {0};
	void *ptr;

	int ch_index;

	ptr = cton_object_getvalue(obj);

	switch (cton_object_gettype(obj)) {
		case CTON_INT8: sprintf(itoa_buf, "%hhd", *(int8_t *)ptr); break;
		case CTON_INT16: sprintf(itoa_buf, "%hd", *(int16_t *)ptr); break;
		case CTON_INT32: sprintf(itoa_buf, "%d", *(int32_t *)ptr); break;
		case CTON_INT64: sprintf(itoa_buf, "%ld", *(int64_t *)ptr); break;
		case CTON_UINT8: sprintf(itoa_buf, "%hhu", *(uint8_t *)ptr); break;
		case CTON_UINT16: sprintf(itoa_buf, "%hu", *(uint16_t *)ptr); break;
		case CTON_UINT32: sprintf(itoa_buf, "%u", *(uint32_t *)ptr); break;
		case CTON_UINT64: sprintf(itoa_buf, "%lu", *(uint64_t *)ptr); break;
		case CTON_FLOAT32: sprintf(itoa_buf, "%g", *(float *)ptr); break;
		case CTON_FLOAT64: sprintf(itoa_buf, "%lg", *(double *)ptr); break;
		default:
			/* Should never go here */
			return -1;
	}

	for (ch_index = 0; ch_index < 32; ch_index ++) {
		if (itoa_buf[ch_index] == '\0') {
			break;
		}

		cton_util_buffer_putchar(buf, itoa_buf[ch_index]);
	}

	return 0;
}

static int cton_json_stringify_string(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	char *ptr;
	size_t len;
	size_t index;

	cton_util_buffer_putchar(buf, '\"');

	len = cton_string_getlen(obj);
	ptr = (char *)cton_string_getptr(obj);

	len --;

	for (index = 0; index < len; index ++) {
		if (ptr[index] == '\"' || ptr[index] == '\\') {
			cton_util_buffer_putchar(buf, '\\');
			cton_util_buffer_putchar(buf, ptr[index]);
		} else if (ptr[index] == '\b') {
			cton_util_buffer_putchar(buf, '\\');
			cton_util_buffer_putchar(buf, 'b');
		} else if (ptr[index] == '\f') {
			cton_util_buffer_putchar(buf, '\\');
			cton_util_buffer_putchar(buf, 'f');
		} else if (ptr[index] == '\n') {
			cton_util_buffer_putchar(buf, '\\');
			cton_util_buffer_putchar(buf, 'n');
		} else if (ptr[index] == '\r') {
			cton_util_buffer_putchar(buf, '\\');
			cton_util_buffer_putchar(buf, 'r');
		} else if (ptr[index] == '\t') {
			cton_util_buffer_putchar(buf, '\\');
			cton_util_buffer_putchar(buf, 't');
		} else {
			cton_util_buffer_putchar(buf, ptr[index]);
		}
	}

	cton_util_buffer_putchar(buf, '\"');

	return 0;
}

static int
cton_json_stringify_binary(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	cton_obj *base64;
	int ret;

	base64 = cton_base64_encode(ctx, obj, CTON_BASE64);
	ret = cton_json_stringify_string(ctx, buf, base64);

	cton_object_delete(base64);

	return ret;
}

static int
cton_json_stringify_array_item(cton_ctx *ctx,
	cton_obj *arr_item, size_t index, void *buf)
{

	if (index != 0) {
		cton_util_buffer_putchar(buf, ',');
	}

	cton_json_stringify_obj(ctx, buf, arr_item);

	return 0;
}

static int
cton_json_stringify_array(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	size_t len;

	cton_util_buffer_putchar(buf, '[');

	len = cton_array_getlen(ctx, obj);

	cton_array_foreach(ctx, obj, (void *)buf, cton_json_stringify_array_item);

	cton_util_buffer_putchar(buf, ']');

	return 0;
}

static int cton_json_stringify_hash_item(cton_ctx *ctx,
	cton_obj *key, cton_obj *value, size_t index, void *buf)
{
	if (index > 0) {
		cton_util_buffer_putchar(buf, ',');
	}
	
	cton_json_stringify_string(ctx, buf, key);
	cton_util_buffer_putchar(buf, ':');
	cton_util_buffer_putchar(buf, ' ');
	cton_json_stringify_obj(ctx, buf, value);

	return 0;
}

static int cton_json_stringify_hash(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{

	cton_util_buffer_putchar(buf, '{');

	cton_hash_foreach(ctx, obj, (void *)buf, cton_json_stringify_hash_item);

	cton_util_buffer_putchar(buf, '}');

	return 0;
}

