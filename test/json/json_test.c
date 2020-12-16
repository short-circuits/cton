#include <cton.h>

int main(int argc, const char **argv)
{
	cton_ctx *ctx;
	cton_obj *data;
	cton_obj *out;

	if (argc == 1) {
		printf("Usage: %s [filename]\n", argv[0]);
	}

	ctx = cton_init(NULL);
	data = cton_util_readfile(ctx, argv[1]);

	cton_json_parse(ctx, 
		cton_str_getptr(ctx, data), cton_str_getlen(ctx, data));

	out = cton_json_stringify(ctx);

	printf("%s\n", cton_str_getptr(ctx, out));

	return 0;
}