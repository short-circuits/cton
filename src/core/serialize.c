#include <ctype.h>
#include <core/cton_core.h>
#include <assert.h>


cton_obj *
tbonv2_serialize(cton_ctx *ctx, cton_obj *obj);

cton_obj *
cton_serialize(cton_ctx *ctx, cton_obj *obj)
{
	return tbonv2_serialize(ctx, obj);
}


cton_obj *
tbonv1_deserialize_object(cton_ctx *ctx, size_t *index, uint8_t *ptr, size_t len);
cton_obj *
tbonv2_deserialize_object(cton_ctx *ctx, size_t *index, uint8_t *ptr, size_t len);


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

		return tbonv1_deserialize_object(ctx, &index, ptr, len);

	}

	return NULL;
}
