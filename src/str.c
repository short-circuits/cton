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

#include "str.h"
#include <string.h>

#define uol_len(s) ((s)->len)
#define uol_dat(s) ((s)->ptr)

#define min(x, y) (((x)<(y))?(x):(y))


/**
 *  uol_str_new -- create new uol string objecy
 *
 * The uol_str_new() function creates a new uol string object. The size of new
 * object is decided by both of str and len.
 *
 * When str equals to NULL and len greater than 0, the function will create a
 * object with the size of len and will initialize it to all '\0'.
 *
 * If str is not NULL, the content of str will dumped into the new object. the
 * size of the created object should equals to the len parameter passed in. If
 * no len given (len equals to zero), the function will call strlen() and using
 * the return value of strlen() as the length. Keep in mind that invalid pointer
 * will cause a crash of the program (SIGSEGV).
 *
 * Otherwise the function won't create new object. NULL will be returned as 
 * promissory.
 *
 * The uol_str_new() function returns a pointer to the object created, or NULL
 * if any error occured.
 */
uol_str_t *uol_str_new(const char* str, size_t len)
{
	uol_str_t *obj;
	size_t content_size;
	size_t alloc_size;

	if (len == 0 && str == NULL) {
		/* Invalid input */
		return NULL;
	}

	if (len > 0) {
		content_size = len;
	} else {
		content_size = strlen(str) + 1; /* 1 for the ending '\0' */
	}

#ifdef UOL_STR_SUPERALLOC
		alloc_size = content_size;
#else
		alloc_size = content_size + (4096 - content_size % 4096);
#endif

#ifdef UOL_STR_SINGLEALLOC

	obj = malloc(sizeof(uol_str_t) + alloc_size);
	if (obj == NULL) {
		return NULL;
	}

	obj->ptr = &obj[1]; /* jump the first element */

#else

	obj = malloc(sizeof(uol_str_t));
	if (obj == NULL) {
		return NULL;
	}

	obj->ptr = malloc(alloc_size);
	if (obj->ptr == NULL) {
		free(obj);
		return NULL;
	}

#endif

	obj->len = alloc_size;

	if (str == NULL) {
		memset(obj->ptr, 0,   alloc_size);

#ifdef UOL_STR_SUPERALLOC
		obj->used = 0;
#endif

	} else {
		memcpy(obj->ptr, str, content_size);

#ifdef UOL_STR_SUPERALLOC
		obj->used = content_size;
#endif

	}

	return obj;
}


/**
 *  uol_memchr -- locate byte in uol string
 *
 * The uol_memchr() function locates the first occurrence of c (converted to an
 * unsigned char) in strings.
 *
 * The uol_memchr() function returns a pointer to the byte located, or NULL if
 * no such byte exists within n bytes.
 */
void *uol_memchr(const uol_str_t *s, int c, size_t n)
{
	size_t  _n;
	uint8_t *ptr;

#ifdef UOL_STR_SUPERALLOC
	_n  = min(n, s->used);
#else
	_n  = min(n, s->len);
#endif

	ptr = s->ptr;

	while (_n-- > 0) {
		if ((int)*ptr++ == c) {
			return &ptr[-1];
		}
	}

	return NULL;
}


/**
 *  uol_memcmp -- compare uol str as a memory
 *
 * The uol_memcmp() function compares byte string s1 against byte string s2.
 * Both strings are assumed to be n bytes long.
 *
 * The uol_memcmp() function returns zero if the two strings are identical,
 * otherwise returns the difference between the first two differing bytes
 * (treated as unsigned char values, so that `\200' is greater than `\0', for
 * example).  Zero-length strings are always identical.  This behavior is not
 * required by C and portable code should only depend on the sign of the
 * returned value.
 */
int uol_memcmp(const uol_str_t *str1, const uol_str_t *str2, size_t n)
{
	register uint8_t * s1;
	register uint8_t * s2;
	register size_t count;

#ifdef UOL_STR_SUPERALLOC
	count = min(str1->used, str2->used);
#else
	count = min(str1->len, str2->len);
#endif

	count = min(count, n);

	s1 = str1->ptr;
	s2 = str2->ptr;

	while (count-- > 0) {
		if (*s1++ != *s2++) {
			return s1[-1] - s2[-1];
		}
	}

	return 0;
}


/**
 *  uol_strcmp -- compare uol strings as string
 *
 * The uol_strcmp() function lexicographically compare the null-terminated 
 * strings s1 and s2.
 *
 * The uol_strcmp() function return an integer greater than, equal to, or less
 * than 0, according as the string s1 is greater than, equal to, or less than
 * the string s2.  The comparison is done using unsigned characters, so that
 * `\200' is greater than `\0'.
 */
int uol_strcmp(const uol_str_t *str1, const uol_str_t *str2)
{
	const unsigned char *s1 = (const unsigned char *)str1->ptr;
	const unsigned char *s2 = (const unsigned char *)str2->ptr;

	size_t count;

#ifdef UOL_STR_SUPERALLOC
	count = min(str1->used, str2->used);
#else
	count = min(str1->len, str2->len);
#endif

	while (count-- > 0) {
		if (*s1 == '\0' || *s2 == '\0') {
			return s1[0] - s2[0];
		}
		if (*s1++ != *s2++) {
			return s1[-1] - s2[-1];
		}
	}

	return 0;
}
