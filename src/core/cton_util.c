/*******************************************************************************
 * CTON util functions
 * 
 ******************************************************************************/

static size_t cton_util_align(size_t size, size_t align)
{
    size_t remain;
    size_t aligned;

    remain = size % align;
    aligned = size ^ remain;
    return remain > 0 ? aligned + align : aligned ;
}

static void * cton_util_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

static void * cton_util_memset(void *b, int c, size_t len)
{
	return memset(b, c, len);
}

int cton_util_strcmp(cton_obj *s1, cton_obj *s2)
{
	size_t len_s1;
	size_t len_s2;
	size_t len_cmp;

	volatile int ret;

	len_s1 = s1->payload.str.used;
	len_s2 = s2->payload.str.used;

	len_cmp = len_s1 < len_s2 ? len_s1 : len_s2;

	ret = 0;
	ret = strncmp((char *)s1->payload.str.ptr, 
		          (char *)s2->payload.str.ptr, len_cmp - 1);

	if (ret != 0) {
		return ret;
	}

	if (len_s1 == len_s2) {
		return 0;
	} else if (len_s1 < len_s2) {
		return s2->payload.str.ptr[len_cmp];
	} else {
		return s1->payload.str.ptr[len_cmp];
	}
}

cton_obj *cton_util_readfile(cton_ctx *ctx, const char *path)
{
	cton_obj *data;
	FILE     *fp;
	size_t    len;
	uint8_t  *ptr;

	fp = fopen(path, "rb");
	if (fp == NULL) {
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data = cton_object_create(ctx, CTON_BINARY);
	cton_str_setlen(ctx, data, len);

	ptr = cton_str_getptr(ctx, data);
	fread(ptr, len, 1, fp);
	fclose(fp);

	return data;
}

int cton_util_writefile(cton_ctx *ctx, cton_obj* obj, const char *path)
{
	FILE     *fp;
	size_t    len;
	uint8_t  *ptr;

	fp = fopen(path, "wb");
	if (fp == NULL) {
		return -1;
	}

	len = cton_str_getlen(ctx, obj);
	ptr = cton_str_getptr(ctx, obj);
	fwrite(ptr, len, 1, fp);
	fclose(fp);

	return 0;
}
