#include <cton.h>

static cton_obj *cton_bmp_read_header(cton_ctx *ctx, cton_obj *bmp);
static cton_obj *cton_bmp_read_dib(cton_ctx *ctx, cton_obj *bmp);
static cton_obj *cton_bmp_read_ciexyztriple(cton_ctx *ctx, uint8_t *ptr);

cton_obj *cton_bmp_parse(cton_ctx *ctx, cton_obj *bmp)
{
	cton_obj *result;
	cton_obj *bmp_data;

	result = cton_object_create(ctx, CTON_HASH);

	cton_hash_set(result, cton_string(ctx, "Domain"),
		cton_string(ctx, "com.microsoft.bmp"));

	bmp_data = cton_bmp_read_header(ctx, bmp);

	cton_hash_set(result,
		cton_string(ctx, "BITMAPFILEHEADER"), bmp_data);

	bmp_data = cton_bmp_read_dib(ctx, bmp);

	cton_hash_set(result,
		cton_string(ctx, "BITMAPINFOHEADER"), bmp_data);

	return result;
}

#define CTON_GET_INT16(ptr)                                                   \
    (int16_t)((uint16_t) (ptr)[0] | ((uint16_t) (ptr)[1] << 8))

#define CTON_GET_INT32(ptr)                                                   \
    (int32_t)((uint32_t) (ptr)[0] | ((uint32_t) (ptr)[1] << 8) |              \
    ((uint32_t) (ptr)[2] << 16) | ((uint32_t) (ptr)[3] << 24))

static cton_obj *cton_bmp_read_header(cton_ctx *ctx, cton_obj *bmp)
{
	cton_obj *result;
	cton_obj *bmp_data;

	uint8_t *src;
	uint8_t *dst;

	result = cton_object_create(ctx, CTON_HASH);
	src    = cton_binary_getptr(bmp);


	bmp_data = cton_object_create(ctx, CTON_STRING);
	cton_string_setlen(bmp_data, 3);

	dst = cton_binary_getptr(bmp_data);
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = 0;

	cton_hash_set(result, cton_string(ctx, "bfType"), bmp_data);


	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[2]);
	cton_hash_set(result, cton_string(ctx, "bfSize"), bmp_data);


	bmp_data = cton_object_create(ctx, CTON_INT16);
	*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[6]);
	cton_hash_set(result, cton_string(ctx, "bfReserved1"), bmp_data);


	bmp_data = cton_object_create(ctx, CTON_INT16);
	*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[8]);
	cton_hash_set(result, cton_string(ctx, "bfReserved2"), bmp_data);


	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[10]);
	cton_hash_set(result, cton_string(ctx, "bfOffBits"), bmp_data);

	return result;
}

static cton_obj *cton_bmp_read_dib(cton_ctx *ctx, cton_obj *bmp)
{
	cton_obj *result;
	cton_obj *bmp_data;

	int32_t size;
	int32_t index;

	uint8_t *src;

	result = cton_object_create(ctx, CTON_HASH);
	src    = cton_binary_getptr(bmp);

	index = 14;
                               

	bmp_data = cton_object_create(ctx, CTON_INT32);
	size = CTON_GET_INT32(&src[index]);
	*(int32_t *)cton_object_getvalue(bmp_data) = size;
	cton_hash_set(result, cton_string(ctx, "biSize"), bmp_data);
	index += 4;

	if (size == 12) {
		/* BITMAPCOREHEADER */

		bmp_data = cton_object_create(ctx, CTON_INT16);
		*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT16(&src[index]);
		cton_hash_set(result, cton_string(ctx, "Width"), bmp_data);
		index += 2;

		bmp_data = cton_object_create(ctx, CTON_INT16);
		*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT16(&src[index]);
		cton_hash_set(result, cton_string(ctx, "Height"), bmp_data);
		index += 2;

		bmp_data = cton_object_create(ctx, CTON_INT16);
		*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT16(&src[index]);
		cton_hash_set(result, cton_string(ctx, "Planes"), bmp_data);
		index += 2;

		bmp_data = cton_object_create(ctx, CTON_INT16);
		*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT16(&src[index]);
		cton_hash_set(result, cton_string(ctx, "BitCount"), bmp_data);
		index += 2;

		return result;

	}   /* BITMAPCOREHEADER */

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "Width"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "Height"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT16);
	*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT16(&src[index]);
	cton_hash_set(result, cton_string(ctx, "Planes"), bmp_data);
	index += 2;

	bmp_data = cton_object_create(ctx, CTON_INT16);
	*(int16_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT16(&src[index]);
	cton_hash_set(result, cton_string(ctx, "BitCount"), bmp_data);
	index += 2;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "Compression"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "SizeImage"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "XPelsPerMeter"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "YPelsPerMeter"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "ClrUsed"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "ClrImportant"), bmp_data);
	index += 4;

	if (size == 40) { return result; }

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "RedMask"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "GreenMask"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "BlueMask"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "AlphaMask"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "CSType"), bmp_data);
	index += 4;

	cton_hash_set(result, cton_string(ctx, "Endpoints"),
		cton_bmp_read_ciexyztriple(ctx, &src[index]));
	index += 36;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "GammaRed"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "GammaGreen"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "GammaBlue"), bmp_data);
	index += 4;

	if (size == 108) { return result; }

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "Intent"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "ProfileData"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "ProfileSize"), bmp_data);
	index += 4;

	bmp_data = cton_object_create(ctx, CTON_INT32);
	*(int32_t *)cton_object_getvalue(bmp_data) = CTON_GET_INT32(&src[index]);
	cton_hash_set(result, cton_string(ctx, "Reserved"), bmp_data);
	index += 4;

	return result;
}

static cton_obj *cton_bmp_read_ciexyztriple(cton_ctx *ctx, uint8_t *ptr)
{
	cton_obj *result;
	cton_obj *ciexyz;
	cton_obj *data;

	int cnt;

	result = cton_object_create(ctx, CTON_HASH);

	for (cnt = 0; cnt < 3; cnt ++) {

		ciexyz = cton_object_create(ctx, CTON_HASH);

		data = cton_object_create(ctx, CTON_INT32);
		*(int32_t *)cton_object_getvalue(data) = CTON_GET_INT32(ptr);
		cton_hash_set(ciexyz, cton_string(ctx, "ciexyzX"), data);
		ptr += 4;

		data = cton_object_create(ctx, CTON_INT32);
		*(int32_t *)cton_object_getvalue(data) = CTON_GET_INT32(ptr);
		cton_hash_set(ciexyz, cton_string(ctx, "ciexyzY"), data);
		ptr += 4;

		data = cton_object_create(ctx, CTON_INT32);
		*(int32_t *)cton_object_getvalue(data) = CTON_GET_INT32(ptr);
		cton_hash_set(ciexyz, cton_string(ctx, "ciexyzZ"), data);
		ptr += 4;

		switch (cnt) {
			case 0:
				cton_hash_set(result,
					cton_string(ctx, "ciexyzRed"), ciexyz);
				break;
			case 1:
				cton_hash_set(result,
					cton_string(ctx, "ciexyzGreen"), ciexyz);
				break;
			case 2:
				cton_hash_set(result,
					cton_string(ctx, "ciexyzBlue"), ciexyz);
				break;
			default: ;
		}
	}

	return result;
}

cton_obj * cton_bmp_serialize(cton_ctx *ctx, cton_obj *obj);
