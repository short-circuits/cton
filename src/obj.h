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

#ifndef _UOL_OBJ_H_
#define _UOL_OBJ_H_ 1

#include <stdlib.h>

typedef struct uol_object_s uol_object_t;
typedef uol_object_t uol_t;

typedef struct {
	int  type;
	void *data;
} uol_object_s;

#define UOL_NULL    0
#define UOL_TRUE    1
#define UOL_FALSE   2
#define UOL_STRING  3
#define UOL_HASH    4
#define UOL_ARRAY   5
#define UOL_INTEGER 6 /* Stand for signed int64 */
#define UOL_FLOAT   7 /* Stand for float64 of IEEE754 */
#define UOL_NUMERIC 8 /* Stand for high precision (limited by memory) */

uol_t * uol_new(void);
uol_t * uol_new_str();
#define uol_type(node) ((node)->type)
#define uol_data(node) ((node)->data)

#define uol_set_type(node, new_type) ((node)->type = new_type)

#include "hash.h"
#include "str.h"

#endif /* _UOL_OBJ_H_ */
