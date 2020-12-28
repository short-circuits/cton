#include <ctype.h>
#include <cton.h>

/*******************************************************************************
 * JSON parse
 ******************************************************************************/

struct cton_json_ctx_s {
	cton_ctx *ctx;
	cton_obj *jctx;
	cton_obj *buf;
	size_t    index;
};

typedef struct cton_json_ctx_s cton_json_ctx;

cton_obj *cton_json_parse_value(cton_ctx *ctx,
	const char *json, size_t *index, size_t len);
size_t cton_json_skip_whitespace(cton_ctx *ctx,
	const char *json, size_t start, size_t len);
cton_obj *cton_json_parse_number(cton_ctx *ctx, 
	const char *json, size_t *index, size_t len);



size_t cton_json_skip_whitespace(cton_ctx *ctx,
	const char *json, size_t start, size_t len)
{
	size_t index;

	(void) ctx;

	index = start;
	while (isspace(json[index])) {
		index ++;
		if (index == len) {
			break;
		}
	}

	return index;
}

cton_obj *
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

	*(double *)cton_object_getvalue(ctx, num) = number;
	return num;
}


cton_obj *
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

	*index = cton_json_skip_whitespace(ctx, json, *index, len);
	if (json[*index] == ']') {
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

cton_obj *
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
	cton_string_setlen(ctx, obj, str_len + 1);

	dst = (char *)cton_string_getptr(ctx, obj);
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
				case 'b':  dst[dst_index] = 'b';  break;
				case 'f':  dst[dst_index] = 'f';  break;
				case 'n':  dst[dst_index] = 'n';  break;
				case 'r':  dst[dst_index] = 'f';  break;
				case 't':  dst[dst_index] = 't';  break;
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

cton_obj * cton_json_parse_hash(cton_ctx *ctx,
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

	parse_index = cton_json_skip_whitespace(ctx, json, parse_index, len);

	if (json[parse_index] == '}') {
		*index = parse_index + 1;
		return hash;
	}


	while (parse_index < len) {

		parse_index = cton_json_skip_whitespace(ctx, json, parse_index, len);
		key = cton_json_parse_string(ctx, json, &parse_index, len);
		parse_index = cton_json_skip_whitespace(ctx, json, parse_index, len);

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

cton_obj * cton_json_parse_value(cton_ctx *ctx,
	const char *json, size_t *index, size_t len)
{
	cton_obj * obj;
	char ch;

	*index = cton_json_skip_whitespace(ctx, json, *index, len);

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
		cton_bool_set(ctx, obj, CTON_TRUE);
		*index += 4;

	} else if (strncmp(&json[*index], "false", 5) == 0) {
		obj = cton_object_create(ctx, CTON_BOOL);
		cton_bool_set(ctx, obj, CTON_FALSE);
		*index += 5;

	} else if (strncmp(&json[*index], "null", 4) == 0) {
		obj = cton_object_create(ctx, CTON_NULL);
		*index += 4;

	} else {
		return NULL;
	}

	*index = cton_json_skip_whitespace(ctx, json, *index, len);

	return obj;
}

int cton_json_parse(cton_ctx *ctx, const char *json, size_t len)
{
	size_t index;
	index = 0;
	cton_obj *root;

	root = cton_json_parse_value(ctx, json, &index, len);

	cton_tree_setroot(ctx, root);

	return 0;
}


/*******************************************************************************
 * JSON stringify
 ******************************************************************************/
#define CTON_JSON_BUFPAGE 4096
int cton_json_stringify_obj(cton_json_ctx *jctx, cton_obj *obj);

int cton_json_stringify_bufputchar(cton_json_ctx *jctx, int c)
{
	int array_len;
	cton_obj *str;
	char     *ptr;

	array_len = cton_array_getlen(jctx->ctx, jctx->buf);

	if (jctx->index % CTON_JSON_BUFPAGE == 0) {
		/* expand buffer first */
		array_len = cton_array_getlen(jctx->ctx, jctx->buf);
		array_len += 1;
		cton_array_setlen(jctx->ctx, jctx->buf, array_len);

		str = cton_object_create(jctx->ctx, CTON_STRING);
		cton_string_setlen(jctx->ctx, str, CTON_JSON_BUFPAGE);
		cton_array_set(jctx->ctx, jctx->buf, str, array_len - 1);
	}

	str = cton_array_get(jctx->ctx, jctx->buf, array_len - 1);
	ptr = (char *)cton_string_getptr(jctx->ctx, str);
	ptr[jctx->index % CTON_JSON_BUFPAGE] = c;
	jctx->index += 1;

	return c;
}

cton_json_ctx *cton_json_stringify_create_ctx(cton_ctx *ctx)
{
	cton_json_ctx *jctx;
	cton_obj *obj;

	obj = cton_object_create(ctx, CTON_BINARY);

	cton_string_setlen(ctx, obj, sizeof(cton_json_ctx));

	jctx = cton_binary_getptr(ctx, obj);
	jctx->ctx  = ctx;
	jctx->jctx = obj;
	jctx->buf  = cton_object_create(ctx, CTON_ARRAY);
	cton_array_settype(ctx, jctx->buf, CTON_STRING);
	cton_array_setlen(ctx, jctx->buf, 0);

	jctx->index = 0;

	return jctx;
}

cton_obj *cton_json_stringify_buf2str(cton_json_ctx *jctx)
{
	cton_obj *str;
	cton_obj *buf;

	size_t buf_cnt;
	size_t buf_index;
	size_t buf_len;
	size_t ch_index;

	uint8_t *o_ptr;
	uint8_t *buf_ptr;

	str = cton_object_create(jctx->ctx, CTON_STRING);
	cton_string_setlen(jctx->ctx, str, jctx->index + 1);
	o_ptr = cton_string_getptr(jctx->ctx, str);

	buf_len = CTON_JSON_BUFPAGE;
	buf_cnt = cton_array_getlen(jctx->ctx, jctx->buf);
	for (buf_index = 0; buf_index < buf_cnt; buf_index ++) {
		buf = cton_array_get(jctx->ctx, jctx->buf, buf_index);

		buf_ptr = cton_string_getptr(jctx->ctx, buf);

		if (buf_index == buf_cnt - 1) {
			buf_len = jctx->index % CTON_JSON_BUFPAGE;
		}

		for (ch_index = 0; ch_index < buf_len; ch_index ++) {
			*o_ptr = buf_ptr[ch_index];
			o_ptr ++;
		}

	}

	*o_ptr = '\0';

	return str;
}

int cton_json_stringify_number(cton_json_ctx *jctx, cton_obj *obj)
{
	char buf[32] = {0};
	void *ptr;

	int ch_index;

	ptr = cton_object_getvalue(jctx->ctx, obj);

	switch (cton_object_gettype(jctx->ctx, obj)) {
		case CTON_INT8: sprintf(buf, "%hhd", *(int8_t *)ptr); break;
		case CTON_INT16: sprintf(buf, "%hd", *(int16_t *)ptr); break;
		case CTON_INT32: sprintf(buf, "%d", *(int32_t *)ptr); break;
		case CTON_INT64: sprintf(buf, "%ld", *(int64_t *)ptr); break;
		case CTON_UINT8: sprintf(buf, "%hhu", *(uint8_t *)ptr); break;
		case CTON_UINT16: sprintf(buf, "%hu", *(uint16_t *)ptr); break;
		case CTON_UINT32: sprintf(buf, "%u", *(uint32_t *)ptr); break;
		case CTON_UINT64: sprintf(buf, "%lu", *(uint64_t *)ptr); break;
		case CTON_FLOAT32: sprintf(buf, "%g", *(float *)ptr); break;
		case CTON_FLOAT64: sprintf(buf, "%lg", *(double *)ptr); break;
		default:
			/* Should never go here */
			return -1;
	}

	for (ch_index = 0; ch_index < 32; ch_index ++) {
		if (buf[ch_index] == '\0') {
			break;
		}

		cton_json_stringify_bufputchar(jctx, buf[ch_index]);
	}

	return 0;
}

int cton_json_stringify_string(cton_json_ctx *jctx, cton_obj *obj)
{
	char *ptr;
	size_t len;
	size_t index;

	cton_json_stringify_bufputchar(jctx, '\"');

	len = cton_string_getlen(jctx->ctx, obj);
	ptr = (char *)cton_string_getptr(jctx->ctx, obj);

	len --;

	for (index = 0; index < len; index ++) {
		cton_json_stringify_bufputchar(jctx, ptr[index]);
	}

	cton_json_stringify_bufputchar(jctx, '\"');

	return 0;
}

int cton_json_stringify_binary(cton_json_ctx *jctx, cton_obj *obj)
{
	return cton_json_stringify_string(jctx, obj);
}


int cton_json_stringify_array(cton_json_ctx *jctx, cton_obj *obj)
{
	cton_obj *sub_obj;
	size_t len;
	size_t index;

	cton_json_stringify_bufputchar(jctx, '[');

	len = cton_array_getlen(jctx->ctx, obj);

	for (index = 0; index < len; index ++) {
		if (index != 0) {
			cton_json_stringify_bufputchar(jctx, ',');
		}
		
		sub_obj = cton_array_get(jctx->ctx, obj, index);
		cton_json_stringify_obj(jctx, sub_obj);

	}

	cton_json_stringify_bufputchar(jctx, ']');

	return 0;
}

int cton_json_stringify_hash(cton_json_ctx *jctx, cton_obj *obj)
{
	cton_hash_item *kvp;
	size_t cnt = 0;

	cton_json_stringify_bufputchar(jctx, '{');

	kvp = obj->payload.hash.root;

	while (kvp != NULL) {
		if (cnt > 0) {
			cton_json_stringify_bufputchar(jctx, ',');
		}

		cton_json_stringify_string(jctx, kvp->key);
		cton_json_stringify_bufputchar(jctx, ':');
		cton_json_stringify_bufputchar(jctx, ' ');
		cton_json_stringify_obj(jctx, kvp->value);

		kvp = kvp->next;

		cnt ++;
	}

	cton_json_stringify_bufputchar(jctx, '}');

	return 0;
}

int cton_json_stringify_obj(cton_json_ctx *jctx, cton_obj *obj)
{
	cton_bool *b;

	switch (cton_object_gettype(jctx->ctx, obj)) {

		case CTON_NULL:
			cton_json_stringify_bufputchar(jctx, 'n');
			cton_json_stringify_bufputchar(jctx, 'u');
			cton_json_stringify_bufputchar(jctx, 'l');
			cton_json_stringify_bufputchar(jctx, 'l');
			break;

		case CTON_BOOL:
			b = (cton_bool *)cton_object_getvalue(jctx->ctx, obj);

			if (*b == CTON_TRUE) {
				cton_json_stringify_bufputchar(jctx, 't');
				cton_json_stringify_bufputchar(jctx, 'r');
				cton_json_stringify_bufputchar(jctx, 'u');
				cton_json_stringify_bufputchar(jctx, 'e');
			} else {
				cton_json_stringify_bufputchar(jctx, 'f');
				cton_json_stringify_bufputchar(jctx, 'a');
				cton_json_stringify_bufputchar(jctx, 'l');
				cton_json_stringify_bufputchar(jctx, 's');
				cton_json_stringify_bufputchar(jctx, 'e');
			}
			break;

		case CTON_BINARY: return cton_json_stringify_binary(jctx, obj);
		case CTON_STRING: return cton_json_stringify_string(jctx, obj);
		case CTON_ARRAY: return cton_json_stringify_array(jctx, obj);
		case CTON_HASH: return cton_json_stringify_hash(jctx, obj);
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
			return cton_json_stringify_number(jctx, obj);
		case CTON_FLOAT8:
		case CTON_FLOAT16:
		case CTON_OBJECT:
		case CTON_INVALID:
		default:
			return -1;
	}
	return 0;
}

cton_obj * cton_json_stringify(cton_ctx *ctx)
{
	cton_obj *output;
	cton_obj *root;

	cton_json_ctx *jctx;

	jctx = cton_json_stringify_create_ctx(ctx);

	root = cton_tree_getroot(ctx);

	cton_json_stringify_obj(jctx, root);

	output = cton_json_stringify_buf2str(jctx);

	return output;
}