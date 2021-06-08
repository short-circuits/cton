#include <ctype.h>
#include <core/cton_core.h>
#include <assert.h>

static int cton_serialize_object(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_hash(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_array(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_string(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_binary(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);


static int
tbonv2_serialize_object(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);

cton_obj *
cton_tbonv2_serialize(cton_ctx *ctx, cton_obj *obj)
{
	cton_buf *buf;
	cton_obj *output;

	buf = cton_buffer_create(ctx);

	/* Magic Header */
	cton_buffer_puts(buf, "TBON");

	/* Version */
	cton_buffer_putchar(buf, 0x00);
	cton_buffer_putchar(buf, 0x02);

	tbonv2_serialize_object(ctx, buf, obj);

	output = cton_buffer_pack(buf, CTON_STRING);

	cton_buffer_destroy(buf);

	return output;
}

static int
tbonv2_serialize_object(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	uint8_t id;

	cton_type type;
	cton_bool *b;

	if (type == CTON_NULL) {
		cton_buffer_putchar(buf, 0x01);
	} else if (type == CTON_BOOL) {
		b = (cton_bool *)cton_object_getvalue(obj);

		if (*b == CTON_TRUE) {
			cton_buffer_putchar(buf, 0x02);
		} else {
			cton_buffer_putchar(buf, 0x03);
		}
	}

	return 0;
}

#if 0

#define TBON_ID_OBJECT  0
#define TBON_ID_NULL    1
#define TBON_ID_TRUE    2
#define TBON_ID_FALSE   3
#define TBON_ID_BINARY  4
#define TBON_ID_STRING  5
#define TBON_ID_ARRAY   6
#define TBON_ID_HASH    7
#define TBON_ID_INT8    8
#define TBON_ID_INT16   9
#define TBON_ID_INT32   10
#define TBON_ID_INT64   11
#define TBON_ID_UINT8   12
#define TBON_ID_UINT16  13
#define TBON_ID_UINT32  14
#define TBON_ID_UINT64  15
#define TBON_ID_FLOAT8  16
#define TBON_ID_FLOAT16 17
#define TBON_ID_FLOAT32 18
#define TBON_ID_FLOAT64 19

static uint8_t cton_getid(cton_type type)
{
	switch (type) {

		case CTON_OBJECT: return TBON_ID_OBJECT;
		case CTON_NULL: return TBON_ID_NULL;
		case CTON_BOOL: return TBON_ID_TRUE;
		case CTON_BINARY: return TBON_ID_BINARY;
		case CTON_STRING: return TBON_ID_STRING;
		case CTON_ARRAY: return TBON_ID_ARRAY;
		case CTON_HASH: return TBON_ID_HASH;
		case CTON_INT8: return TBON_ID_INT8;
		case CTON_INT16: return TBON_ID_INT16;
		case CTON_INT32: return TBON_ID_INT32;
		case CTON_INT64: return TBON_ID_INT64;
		case CTON_UINT8: return TBON_ID_UINT8;
		case CTON_UINT16: return TBON_ID_UINT16;
		case CTON_UINT32: return TBON_ID_UINT32;
		case CTON_UINT64: return TBON_ID_UINT64;
		case CTON_FLOAT8: return TBON_ID_FLOAT8;
		case CTON_FLOAT16: return TBON_ID_FLOAT16;
		case CTON_FLOAT32: return TBON_ID_FLOAT32;
		case CTON_FLOAT64: return TBON_ID_FLOAT64;

		case CTON_INVALID:
		default:
			return 255;
	}

	return 255;
}


static int cton_serialize_object(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_hash(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_array(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_string(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int cton_serialize_binary(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);

static void cton_serialize_8bit(cton_ctx *ctx, cton_buf *buf, void *ptr);
static void cton_serialize_16bit(cton_ctx *ctx, cton_buf *buf, void *ptr);
static void cton_serialize_32bit(cton_ctx *ctx, cton_buf *buf, void *ptr);
static void cton_serialize_64bit(cton_ctx *ctx, cton_buf *buf, void *ptr);
static void cton_serialize_vw(cton_ctx *ctx, cton_buf *buf, uint64_t data);


static void
cton_serialize_8bit(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint8_t data;

	(void)ctx;

	data = *(uint8_t *)ptr;
	cton_util_buffer_putchar(buf, data);
}


static void
cton_serialize_16bit(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint16_t data;

	(void)ctx;

	data = *(uint16_t *)ptr;
	cton_util_buffer_putchar(buf, ((data & 0xFF00) >> 8));
	cton_util_buffer_putchar(buf,  (data & 0x00FF));
}


static void
cton_serialize_32bit(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint32_t data;

	(void)ctx;

	data = *(uint32_t *)ptr;
	cton_util_buffer_putchar(buf, ((data & 0xFF000000) >> 24));
	cton_util_buffer_putchar(buf, ((data & 0x00FF0000) >> 16));
	cton_util_buffer_putchar(buf, ((data & 0x0000FF00) >> 8));
	cton_util_buffer_putchar(buf,  (data & 0x000000FF));
}


static void
cton_serialize_64bit(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint64_t data;

	(void)ctx;

	data = *(uint64_t *)ptr;
	cton_util_buffer_putchar(buf, ((data & 0xFF00000000000000UL) >> 56));
	cton_util_buffer_putchar(buf, ((data & 0x00FF000000000000UL) >> 48));
	cton_util_buffer_putchar(buf, ((data & 0x0000FF0000000000UL) >> 40));
	cton_util_buffer_putchar(buf, ((data & 0x000000FF00000000UL) >> 32));
	cton_util_buffer_putchar(buf, ((data & 0x00000000FF000000UL) >> 24));
	cton_util_buffer_putchar(buf, ((data & 0x0000000000FF0000UL) >> 16));
	cton_util_buffer_putchar(buf, ((data & 0x000000000000FF00UL) >> 8));
	cton_util_buffer_putchar(buf,  (data & 0x00000000000000FFUL));
}


static void
cton_serialize_vw(cton_ctx *ctx, cton_buf *buf, uint64_t data)
{
	(void)ctx;

	if (data <= 0x7F) {
		/* 7bit, 1 byte */
		cton_util_buffer_putchar(buf, (data & 0x7F));

	} else if (data <= 0x7FF) {
		/* 11bit, 2 byte */
		cton_util_buffer_putchar(buf, ( 0xC0 | (data >> 6) ));
		cton_util_buffer_putchar(buf, ( 0x80 | (data & 0x3F) ));

	} else if (data <= 0xFFFF) {
		/* 3 byte */
		cton_util_buffer_putchar(buf, ( 0xE0 | (data >> 12) ));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 6) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | (data & 0x3F) ));

	} else if (data <= 0x1FFFFF) {
		/* 4 byte */
		cton_util_buffer_putchar(buf, ( 0xF0 | (data >> 18) ));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 12) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 6) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | (data & 0x3F) ));

	} else if (data <= 0x3FFFFFF) {
		/* 5 byte */
		cton_util_buffer_putchar(buf, ( 0xF8 | (data >> 24) ));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 18) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 12) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 6) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | (data & 0x3F) ));

	} else if (data <= 0x7FFFFFFF) {
		/* 6 byte */
		cton_util_buffer_putchar(buf, ( 0xFC | (data >> 30) ));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 24) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 18) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 12) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 6) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | (data & 0x3F) ));

	} else {
		cton_util_buffer_putchar(buf, 0xFE);
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 60) & 0x0F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 54) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 48) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 42) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 36) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 30) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 24) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 18) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 12) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | ((data >> 6) & 0x3F)));
		cton_util_buffer_putchar(buf, ( 0x80 | (data & 0x3F) ));
	}
}


