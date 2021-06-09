#include <cton.h>

int main(int argc, const char **argv)
{
	cton_ctx *ctx;
	cton_obj *json;
	cton_obj *data;
	cton_obj *out;

	if (argc <= 2) {
		printf("Usage: %s [input filename] [output filename]\n", argv[0]);
	}

	ctx = cton_init(NULL);
	json = cton_util_readfile(ctx, argv[1]);

	data = cton_json_parse(ctx, json);

	out = cton_serialize(ctx, data);

	cton_util_writefile(out, argv[2]);

	return 0;
}
