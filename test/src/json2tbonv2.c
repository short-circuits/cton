#include <cton.h>

int main(int argc, const char **argv)
{
	cton_ctx *ctx;
	cton_obj *json;
	cton_obj *data;
	cton_obj *out;

	if (argc == 1) {
		printf("Usage: %s [filename]\n", argv[0]);
	}

	ctx = cton_init(NULL);
	json = cton_util_readfile(ctx, argv[1]);

	data = cton_json_parse(ctx, json);

	out = cton_serialize(ctx, data);

	printf("%s\n", cton_string_getptr(out));

	return 0;
}