static void
cton_serialize_value(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	cton_bool *b;
	cton_type type;

	type = obj->type;

	if (type == CTON_NULL) {
		cton_util_buffer_putchar(buf, TBON_ID_NULL);

	} else if (type == CTON_BOOL) {
		b = (cton_bool *)cton_object_getvalue(obj);

		if (*b == CTON_TRUE) {
			cton_util_buffer_putchar(buf, TBON_ID_TRUE);
		} else {
			cton_util_buffer_putchar(buf, TBON_ID_FALSE);
		}

	} else if (type == CTON_BINARY) {
		cton_serialize_binary(ctx, buf, obj);

	} else if (type == CTON_STRING) {
		cton_serialize_string(ctx, buf, obj);
		
	} else if (type == CTON_ARRAY) {
		cton_serialize_array(ctx, buf, obj);
		
	} else if (type == CTON_HASH) {
		cton_serialize_hash(ctx, buf, obj);
		
	} else if (type == CTON_INT8 || type == CTON_UINT8 || type == CTON_FLOAT8) {
		cton_serialize_8bit(ctx, buf, obj);

	} else if (type == CTON_INT16 || type == CTON_UINT16 || \
		type == CTON_FLOAT16) {
		cton_serialize_16bit(ctx, buf, obj);
		
	} else if (type == CTON_INT32 || type == CTON_UINT32 || \
		type == CTON_FLOAT32) {
		cton_serialize_32bit(ctx, buf, obj);
		
	} else if (type == CTON_INT64 || type == CTON_UINT64 || \
		type == CTON_FLOAT64) {
		cton_serialize_64bit(ctx, buf, obj);
		
	}
}


