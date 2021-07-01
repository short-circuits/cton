
#include <core/cton_core.h>

#include <ctype.h>

/*******************************************************************************
 * CTON stringify
 ******************************************************************************/

static void cton_object_stringify(cton_buf *buf, cton_obj *obj);
static void cton_type_stringify(cton_buf *buf, cton_type type);
static int cton_hash_stringify(cton_buf *buf, cton_obj *obj);
static int cton_array_stringify(cton_buf *buf, cton_obj *obj);
static int cton_string_stringify(cton_buf *buf, cton_obj *obj);
static int cton_binary_stringify(cton_buf *buf, cton_obj *obj);



/*
 * cton_stringify()
 */
cton_obj *
cton_stringify(cton_ctx *ctx, cton_obj *obj)
{
	cton_buf *buf;
	cton_obj *output;

	buf = cton_buffer_create(ctx);

	cton_object_stringify(buf, obj);

	output = cton_buffer_pack(buf, CTON_STRING);

	cton_buffer_destroy(buf);

	return output;
}

static void cton_type_stringify(cton_buf *buf, cton_type type)
{
	switch (type) {
		case CTON_NULL:
			break;

		case CTON_BOOL:
			break;

		case CTON_BINARY:
			cton_buffer_puts(buf, "c:");
			break;

		case CTON_STRING:
			cton_buffer_puts(buf, "s:");
			break;

		case CTON_ARRAY:
			break;

		case CTON_HASH:
			break;

		case CTON_INT8:
			cton_buffer_puts(buf, "i8:");
			break;

		case CTON_INT16:
			cton_buffer_puts(buf, "i16:");
			break;

		case CTON_INT32:
			cton_buffer_puts(buf, "i32:");
			break;

		case CTON_INT64:
			cton_buffer_puts(buf, "i64:");
			break;

		case CTON_UINT8:
			cton_buffer_puts(buf, "u8:");
			break;

		case CTON_UINT16:
			cton_buffer_puts(buf, "u16:");
			break;

		case CTON_UINT32:
			cton_buffer_puts(buf, "u32:");
			break;

		case CTON_UINT64:
			cton_buffer_puts(buf, "u64:");
			break;

		case CTON_FLOAT32:
			cton_buffer_puts(buf, "f32:");
			break;

		case CTON_FLOAT64:
			cton_buffer_puts(buf, "f64:");
			break;

		case CTON_FLOAT8:
		case CTON_FLOAT16:
		case CTON_OBJECT:
		case CTON_INVALID:
		default:
			return ;
	}
}

static void cton_arrtype_stringify(cton_buf *buf, cton_type type)
{
	switch (type) {
		case CTON_NULL:
			cton_buffer_puts(buf, "n:");
			break;

		case CTON_BOOL:
			cton_buffer_puts(buf, "b:");
			break;

		case CTON_BINARY:
			cton_buffer_puts(buf, "c:");
			break;

		case CTON_STRING:
			cton_buffer_puts(buf, "s:");
			break;

		case CTON_ARRAY:
			cton_buffer_puts(buf, "a:");
			break;

		case CTON_HASH:
			cton_buffer_puts(buf, "h:");
			break;

		case CTON_INT8:
			cton_buffer_puts(buf, "i8:");
			break;

		case CTON_INT16:
			cton_buffer_puts(buf, "i16:");
			break;

		case CTON_INT32:
			cton_buffer_puts(buf, "i32:");
			break;

		case CTON_INT64:
			cton_buffer_puts(buf, "i64:");
			break;

		case CTON_UINT8:
			cton_buffer_puts(buf, "u8:");
			break;

		case CTON_UINT16:
			cton_buffer_puts(buf, "u16:");
			break;

		case CTON_UINT32:
			cton_buffer_puts(buf, "u32:");
			break;

		case CTON_UINT64:
			cton_buffer_puts(buf, "u64:");
			break;

		case CTON_FLOAT32:
			cton_buffer_puts(buf, "f32:");
			break;

		case CTON_FLOAT64:
			cton_buffer_puts(buf, "f64:");
			break;

		case CTON_FLOAT8:
		case CTON_FLOAT16:
		case CTON_OBJECT:
		case CTON_INVALID:
		default:
			return ;
	}
}

