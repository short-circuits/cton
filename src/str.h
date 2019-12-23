/* Copyright (c) 2019 Lee Yeonji <yeonji@ieee.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _UOL_STR_H_
#define _UOL_STR_H_ 1

#include <stdint.h>
#include <stdlib.h>

typedef struct uol_str_s uol_str_t;

struct uol_str_s {
	size_t len;
#ifdef UOL_STR_SUPERALLOC
	size_t used;
#endif
	uint8_t *ptr;
};

uol_str_t *uol_str_new(const char* str, size_t len);

void *uol_memchr(const uol_str_t *s, int c, size_t n);
int   uol_memcmp(const uol_str_t *str1, const uol_str_t *str2, size_t n);
void *uol_memcpy(uol_str_t *restrict dst, \
			const uol_str_t *restrict src, size_t n);

#if 0

uol_memicmp1();
uol_memmove();
uol_memset();
uol_strcat();
uol_strchr();

#endif

int uol_strcmp(const uol_str_t *s1, const uol_str_t *s2);

#if 0
uol_strcmpi1();
uol_strcoll();
uol_strcpy();
uol_strcspn();
uol_strdup1();
uol_strerror();
uol_stricmp1();
uol_strlen();
uol_strncat();
uol_strncmp();
uol_strncpy();
uol_strnicmp1();
uol_strnset1();
uol_strpbrk();
uol_strrchr();
uol_strset1();
uol_strspn();
uol_strstr();
uol_strtok();
uol_strtok_r();
uol_strxfrm();

#endif

#endif /* _UOL_STR_H_ */