static int
cton_serialize_object(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	uint8_t id;

	id = cton_getid(obj->type);

	if (id != TBON_ID_NULL && id != TBON_ID_TRUE) {
		cton_util_buffer_putchar(buf, id);
	}

	cton_serialize_value(ctx, buf, obj);

	return 0;
}


cton_obj *
cton_serialize(cton_ctx *ctx, cton_obj *obj)
{
	cton_buf *buf;
	cton_obj *output;

	buf = cton_util_buffer_create(ctx);

	/* Magic Header */
	cton_util_buffer_puts(buf, "TBON");

	/* Version */
	cton_util_buffer_putchar(buf, 0x00);
	cton_util_buffer_putchar(buf, 0x01);

	cton_serialize_object(ctx, buf, obj);

	output = cton_util_buffer_pack(buf, CTON_STRING);

	cton_util_buffer_destroy(buf);

	return output;
}



static int cton_serialize_string(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	uint64_t length;
	char  *ptr;

	length = cton_string_getlen(obj);
	cton_serialize_vw(ctx, buf, length);

	ptr = cton_string_getptr(obj);

	length -= 1;

	while (length > 0) {
		cton_util_buffer_putchar(buf, *ptr);
		ptr ++;
		length --;
	}

	return 0;
}

static int cton_serialize_binary(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	uint64_t length;
	uint8_t  *ptr;

	length = cton_binary_getlen(obj);
	cton_serialize_vw(ctx, buf, length);

	ptr = cton_binary_getptr(obj);

	length -= 1;

	while (length > 0) {
		cton_util_buffer_putchar(buf, *ptr);
		ptr ++;
		length --;
	}

	return 0;
}

static int cton_serialize_hash_item(cton_ctx *ctx,
	cton_obj *key, cton_obj *value, size_t index, void *buf)
{
	(void) index;
	cton_serialize_object(ctx, buf, key);
	cton_serialize_object(ctx, buf, value);
	return 0;
}

static int cton_serialize_hash(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	uint64_t length;

	length = cton_hash_getlen(obj);
	cton_serialize_vw(ctx, buf, length);

	cton_hash_foreach(obj, (void *)buf, cton_serialize_hash_item);

	return 0;
}

static int cton_serialize_array_item(cton_ctx *ctx,
	cton_obj *arr_item, size_t index, void *buf)
{
	(void) index;
	cton_serialize_value(ctx, buf, arr_item);

	return 0;
}

static int cton_serialize_array_object(cton_ctx *ctx,
	cton_obj *arr_item, size_t index, void *buf)
{
	(void)index;
	cton_serialize_object(ctx, buf, arr_item);
	return 0;
}

