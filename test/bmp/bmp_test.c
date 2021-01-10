#include <cton.h>
#include <cton_bmp.h>
#include <cton_json.h>

int main(int argc, const char **argv)
{
	cton_ctx *ctx;
	cton_obj *bmp;
	cton_obj *data;
	cton_obj *json;

	if (argc == 1) {
		printf("Usage: %s [string]\n", argv[0]);
		return -1;
	}

	ctx = cton_init(NULL);
	bmp = cton_util_readfile(ctx, argv[1]);
	data = cton_bmp_parse(ctx, bmp);
	json = cton_json_stringify(ctx, data);

	printf("%s\n", cton_string_getptr(ctx, json));
	
	return 0;
}