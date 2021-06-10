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
 *     +--------+=================+--------+~~~~~~~~~~~~~~~~~+
 *     |  0x5F  |   Base128(N)    |  type  |    N objects    |
 *     +--------+=================+--------+~~~~~~~~~~~~~~~~~+
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
 *       +--------+=================+--------+~~~~~~~~~~~~~~~~~+
 *       |  0x5F  |   Base128(N)    |  0x5F  |    N objects    |
 *       +--------+=================+--------+~~~~~~~~~~~~~~~~~+
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


static void
tbonv2_buffer_put_8BE(cton_buf *buf, uint8_t data);
static void
tbonv2_buffer_put_16BE(cton_buf *buf, uint16_t data);
static void
tbonv2_buffer_put_32BE(cton_buf *buf, uint32_t data);
static void
tbonv2_buffer_put_64BE(cton_buf *buf, uint64_t data);
static void
tbonv2_buffer_put_varint(cton_buf *buf, uint64_t data);

static void
tbonv2_serialize_id_with_length(cton_buf *buf,uint8_t id, uint64_t len);
static void
tbonv2_serialize_bytes(cton_buf *buf, uint8_t *ptr, uint64_t len);
static int
tbonv2_serialize_object(cton_ctx *ctx, cton_buf *buf, cton_obj *obj);
static int
tbonv2_serialize_hash_item(cton_ctx *ctx, cton_obj *key, cton_obj *value,
    size_t index, void *buf);
static int
tbonv2_serialize_array_items(cton_buf *buf, cton_obj *arr, cton_type type);


static int
tbonv2_serialize_array_object(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf);

static void
tbonv2_serialize_numeric_8b(cton_buf *buf, cton_obj *obj);
static void
tbonv2_serialize_numeric_16b(cton_buf *buf, cton_obj *obj);
static void
tbonv2_serialize_numeric_32b(cton_buf *buf, cton_obj *obj);
static void
tbonv2_serialize_numeric_64b(cton_buf *buf, cton_obj *obj);


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

struct tbonv2_callback {
    uint8_t   id;
    void    (*serialize)(cton_buf *, cton_obj *);
};

static const struct tbonv2_callback tbonv2_hook[CTON_TYPE_CNT] = {
    { 0x00, NULL }, /* CTON_INVALID = 0, */
    { 0x00, NULL }, /* CTON_OBJECT  = 1, */
    { 0x01, NULL }, /* CTON_NULL    = 2, */
    { 0x00, NULL }, /* CTON_BOOL    = 3, */
    { 0x00, NULL }, /* CTON_BINARY  = 4, */
    { 0x00, NULL }, /* CTON_STRING  = 5, */
    { 0x00, NULL }, /* CTON_ARRAY   = 6, */
    { 0x00, NULL }, /* CTON_HASH    = 7, */
    { 0x10, tbonv2_serialize_numeric_8b  }, /* CTON_INT8    = 8, */
    { 0x11, tbonv2_serialize_numeric_16b }, /* CTON_INT16   = 9, */
    { 0x12, tbonv2_serialize_numeric_32b }, /* CTON_INT32   = 10 */
    { 0x13, tbonv2_serialize_numeric_64b }, /* CTON_INT64   = 11 */
    { 0x18, tbonv2_serialize_numeric_8b  }, /* CTON_UINT8   = 12 */
    { 0x19, tbonv2_serialize_numeric_16b }, /* CTON_UINT16  = 13 */
    { 0x1A, tbonv2_serialize_numeric_32b }, /* CTON_UINT32  = 14 */
    { 0x1B, tbonv2_serialize_numeric_64b }, /* CTON_UINT64  = 15 */
    { 0x08, tbonv2_serialize_numeric_8b  }, /* CTON_FLOAT8  = 16 */
    { 0x09, tbonv2_serialize_numeric_16b }, /* CTON_FLOAT16 = 17 */
    { 0x0A, tbonv2_serialize_numeric_32b }, /* CTON_FLOAT32 = 18 */
    { 0x0B, tbonv2_serialize_numeric_64b }, /* CTON_FLOAT64 = 19 */
};