static int cton_serialize_array(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
	cton_type type;
	uint64_t length;
	uint8_t id;

	type = cton_array_gettype(obj);
	id = cton_getid(type);
	cton_util_buffer_putchar(buf, id);

	length = cton_array_getlen(obj);
	cton_serialize_vw(ctx, buf, length);

	if (type == CTON_OBJECT) {
		return cton_array_foreach(obj,
			(void *)buf, cton_serialize_array_object);
	} else {
		return cton_array_foreach(obj,
			(void *)buf, cton_serialize_array_item);
	}

	return -1;
}

/*******************************************************************************
 *    Deserialize
 ******************************************************************************/

static cton_obj *
cton_deserialize_object(cton_ctx *ctx, size_t *index, uint8_t *ptr, size_t len);
static cton_obj *
cton_deserialize_value(cton_ctx *ctx,
	size_t *index, uint8_t *ptr, cton_type type, size_t len);

static cton_type cton_get_type_from_id(uint8_t id)
{
	switch (id) {

		case TBON_ID_OBJECT: return CTON_OBJECT;
		case TBON_ID_NULL: return CTON_NULL;
		case TBON_ID_TRUE: return CTON_BOOL;
		case TBON_ID_FALSE: return CTON_BOOL;
		case TBON_ID_BINARY: return CTON_BINARY;
		case TBON_ID_STRING: return CTON_STRING;
		case TBON_ID_ARRAY: return CTON_ARRAY;
		case TBON_ID_HASH: return CTON_HASH;
		case TBON_ID_INT8: return CTON_INT8;
		case TBON_ID_INT16: return CTON_INT16;
		case TBON_ID_INT32: return CTON_INT32;
		case TBON_ID_INT64: return CTON_INT64;
		case TBON_ID_UINT8: return CTON_UINT8;
		case TBON_ID_UINT16: return CTON_UINT16;
		case TBON_ID_UINT32: return CTON_UINT32;
		case TBON_ID_UINT64: return CTON_UINT64;
		case TBON_ID_FLOAT8: return CTON_FLOAT8;
		case TBON_ID_FLOAT16: return CTON_FLOAT16;
		case TBON_ID_FLOAT32: return CTON_FLOAT32;
		case TBON_ID_FLOAT64: return CTON_FLOAT64;
		default:
			return CTON_INVALID;
	}

	return CTON_INVALID;
}

static uint8_t cton_deserialize_vwlen(uint8_t *ptr)
{
	if (*ptr <= 0x7F) {
		/* 7bit, 1 byte */
		return 1;

	} else if (*ptr <= 0xDF) {
		/* 11bit, 2 byte */
		return 2;

	} else if (*ptr <= 0xEF) {
		/* 3 byte */
		return 3;

	} else if (*ptr <= 0xF7) {
		/* 4 byte */
		return 4;

	} else if (*ptr <= 0xFB) {
		/* 5 byte */
		return 5;

	} else if (*ptr <= 0xFD) {
		/* 6 byte */
		return 6;

	} else {
		return 12;
	}
}

static uint64_t cton_deserialize_vw(uint8_t *ptr)
{
	uint64_t data;
	uint8_t  tails;

	tails = 0;

	if (*ptr <= 0x7F) {
		/* 7bit, 1 byte */
		data = *ptr;

	} else if (*ptr <= 0xDF) {
		/* 11bit, 2 byte */
		data = *ptr & 0x1F;
		tails = 1;

	} else if (*ptr <= 0xEF) {
		/* 3 byte */
		data = *ptr & 0x0F;
		tails = 2;

	} else if (*ptr <= 0xF7) {
		/* 4 byte */
		data = *ptr & 0x07;
		tails = 3;

	} else if (*ptr <= 0xFB) {
		/* 5 byte */
		data = *ptr & 0x03;
		tails = 4;

	} else if (*ptr <= 0xFD) {
		/* 6 byte */
		data = *ptr & 0x01;
		tails = 5;

	} else {
		ptr ++;
		data = *ptr & 0x0F;
		tails = 10;
	}

	while (tails > 0) {
		ptr ++;
		data = ((data << 6) | ((*ptr) & 0x3F));
		tails --;
	}

	return data;
}

