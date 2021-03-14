/*******************************************************************************
 * "THE BEER-WARE LICENSE" (Revision 43):
 * <yeonji@ieee.org> create this project. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a sweety curry rice in return (cuz
 * I cannot drink alcoholic beverages). Yeonji Lee
 *
 ******************************************************************************/

#ifndef _CTON_CORELIB_HEADER_
#define _CTON_CORELIB_HEADER_ 1

#include <inttypes.h>
#include <stddef.h>

#define _CTON_CORELIB_ 1

#include <cton.h>
#include <auto_config.h>

#define CTON_STRUCT_MAGIC 0x4E4F5443

/*
 * cton_objtype(obj)
 *
 * DESCRIPTION
 *   Same as cton_object_gettype(), but only available for internal useage.
 *
 * PARAMETER
 *   obj: The object which you want to get it's type
 *
 * RETURN
 *   The type of given object.
 */
#define cton_objtype(obj) ((obj)->type)

struct cton_bool_s {
	struct cton_obj_s obj;
	enum   cton_bool_e val;
};

struct cton_string_s {
	struct   cton_obj_s obj;
    size_t   len;
    size_t   used;
    uint8_t  *ptr;
};

struct cton_array_s {
	struct           cton_obj_s obj;
    size_t           len;
    size_t           used;
    void             *ptr;

    enum cton_type_e sub_type;
};

typedef struct cton_rbtree_node_s cton_rbtree_node_t;

struct cton_rbtree_node_s {
    cton_obj            *key;
    cton_obj            *value;
    cton_rbtree_node_t  *left;
    cton_rbtree_node_t  *right;
    cton_rbtree_node_t  *parent;
    unsigned char        color;
};

typedef struct cton_rbtree_s cton_rbtree_t;

struct cton_rbtree_s {
    cton_rbtree_node_t *root;
    cton_rbtree_node_t *sentinel;
};

struct cton_hash_s {
	struct cton_obj_s    obj;
    struct cton_rbtree_s tree;
    int count;
};

int cton_array_complex(cton_obj *obj);

#endif /* _CTON_CORELIB_HEADER_ */