static int
tbonv2_serialize_object(cton_ctx *ctx, cton_buf *buf, cton_obj *obj)
{
    cton_type  type;
    cton_type  subtype;

    cton_bool *b;
    uint8_t   *ptr;
    uint64_t   length;

    (void) ctx;

    type = cton_object_gettype(obj);

    if (tbonv2_hook[type].id != 0x00) {
        cton_buffer_putchar(buf, tbonv2_hook[type].id);
    }

    if (tbonv2_hook[type].serialize != NULL) {
        tbonv2_hook[type].serialize(buf, obj);
    }

    if (type == CTON_BOOL) {
        b = (cton_bool *)cton_object_getvalue(obj);

        if (*b == CTON_TRUE) {
            cton_buffer_putchar(buf, 0x02);
        } else {
            cton_buffer_putchar(buf, 0x03);
        }

    } else if (type == CTON_HASH) {
        length = cton_hash_getlen(obj);

        tbonv2_serialize_id_with_length(buf, 0x20, length);

        cton_hash_foreach(obj, (void *)buf, tbonv2_serialize_hash_item);

    } else if (type == CTON_BINARY) {
        length = cton_binary_getlen(obj);

        tbonv2_serialize_id_with_length(buf, 0x80, length);

        ptr = cton_binary_getptr(obj);

        tbonv2_serialize_bytes(buf, ptr, length);

    } else if (type == CTON_STRING) {
        length = cton_string_getlen(obj);

        tbonv2_serialize_id_with_length(buf, 0xA0, length);

        ptr = (uint8_t *)cton_string_getptr(obj);

        tbonv2_serialize_bytes(buf, ptr, length);

    } else if (type == CTON_ARRAY) {
        subtype = cton_array_gettype(obj);
        length  = cton_array_getlen(obj);

        if (subtype == CTON_OBJECT) {
            tbonv2_serialize_id_with_length(buf, 0x40, length);

            cton_array_foreach(obj, (void *)buf, tbonv2_serialize_array_object);

        } else {
            tbonv2_serialize_id_with_length(buf, 0x60, length);

            tbonv2_serialize_array_items(buf, obj, subtype);

        }

    }

    return 0;
}


/*******************************************************************************
 * TYPE SPECIFIC SERIALIZE UTILTY FUNCTIONS
 ******************************************************************************/

static void
tbonv2_serialize_id_with_length(cton_buf *buf, uint8_t id, uint64_t len)
{
    if (len < 0x1F) {
        id += len;
        cton_buffer_putchar(buf, id);
    } else {
        id += 0x1F;
        cton_buffer_putchar(buf, id);
        tbonv2_buffer_put_varint(buf, len);
    }
}

static void
tbonv2_serialize_bytes(cton_buf *buf, uint8_t *ptr, uint64_t len)
{
    while (len > 0) {
        cton_buffer_putchar(buf, *ptr);
        ptr ++;
        len --;
    }
}

static int
tbonv2_serialize_hash_item(cton_ctx *ctx, cton_obj *key, cton_obj *value,
    size_t index, void *buf)
{
    (void) index;
    tbonv2_serialize_object(ctx, buf, key);
    tbonv2_serialize_object(ctx, buf, value);
    return 0;
}

static int
tbonv2_serialize_array_object(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    (void) index;
    tbonv2_serialize_object(ctx, buf, obj);

    return 0;
}

static int
tbonv2_serialize_array_null(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    (void) index;
    (void) ctx;
    (void) obj;

    cton_buffer_putchar(buf, 0x01);

    return 0;
}

static int
tbonv2_serialize_array_bool(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    cton_bool *b;

    (void) index;
    (void) ctx;
    
    b = (cton_bool *)cton_object_getvalue(obj);

    if (*b == CTON_TRUE) {
        cton_buffer_putchar(buf, 0x02);
    } else {
        cton_buffer_putchar(buf, 0x03);
    }

    return 0;
}