static uint64_t cton_deserialize_64bit(uint8_t *ptr)
{
	uint64_t data;

	data = (((uint64_t)ptr[0] << 56) | ((uint64_t)ptr[1] << 48) |      \
			((uint64_t)ptr[2] << 40) | ((uint64_t)ptr[3] << 32) |      \
			((uint64_t)ptr[4] << 24) | ((uint64_t)ptr[5] << 16) |      \
			((uint64_t)ptr[6] << 8) | ((uint64_t)ptr[7]) );

	return data;
}

static uint32_t cton_deserialize_32bit(uint8_t *ptr)
{
	uint32_t data;

	data = (((uint32_t)ptr[0] << 24) | ((uint32_t)ptr[1] << 16) |      \
			((uint32_t)ptr[2] << 8) | ((uint32_t)ptr[3]) );

	return data;
}

static uint16_t cton_deserialize_16bit(uint8_t *ptr)
{
	uint16_t data;

	data = (((uint16_t)ptr[0] << 8) | ((uint16_t)ptr[1]) );

	return data;
}

static uint8_t cton_deserialize_8bit(uint8_t *ptr)
{
	return (uint8_t)ptr[0];
}

static cton_obj *
cton_deserialize_string(cton_ctx *ctx,
	size_t *index, uint8_t *ptr, size_t len, cton_type type)
{
	size_t str_len;
	cton_obj *obj;
	uint8_t  *dst;

	if (*index + cton_deserialize_vwlen(ptr) >= len) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	str_len = cton_deserialize_vw(ptr + *index);
	*index += cton_deserialize_vwlen(ptr);

	if (*index + str_len >= len) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	obj = cton_object_create(ctx, type);
	cton_string_setlen(obj, str_len);
	dst = cton_binary_getptr(obj);

	while (str_len > 0) {
		*dst++ = ptr[*index];
		*index += 1;
		str_len -= 1;
	}

	return obj;
}


static cton_obj *
cton_deserialize_hash(cton_ctx *ctx, size_t *index, uint8_t *ptr, size_t len)
{
	size_t hash_len;
	cton_obj *hash;
	cton_obj *key;
	cton_obj *value;

	if (*index + cton_deserialize_vwlen(ptr) >= len) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	hash_len = cton_deserialize_vw(ptr + *index);
	*index +=  cton_deserialize_vwlen(ptr);

	hash = cton_object_create(ctx, CTON_HASH);
	if (hash == NULL) {
		return NULL;
	}

	while (hash_len > 0) {
		key = cton_deserialize_object(ctx, index, ptr, len);
		if (key == NULL) {
			return hash;
		}

		value = cton_deserialize_object(ctx, index, ptr, len);
		if (value == NULL) {
			cton_object_delete(key);
			return hash;
		}

		cton_hash_set(hash, key, value);
		hash_len -= 1;
	}

	return hash;
}


static cton_obj *
cton_deserialize_array(cton_ctx *ctx, size_t *index, uint8_t *ptr, size_t len)
{
	size_t arr_len;
	size_t arr_index;
	cton_type type;
	cton_obj *arr;
	cton_obj *obj;

	if (*index +  cton_deserialize_vwlen(ptr) + 1 >= len) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	type = cton_get_type_from_id(ptr[*index]);
	*index += 1;

	arr_len = cton_deserialize_vw(ptr + *index);
	*index +=  cton_deserialize_vwlen(ptr);

	arr = cton_object_create(ctx, CTON_ARRAY);
	if (arr == NULL) {
		return NULL;
	}
	cton_array_settype(arr, type);
	cton_array_setlen(arr, arr_len);

	if (type == CTON_OBJECT) {

		for (arr_index = 0; arr_index < arr_len; arr_index += 1) {
			obj = cton_deserialize_object(ctx, index, ptr, len);
			cton_array_set(arr, obj, arr_index);
		}

	} else {

		for (arr_index = 0; arr_index < arr_len; arr_index += 1) {

			fprintf(stderr, "%ld, %02x\n", *index - 1, ptr[*index - 1]);
			obj = cton_deserialize_value(ctx, index, ptr, type, len);
			cton_array_set(arr, obj, arr_index);
		}

	}

	return arr;
}

