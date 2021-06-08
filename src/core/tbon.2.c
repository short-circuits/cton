/*******************************************************************************
 * TBONv2 Serialize & Deserialize functions
 *******************************************************************************
 * TBONv2 Specifation (abstract)
 * =============================
 * 
 * Types
 * -----
 * NULL
 * BOOL (TRUE/FALSE)
 * Signed Integer (INT8/INT16/INT32/INT64)
 * Unsigned Integer (UINT8/UINT16/UINT32/UINT64)
 * Float (FLOAT16/FLOAT32/FLOAT64/FLOAT128)
 * Array
 * Dictionary (aka Key-Value store)
 * Raw (CSTRING/BINARY)
 * 
 * Format
 * ------
 * Magic Header: 0x54 0x42 0x4F 0x4E
 * Version: 0x00 0x02
 *
 * Type Tags:
 * 
 * | Type       |  Binary  | Hex         |
 * |:-----------|:--------:|:------------|
 * | RESERVED   | 00000000 | 0x00        |
 * | NULL       | 00000001 | 0x01        |
 * | Bool False | 00000010 | 0x02        |
 * | Bool True  | 00000011 | 0x03        |
 * | RESERVED   | 000001xx | 0x04 - 0x07 |
 * | Float      | 00001bbb | 0x08 - 0x0F |
 * | Integer    | 00010bbb | 0x10 - 0x17 |
 * | Unsigned   | 00011bbb | 0x18 - 0x1F |
 * | Map        | 001sssss | 0x20 - 0x3F |
 * | Array      | 010sssss | 0x40 - 0x5F |
 * | ArrayCx    | 011sssss | 0x60 - 0x7F |
 * | Binary     | 100sssss | 0x80 - 0x9F |
 * | String     | 101sssss | 0xA0 - 0xBF |
 * | RESERVED   | 11xxxxxx | 0xC0 - 0xFF |
 * 
 * Base 128 Varints Encoding:
 *     TBONv2 use Base128 to encode variable length integers, which is same with
 *     Google ProtoBuf. The below instruments comes from google website.
 *
 *     To understand your simple protocol buffer encoding, you first need to
 *     understand varints. Varints are a method of serializing integers using
 *     one or more bytes. Smaller numbers take a smaller number of bytes.
 *
 *     Each byte in a varint, except the last byte, has the most significant
 *     bit (msb) set – this indicates that there are further bytes to come. The
 *     lower 7 bits of each byte are used to store the two's complement
 *     representation of the number in groups of 7 bits, least significant group
 *     first.
 *
 *     So, for example, here is the number 1 – it's a single byte, so the msb is
 *     not set:
 *
 *     0000 0001
 *
 *     And here is 300 – this is a bit more complicated:
 *
 *     1010 1100 0000 0010
 *     How do you figure out that this is 300? First you drop the msb from each
 *     byte, as this is just there to tell us whether we've reached the end of
 *     the number (as you can see, it's set in the first byte as there is more
 *     than one byte in the varint):
 *
 *     1010 1100 0000 0010
 *     → 010 1100  000 0010
 *     You reverse the two groups of 7 bits because, as you remember, varints
 *     store numbers with the least significant group first. Then you
 *     concatenate them to get your final value:
 *
 *     000 0010  010 1100
 *     →  000 0010 ++ 010 1100
 *     →  100101100
 *     →  256 + 32 + 8 + 4 = 300
 *
 * 
 * NULL object:
 *     As single byte of `0x01`
 *     +--------+
 *     |  0xc0  |
 *     +--------+
 *
 * BOOL object:
 *     Single byte of `0x02` for false
 *     +--------+
 *     |  0x02  |
 *     +--------+
 *     Or `0x03` for true.
 *     +--------+
 *     |  0xc3  |
 *     +--------+
 *
 * NUMERIC object:
 *     Numeric object type contains a `bbb` area, which stands for bits as the
 *     table below:
 *
 *         | bbb |  bits    |
 *         |:----|:--------:|
 *         | 000 | 8        |
 *         | 001 | 16       |
 *         | 010 | 32       |
 *         | 011 | 64       |
 *         | 100 | 128      |
 *         | 101 | RESERVED |
 *         | 110 | RESERVED |
 *         | 111 | RESERVED |
 *
 *     For example, `0x12 (00010010, xxx=010)` stands for 32-bit signed int *
 *
 *     FLOAT8 is not offered as it is not an IEEE754 standard float.
 *
 *     FLOAT16 stores a 16-bit big-endian signed float point number
 *     +--------+--------+--------+
 *     |  0x09  |XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+
 *
 *     FLOAT32 stores a 32-bit big-endian signed float point number
 *     +--------+--------+--------+--------+--------+
 *     |  0x0A  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+
 *
 *     FLOAT64 stores a 64-bit big-endian signed float point number
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |  0x0B  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |XXXXXXXX|
 *     +--------+
 *
 *     FLOAT128 stores a 128-bit big-endian signed float point number
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |  0x0C  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |XXXXXXXX|
 *     +--------+
 *
 *     INT8 stores a 8-bit signed integer
 *     +--------+--------+
 *     |  0x10  |XXXXXXXX|
 *     +--------+--------+
 *
 *     INT16 stores a 16-bit big-endian signed integer
 *     +--------+--------+--------+
 *     |  0x11  |XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+
 *
 *     INT32 stores a 32-bit big-endian signed integer
 *     +--------+--------+--------+--------+--------+
 *     |  0x12  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+
 *
 *     INT64 stores a 64-bit big-endian signed integer
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |  0x13  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |XXXXXXXX|
 *     +--------+
 *
 *     UINT8 stores a 8-bit unsigned integer
 *     +--------+--------+
 *     |  0x18  |XXXXXXXX|
 *     +--------+--------+
 *
 *     UINT16 stores a 16-bit big-endian unsigned integer
 *     +--------+--------+--------+
 *     |  0x19  |XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+
 *
 *     UINT32 stores a 32-bit big-endian unsigned integer
 *     +--------+--------+--------+--------+--------+
 *     |  0x1A  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+
 *
 *     UINT64 stores a 64-bit big-endian unsigned integer
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |  0x1B  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
 *     +--------+--------+--------+--------+--------+--------+--------+--------+
 *     |XXXXXXXX|
 *     +--------+
 *
 *     128-bit integer and unsigned integer is reseved as almost archtecture not
 *     implement it.
 * 
 * MAP object:
 *     Map format family stores a sequence of key-value pairs in 1 ~ 11 bytes of
 *     extra bytes in addition to the key-value pairs.
 * 
 *     To stores a map whose length is upto 30 elements
 *     +--------+~~~~~~~~~~~~~~~~~+
 *     |001XXXXX|   N*2 objects   |
 *     +--------+~~~~~~~~~~~~~~~~~+
 *     Where XXXXX is a 5-bit unsigned integer which represents N
 *
 *     For a map whose length is upto (2^64)-1 elements
 *     +--------+=================+~~~~~~~~~~~~~~~~~+
 *     |  0x3F  |   Base128(N)    |   N*2 objects   |
 *     +--------+=================+~~~~~~~~~~~~~~~~~+
 * 
 * ARRAY object:
 *     Array object can be splited into two sub-type.
 * 
 *     To stores a map whose length is upto 30 elements
 *     +--------+~~~~~~~~~~~~~~~~~+
 *     |011XXXXX|    N objects    |
 *     +--------+~~~~~~~~~~~~~~~~~+
 *     Where XXXXX is a 5-bit unsigned integer which represents N
 *
 *     For a map whose length is upto (2^64)-1 elements
 *     +--------+=================+~~~~~~~~~~~~~~~~~+
 *     |  0x7F  |   Base128(N)    |    N objects    |
 *     +--------+=================+~~~~~~~~~~~~~~~~~+
 *
 *     For Array to save elements in same type, There are an shortcuts to omit
 *     the type contains in "N objects area"
 * 
 *     To stores a map whose length is upto 30 elements with the same type
 *     +--------+--------+~~~~~~~~~~~~~~~~~+
 *     |010XXXXX|  type  |    N objects    |
 *     +--------+--------+~~~~~~~~~~~~~~~~~+
 *     Where XXXXX is a 5-bit unsigned integer which represents N
 *
 *     For a map whose length is upto (2^64)-1 elements with the same type
 *     +--------+--------+=================+~~~~~~~~~~~~~~~~~+
 *     |  0x5F  |  type  |   Base128(N)    |    N objects    |
 *     +--------+--------+=================+~~~~~~~~~~~~~~~~~+
 * 
 *     * When type is NULL(0x01), Bool(0x02), or Numeric(0x08 ~ 0x1F), the
 *       "N objects" area only saves the data without extra attachments.
 *       For example, when type is 0x02, data area will just save a sequence of
 *       0x02 and 0x03 stands FALSE or TRUE.
 *       For another example, when type is 0x10, the data area will just
 *       contains a sequence of INT8 data (the XXXXXXXX parts in NUMERIC object
 *       chapter).
 *     * When type is MAP, ARRAY, STRING or BINARY, the type should be set to
 *       the tag with the max length, and "N objects" area stores the data
 *       without type tag.
 *       For example, ARRAY of ARRAY should be:
 *       +--------+--------+=================+~~~~~~~~~~~~~~~~~+
 *       |  0x5F  |  0x5F  |   Base128(N)    |    N objects    |
 *       +--------+--------+=================+~~~~~~~~~~~~~~~~~+
 *       Which N objects area is like
 *       +--------+=================+~~~~~~~~~~~~~~~~~+
 *       |  type  |   Base128(N)    |    N objects    |
 *       +--------+=================+~~~~~~~~~~~~~~~~~+
 *     * I think the object like ARRAY of ARRAY should not be used, but it is
 *       allowed by the defination. This implementation allows to parse the
 *       ARRAY of (ARRYA of ARRAY), but will not generate it.
 * 
 * STRING and BINARY object
 *     STRING and BINARY format family stores a byte array in 1 ~ 11 bytes of
 *     extra bytes in addition to the byte array.
 * 
 *     To stores a byte array whose length is upto 30 bytes
 *     +--------+~~~~~~~~+
 *     |100XXXXX|  data  |
 *     +--------+~~~~~~~~+
 *     Or
 *     +--------+~~~~~~~~+
 *     |101XXXXX|  data  |
 *     +--------+~~~~~~~~+
 *     Where XXXXX is a 5-bit unsigned integer which represents N
 *
 *     For a byte array whose length is upto (2^64)-1 bytes
 *     +--------+=================+~~~~~~~~+
 *     |  0x9F  |   Base128(N)    |  data  |
 *     +--------+=================+~~~~~~~~+
 *     Or
 *     +--------+=================+~~~~~~~~+
 *     |  0xBF  |   Base128(N)    |  data  |
 *     +--------+=================+~~~~~~~~+
 * 
 *     * STRING object stands for the byte array with an ending '\0'.
 *     * The ending '\0' is not counted in N. Which means the really memory
 *       usage of STRING object is `N + 1`, and the parser should add an ending
 *       `\0` manually.
 * 
 ******************************************************************************/
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
tbonv2_serialize(cton_ctx *ctx, cton_obj *obj)
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