static int
tbonv2_serialize_array_string(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint64_t length;
    uint8_t  *ptr;

    (void) index;
    (void) ctx;
    
    length = cton_string_getlen(obj);

    tbonv2_buffer_put_varint(buf, length);

    ptr = (uint8_t *)cton_string_getptr(obj);

    tbonv2_serialize_bytes(buf, ptr, length);

    return 0;
}

static int
tbonv2_serialize_array_binary(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint64_t length;
    uint8_t  *ptr;

    (void) index;
    (void) ctx;
    
    length = cton_binary_getlen(obj);

    tbonv2_buffer_put_varint(buf, length);

    ptr = cton_binary_getptr(obj);

    tbonv2_serialize_bytes(buf, ptr, length);

    return 0;
}

static int
tbonv2_serialize_array_hash(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint64_t length;

    (void) index;
    (void) ctx;
    
    
    length = cton_hash_getlen(obj);

    tbonv2_buffer_put_varint(buf, length);

    cton_hash_foreach(obj, (void *)buf, tbonv2_serialize_hash_item);

    return 0;
}

static int
tbonv2_serialize_array_array(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint64_t length;
    cton_type subtype;

    (void) index;
    (void) ctx;
    
    length = cton_array_getlen(obj);
    subtype = cton_array_gettype(obj);

    tbonv2_buffer_put_varint(buf, length);

    tbonv2_serialize_array_items(buf, obj, subtype);

    return 0;
}

static int
tbonv2_serialize_numarr_8b(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint8_t *data;

    (void) index;
    (void) ctx;

    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_8BE(buf, *data);

    return 0;
}

static int
tbonv2_serialize_numarr_16b(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint16_t *data;

    (void) index;
    (void) ctx;

    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_16BE(buf, *data);

    return 0;
}

static int
tbonv2_serialize_numarr_32b(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint32_t *data;

    (void) index;
    (void) ctx;

    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_32BE(buf, *data);

    return 0;
}

static int
tbonv2_serialize_numarr_64b(cton_ctx *ctx, cton_obj *obj, size_t index,
    void *buf)
{
    uint64_t *data;

    (void) index;
    (void) ctx;

    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_64BE(buf, *data);

    return 0;
}

struct tbonv2_array_callbacks {
    uint8_t   id;
    int     (*callback)(cton_ctx *, cton_obj *, size_t, void *);
};

static const struct tbonv2_array_callbacks tbonv2_arr_hook[CTON_TYPE_CNT] = {
    { 0x00, NULL }, /* CTON_INVALID = 0, */
    { 0x00, NULL }, /* CTON_OBJECT  = 1, */
    { 0x01, tbonv2_serialize_array_null }, /* CTON_NULL    = 2, */
    { 0x02, tbonv2_serialize_array_bool }, /* CTON_BOOL    = 3, */
    { 0x9F, tbonv2_serialize_array_binary }, /* CTON_BINARY  = 4, */
    { 0xBF, tbonv2_serialize_array_string }, /* CTON_STRING  = 5, */
    { 0x5F, tbonv2_serialize_array_array }, /* CTON_ARRAY   = 6, */
    { 0x3F, tbonv2_serialize_array_hash }, /* CTON_HASH    = 7, */
    { 0x10, tbonv2_serialize_numarr_8b  }, /* CTON_INT8    = 8, */
    { 0x11, tbonv2_serialize_numarr_16b }, /* CTON_INT16   = 9, */
    { 0x12, tbonv2_serialize_numarr_32b }, /* CTON_INT32   = 10 */
    { 0x13, tbonv2_serialize_numarr_64b }, /* CTON_INT64   = 11 */
    { 0x18, tbonv2_serialize_numarr_8b  }, /* CTON_UINT8   = 12 */
    { 0x19, tbonv2_serialize_numarr_16b }, /* CTON_UINT16  = 13 */
    { 0x1A, tbonv2_serialize_numarr_32b }, /* CTON_UINT32  = 14 */
    { 0x1B, tbonv2_serialize_numarr_64b }, /* CTON_UINT64  = 15 */
    { 0x08, tbonv2_serialize_numarr_8b  }, /* CTON_FLOAT8  = 16 */
    { 0x09, tbonv2_serialize_numarr_16b }, /* CTON_FLOAT16 = 17 */
    { 0x0A, tbonv2_serialize_numarr_32b }, /* CTON_FLOAT32 = 18 */
    { 0x0B, tbonv2_serialize_numarr_64b }, /* CTON_FLOAT64 = 19 */
};

