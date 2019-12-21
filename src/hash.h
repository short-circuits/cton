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

#ifndef _UOL_HASH_H_
#define _UOL_HASH_H_ 1

#include "obj.h"

typedef struct uol_hash_node_s uol_hash_node_t;
typedef struct uol_hash_s      uol_hash_t;

struct uol_hash_node_s {
    uol_object_t     *key;
    uol_object_t     *value;

    /* manage data using RB-Tree */
    uol_hash_node_t  *left;
    uol_hash_node_t  *right;
    uol_hash_node_t  *parent;
    unsigned char     color;
};


struct uol_hash_s{
    uol_hash_node_t * root;
    uol_hash_node_t * nil;
};


uol_hash_t *uol_hash_new(void);
int *uol_hash_insert(uol_hash_t *hash, uol_object_t *key, uol_object_t *value);
int *uol_hash_remove(uol_hash_t *hash, uol_object_t *key);
uol_object_t *uol_hash_search(uol_hash_t *hash, uol_object_t *key);

#endif /* _UOL_HASH_H_ */