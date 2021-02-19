#include <cton.h>
#include <string.h>
#include <assert.h>

cton_obj *cton_array_push(cton_obj *arr, cton_obj *obj)
{
	size_t len;
	len = cton_array_getlen(arr);
	cton_array_setlen(arr, len + 1);
	cton_array_set(arr, obj, len + 1);
	return arr;
}

int markdown_line2ast_callback(cton_ctx *ctx, cton_obj *obj, size_t index, void *rctx)
{
	cton_obj *ast;
	cton_obj *item;

	ast = rctx;

	item = cton_object_create(ctx, CTON_HASH);
	
	cton_hash_set(item, cton_string(ctx, "class"), cton_string(ctx, "par"));
	cton_hash_set(item, cton_string(ctx, "raw"), obj);

	cton_array_set(ast, item, index);

	return 0;
}

cton_obj *markdown_line2ast(cton_ctx *ctx, cton_obj *md)
{
	cton_obj *ast;

	ast = cton_object_create(ctx, CTON_ARRAY);
	cton_array_settype(ast, CTON_HASH);
	cton_array_setlen(ast, cton_array_getlen(md));

	cton_array_foreach(md, ast, markdown_line2ast_callback);

	return ast;
}

int markdown_detect_blocks(cton_ctx *ctx, cton_obj *ast_item, size_t line_no, void *rctx)
{
	cton_obj *content;
	cton_obj *data;
	char *ch;
	char mark;
	size_t len;
	size_t index;
	size_t indent;
	size_t level;

	(void) line_no;
	(void) rctx;

	content = cton_hash_sget(ast_item, "raw");
	len = cton_string_getlen(content);
	ch  = cton_string_getptr(content);

	/* Detect space line */
	if (len <= sizeof("")) {
		cton_hash_set(ast_item, cton_string(ctx, "class"), cton_string(ctx, "newline"));
		return 0;
	}

	index = 0;
	indent = 0;

	/* skip white spaces */
	while (index < len) {
		if (ch[index] == ' ') {
			indent += 1;
		} else if (ch[index] == '\t') {
			indent += 4 - indent % 4;
		} else {
			break;
		}
		index += 1;
	}

	data = cton_object_create(ctx, CTON_FLOAT64);
	*(double*)cton_object_getvalue(data) = (double)indent;
	cton_hash_set(ast_item, cton_string(ctx, "indent"), data);

	if (indent < 4) {

		/* detect bar */
		if (ch[index] == '-' || ch[index] == '_' || ch[index] == '*' || ch[index] == '=') {
			level = 0;
			mark = ch[index];

			while (index < len) {
				if (ch[index] == mark) {
					level += 1;
				} else if (ch[index] == ' ' || ch[index] == '\t') {
					;
				} else {
					break;
				}
				index += 1;
			}

			if ((level >= 3) && (len - index == 1)) {
				switch (mark) {
					case '_': data = cton_string(ctx, "_"); break;
					case '-': data = cton_string(ctx, "-"); break;
					case '*': data = cton_string(ctx, "*"); break;
					case '=': data = cton_string(ctx, "="); break;
					default: data = cton_string(ctx, "-");
				}
				cton_hash_set(ast_item, cton_string(ctx, "class"), cton_string(ctx, "bar"));
				cton_hash_set(ast_item, cton_string(ctx, "bar_style"), data);
				return 0;
			}
		}

		if (ch[index] == '#') {
			/* detect ATX header*/

			level = 0;

			while (index < len) {
				if (ch[index] == '#') {
					level += 1;
				} else if (ch[index] == ' ' || ch[index] == '\t') {
					index += 1;
					break;
				} else {
					level = 0;
					break;
				}
				index += 1;
			}

			if (level > 0) {
				cton_hash_set(ast_item, cton_string(ctx, "class"), cton_string(ctx, "header"));

				data = cton_object_create(ctx, CTON_FLOAT64);
				*(double*)cton_object_getvalue(data) = (double)level;
				cton_hash_set(ast_item, cton_string(ctx, "level"), data);
			}

		} else if (ch[index] == '>') {
			/* detect quote*/

			level = 0;

			while (index < len) {

				if (ch[index] == '>') {
					level += 1;
				} else if (ch[index] == ' ' || ch[index] == '\t') {
					;
				} else {
					break;
				}
				index += 1;
				fprintf(stderr, "I'm here: %c\n", ch[index]);
			}

			if (level > 0) {
				cton_hash_set(ast_item, cton_string(ctx, "class"), cton_string(ctx, "quote"));

				data = cton_object_create(ctx, CTON_FLOAT64);
				*(double*)cton_object_getvalue(data) = (double)level;
				cton_hash_set(ast_item, cton_string(ctx, "level"), data);
			}

		}

	} /* if (indent < 4) */


	data = cton_object_create(ctx, CTON_STRING);
	cton_string_setlen(data, len - index);
	memcpy(cton_string_getptr(data), &ch[index], len - index);
	cton_hash_set(ast_item, cton_string(ctx, "text"), data);

	return 0;
}

int markdown_detect_inlines(cton_ctx *ctx, cton_obj *ast_item, size_t line_no, void *rctx)
{

	cton_obj *content;
	cton_obj *tokens;
#if 0
	cton_obj *token;
	cton_obj *data;
	#endif
	char *ch;
	size_t len;
	size_t index;

	(void) line_no;
	(void) rctx;

	content = cton_hash_sget(ast_item, "text");

	if (content == NULL) {
		return 0;
	}

	len = cton_string_getlen(content);
	ch  = cton_string_getptr(content);

	tokens = cton_object_create(ctx, CTON_ARRAY);
	cton_array_settype(tokens, CTON_OBJECT);

	index = 0;
	while (index < len) {
		index += 1;
	}

	cton_hash_set(ast_item, cton_string(ctx, "sub"), tokens);
	
	return 0;
}

cton_obj *markdown_parse(cton_ctx *ctx, cton_obj *md)
{
	cton_obj *lines;
	cton_obj *ast;

	lines = cton_util_linesplit(ctx, md);
	ast   = markdown_line2ast(ctx, lines);
	cton_object_delete(lines);

	cton_array_foreach(ast, NULL, markdown_detect_blocks);
	cton_array_foreach(ast, NULL, markdown_detect_inlines);

	return ast;
}

int main(int argc, const char **argv)
{
	cton_ctx *ctx;
	cton_obj *markdown;
	cton_obj *parsed;
	cton_obj *json;

	(void) argc;

	ctx = cton_init(NULL);

	markdown = cton_util_readfile(ctx, argv[1]);
	assert(markdown != NULL);

	parsed = markdown_parse(ctx, markdown);

	json = cton_json_stringify(ctx, parsed);

	cton_util_writefile(json, argv[2]);

	return 0;
}