static int
tbonv2_serialize_array_items(cton_buf *buf, cton_obj *arr, cton_type type)
{
    cton_buffer_putchar(buf, tbonv2_arr_hook[type].id);
    cton_array_foreach(arr, (void *)buf, tbonv2_arr_hook[type].callback);

    return 0;
}

static void
tbonv2_serialize_numeric_8b(cton_buf *buf, cton_obj *obj)
{
    uint8_t *data;
    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_8BE(buf, *data);
}

static void
tbonv2_serialize_numeric_16b(cton_buf *buf, cton_obj *obj)
{
    uint16_t *data;
    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_16BE(buf, *data);
}

static void
tbonv2_serialize_numeric_32b(cton_buf *buf, cton_obj *obj)
{
    uint32_t *data;
    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_32BE(buf, *data);
}

static void
tbonv2_serialize_numeric_64b(cton_buf *buf, cton_obj *obj)
{
    uint64_t *data;
    data = cton_object_getvalue(obj);
    tbonv2_buffer_put_64BE(buf, *data);
}


/*******************************************************************************
 * NUMERIC INPUT / OUTPUT UTILTY FUNCTIONS
 ******************************************************************************/

static void
tbonv2_buffer_put_8BE(cton_buf *buf, uint8_t data)
{
    cton_buffer_putchar(buf, data);
}

static void
tbonv2_buffer_put_16BE(cton_buf *buf, uint16_t data)
{
    cton_buffer_putchar(buf, ((data & 0xFF00) >> 8));
    cton_buffer_putchar(buf,  (data & 0x00FF));
}

static void
tbonv2_buffer_put_32BE(cton_buf *buf, uint32_t data)
{
    cton_buffer_putchar(buf, ((data & 0xFF000000) >> 24));
    cton_buffer_putchar(buf, ((data & 0x00FF0000) >> 16));
    cton_buffer_putchar(buf, ((data & 0x0000FF00) >> 8));
    cton_buffer_putchar(buf,  (data & 0x000000FF));
}

static void
tbonv2_buffer_put_64BE(cton_buf *buf, uint64_t data)
{
    cton_buffer_putchar(buf, ((data & 0xFF00000000000000UL) >> 56));
    cton_buffer_putchar(buf, ((data & 0x00FF000000000000UL) >> 48));
    cton_buffer_putchar(buf, ((data & 0x0000FF0000000000UL) >> 40));
    cton_buffer_putchar(buf, ((data & 0x000000FF00000000UL) >> 32));
    cton_buffer_putchar(buf, ((data & 0x00000000FF000000UL) >> 24));
    cton_buffer_putchar(buf, ((data & 0x0000000000FF0000UL) >> 16));
    cton_buffer_putchar(buf, ((data & 0x000000000000FF00UL) >> 8));
    cton_buffer_putchar(buf,  (data & 0x00000000000000FFUL));
}

static void
tbonv2_buffer_put_varint(cton_buf *buf, uint64_t data)
{
    uint8_t byte;

    while (data >= 0x80) {
        byte = data % 0x80 + 0x80;
        data = data >> 7;
        cton_buffer_putchar(buf, byte);
    }

    byte = data % 0x80;

    cton_buffer_putchar(buf, byte);
}