static void
tbonv2_buffer_put_8BE(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint8_t data;

	(void)ctx;

	data = *(uint8_t *)ptr;
	cton_buffer_putchar(buf, data);
}


static void
tbonv2_buffer_put_16BE(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint16_t data;

	(void)ctx;

	data = *(uint16_t *)ptr;
	cton_buffer_putchar(buf, ((data & 0xFF00) >> 8));
	cton_buffer_putchar(buf,  (data & 0x00FF));
}


static void
tbonv2_buffer_put_32BE(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint32_t data;

	(void)ctx;

	data = *(uint32_t *)ptr;
	cton_buffer_putchar(buf, ((data & 0xFF000000) >> 24));
	cton_buffer_putchar(buf, ((data & 0x00FF0000) >> 16));
	cton_buffer_putchar(buf, ((data & 0x0000FF00) >> 8));
	cton_buffer_putchar(buf,  (data & 0x000000FF));
}


static void
tbonv2_buffer_put_64BE(cton_ctx *ctx, cton_buf *buf, void *ptr)
{
	uint64_t data;

	(void)ctx;

	data = *(uint64_t *)ptr;
	cton_buffer_putchar(buf, ((data & 0xFF00000000000000UL) >> 56));
	cton_buffer_putchar(buf, ((data & 0x00FF000000000000UL) >> 48));
	cton_buffer_putchar(buf, ((data & 0x0000FF0000000000UL) >> 40));
	cton_buffer_putchar(buf, ((data & 0x000000FF00000000UL) >> 32));
	cton_buffer_putchar(buf, ((data & 0x00000000FF000000UL) >> 24));
	cton_buffer_putchar(buf, ((data & 0x0000000000FF0000UL) >> 16));
	cton_buffer_putchar(buf, ((data & 0x000000000000FF00UL) >> 8));
	cton_buffer_putchar(buf,  (data & 0x00000000000000FFUL));
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