static int cton_value_stringify(cton_buf *buf, cton_obj *obj, cton_type type)
{
	char       itoa_buf[32] = {0};
	void       *ptr;
	cton_bool  *b;

	ptr = cton_object_getvalue(obj);

	switch (type) {

		case CTON_NULL:
			cton_buffer_putchar(buf, 'n');
			cton_buffer_putchar(buf, 'u');
			cton_buffer_putchar(buf, 'l');
			cton_buffer_putchar(buf, 'l');
			break;

		case CTON_BOOL:
			b = (cton_bool *)ptr;

			if (*b == CTON_TRUE) {
				cton_buffer_putchar(buf, 't');
				cton_buffer_putchar(buf, 'r');
				cton_buffer_putchar(buf, 'u');
				cton_buffer_putchar(buf, 'e');
			} else {
				cton_buffer_putchar(buf, 'f');
				cton_buffer_putchar(buf, 'a');
				cton_buffer_putchar(buf, 'l');
				cton_buffer_putchar(buf, 's');
				cton_buffer_putchar(buf, 'e');
			}
			break;

		case CTON_BINARY: return cton_binary_stringify(buf, obj);
		case CTON_STRING: return cton_string_stringify(buf, obj);
		case CTON_ARRAY: return cton_array_stringify(buf, obj);
		case CTON_HASH: return cton_hash_stringify(buf, obj);
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
		case CTON_FLOAT8:
		case CTON_FLOAT16:
		case CTON_OBJECT:
		case CTON_INVALID:
		default:
			return -1;
	}

	cton_buffer_puts(buf, itoa_buf);

	return 0;
}

static void cton_object_stringify(cton_buf *buf, cton_obj *obj)
{
	cton_type  t;
	t = cton_object_gettype(obj);
	cton_type_stringify(buf, t);
	cton_value_stringify(buf, obj, t);
}

static int cton_string_stringify(cton_buf *buf, cton_obj *obj)
{
	char *ptr;
	size_t len;
	size_t index;

	cton_buffer_putchar(buf, '\"');

	len = cton_string_getlen(obj);
	ptr = (char *)cton_string_getptr(obj);

	len --;

	for (index = 0; index < len; index ++) {
		if (ptr[index] == '\"' || ptr[index] == '\\') {
			cton_buffer_putchar(buf, '\\');
			cton_buffer_putchar(buf, ptr[index]);
		} else if (ptr[index] == '\b') {
			cton_buffer_putchar(buf, '\\');
			cton_buffer_putchar(buf, 'b');
		} else if (ptr[index] == '\f') {
			cton_buffer_putchar(buf, '\\');
			cton_buffer_putchar(buf, 'f');
		} else if (ptr[index] == '\n') {
			cton_buffer_putchar(buf, '\\');
			cton_buffer_putchar(buf, 'n');
		} else if (ptr[index] == '\r') {
			cton_buffer_putchar(buf, '\\');
			cton_buffer_putchar(buf, 'r');
		} else if (ptr[index] == '\t') {
			cton_buffer_putchar(buf, '\\');
			cton_buffer_putchar(buf, 't');
		} else {
			cton_buffer_putchar(buf, ptr[index]);
		}
	}

	cton_buffer_putchar(buf, '\"');

	return 0;
}

static int
cton_binary_stringify(cton_buf *buf, cton_obj *obj)
{
	cton_obj  *base64;
	char      *ptr;

	base64 = cton_base64_encode(buf->ctx, obj, CTON_BASE64);

	ptr = (char *)cton_string_getptr(base64);

	cton_buffer_puts(buf, ptr);

	cton_object_delete(base64);

	return 0;
}

static int
cton_array_stringify_object(cton_ctx *ctx,
	cton_obj *arr_item, size_t index, void *buf)
{
	(void) ctx;
	if (index != 0) {
		cton_buffer_putchar(buf, ',');
	}

	cton_object_stringify(buf, arr_item);

	return 0;
}

static int
cton_array_stringify_value(cton_ctx *ctx,
	cton_obj *arr_item, size_t index, void *buf)
{
	(void) ctx;
	if (index != 0) {
		cton_buffer_putchar(buf, ',');
	}

	cton_value_stringify(buf, arr_item, cton_object_gettype(arr_item));

	return 0;
}

static int
cton_array_stringify(cton_buf *buf, cton_obj *obj)
{
	cton_type type;

	type = cton_array_gettype(obj);

	if (type != CTON_OBJECT) {
		cton_arrtype_stringify(buf, type);
	}

	cton_buffer_putchar(buf, '[');

	if (type == CTON_OBJECT) {
		cton_array_foreach(obj, (void *)buf, cton_array_stringify_object);
	} else {
		cton_array_foreach(obj, (void *)buf, cton_array_stringify_value);
	}

	cton_buffer_putchar(buf, ']');

	return 0;
}

static int cton_hash_stringify_item(cton_ctx *ctx,
	cton_obj *key, cton_obj *value, size_t index, void *buf)
{
	(void) ctx;

	if (index > 0) {
		cton_buffer_putchar(buf, ',');
	}
	
	cton_string_stringify(buf, key);
	cton_buffer_putchar(buf, ':');
	cton_buffer_putchar(buf, ' ');
	cton_object_stringify(buf, value);

	return 0;
}

static int cton_hash_stringify(cton_buf *buf, cton_obj *obj)
{	
	cton_buffer_putchar(buf, '{');

	cton_hash_foreach(obj, (void *)buf, cton_hash_stringify_item);

	cton_buffer_putchar(buf, '}');

	return 0;
}