static cton_obj * cton_deserialize_value(cton_ctx *ctx,
	size_t *index, uint8_t *ptr, cton_type type, size_t len)
{
	cton_obj *obj;
	uint8_t  *v8_ptr;
	uint16_t *v16_ptr;
	uint32_t *v32_ptr;
	uint64_t *v64_ptr;

	if (*index >= len) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	switch (type) {
		case CTON_STRING:
		case CTON_BINARY:
			obj = cton_deserialize_string(ctx, index, ptr, len, type);
			break;

		case CTON_ARRAY:
			obj = cton_deserialize_array(ctx, index, ptr, len);
			break;

		case CTON_HASH:
			obj = cton_deserialize_hash(ctx, index, ptr, len);
			break;

		case CTON_INT8:
		case CTON_UINT8:
		case CTON_FLOAT8:
			obj = cton_object_create(ctx, type);
			v8_ptr = cton_object_getvalue(obj);
			*v8_ptr = cton_deserialize_8bit(ptr + *index);
			*index += 1;
			break;

		case CTON_INT16:
		case CTON_UINT16:
		case CTON_FLOAT16:
			obj = cton_object_create(ctx, type);
			v16_ptr = cton_object_getvalue(obj);
			*v16_ptr = cton_deserialize_16bit(ptr + *index);
			*index += 2;
			break;

		case CTON_INT32:
		case CTON_UINT32:
		case CTON_FLOAT32:
			obj = cton_object_create(ctx, type);
			v32_ptr = cton_object_getvalue(obj);
			*v32_ptr = cton_deserialize_32bit(ptr + *index);
			*index += 4;
			break;

		case CTON_INT64:
		case CTON_UINT64:
		case CTON_FLOAT64:
			obj = cton_object_create(ctx, type);
			v64_ptr = cton_object_getvalue(obj);
			*v64_ptr = cton_deserialize_64bit(ptr + *index);
			*index += 8;
			break;

		case CTON_BOOL:
			obj = cton_object_create(ctx, CTON_BOOL);
			if (ptr[*index] == TBON_ID_TRUE) {
				cton_bool_set(obj, CTON_TRUE);
			} else {
				cton_bool_set(obj, CTON_FALSE);
			}
			*index += 1;
			break;

		case CTON_NULL:
			obj = cton_object_create(ctx, CTON_NULL);
			*index += 1;
			break;

		default:
			obj = NULL;
	}

	return obj;
}

static cton_obj *
cton_deserialize_object(cton_ctx *ctx, size_t *index, uint8_t *ptr, size_t len)
{
	cton_type type;

	if (*index >= len) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	type = cton_get_type_from_id(ptr[*index]);
	if (type != CTON_NULL && type != CTON_BOOL) {
		*index += 1;
	}

	if (type == CTON_INVALID) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	return cton_deserialize_value(ctx, index, ptr, type, len);
}

cton_obj *cton_deserialize(cton_ctx *ctx, cton_obj *tbon)
{
	uint8_t *ptr;
	size_t len;
	size_t index;

	len = cton_string_getlen(tbon);
	if (len <= sizeof("TBON01")) {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	ptr = cton_binary_getptr(tbon);

	if (ptr[0] != 'T' || ptr[1] != 'B' || ptr[2] != 'O' || ptr[3] != 'N') {
		cton_seterr(ctx, CTON_ERROR_BROKEN);
		return NULL;
	}

	if (ptr[4] == 0x00 && ptr[5] == 0x01) {

		/* Version 0.1*/
		index = 6;

		return cton_deserialize_object(ctx, &index, ptr, len);

	}

	return NULL;
}

#endif
