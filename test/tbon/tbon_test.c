#include <cton.h>

int main(int argc, const char **argv)
{
	cton_ctx *ctx;
	cton_obj *tbon;
	cton_obj *data;
	cton_obj *ton;

	if (argc == 1) {
		printf("Usage: %s [string]\n", argv[0]);
		return -1;
	}

	ctx = cton_init(NULL);
	tbon = cton_util_readfile(ctx, argv[1]);

	data = cton_deserialize(ctx, tbon);

	ton = cton_stringify(ctx, data);

	printf("%s\n", cton_string_getptr(ton));

	
	return 0;
}