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

#include "obj.h"

/* Functions for color in uol_hash_t */
#define uol_rbtree_set_red(node)   ((node)->color = 1)
#define uol_rbtree_set_black(node) ((node)->color = 0)
#define uol_rbtree_is_red(node)    ((node)->color)
#define uol_rbtree_is_black(node)  (!uol_rbtree_is_red(node))


/**
 *  uol_hash_new -- create a new uol hash
 *
 * The uol_hash_new() function creates a new uol hash structure and initialize 
 * it as a empty hash table.
 *
 * The uol_hash_new() function returns the pointer to the new created struct if
 * succeeded to create it. Otherwise, it will returns NULL.
 */
uol_hash_t *uol_hash_new(void)
{
	uol_hash_t *hash;

	/* Alloc the tree struct */
	hash = malloc(sizeof(uol_hash_t));
	if (hash == NULL) {
		return NULL;
	}

	/* Create a nil node as sentinel node */
	hash->nil = malloc(sizeof(uol_hash_node_t));
	if (hash->nil == NULL) {
		free(hash);
		return NULL;
	}

	/* Initialize nil */
	hash->nil->left   = NULL;
	hash->nil->right  = NULL;
	hash->nil->parent = NULL;
	uol_rbtree_set_black(hash->nil); /* sentinel node should be black */

	/* RB-Tree initialized as a empty tree */
	hash->root = hash->nil;

	return hash;
}


/**
 *  uol_rbtree_rotateL -- left rotate the uol hash rb-tree
 *
 * The uol_rbtree_rotateL() is a anonymous function to left rotate the uol hash
 * rb-tree.
 *
 * Tips: Binary Tree Left Rotate:
 *
 *    |                 |
 *   node             right
 *   /  \             /   \
 *  a   right   =>  node   c
 *      /   \       /  \
 *     b     c     a    b
 */
void uol_rbtree_rotateL(uol_hash_t * tree, uol_hash_node_t * node)
{
	uol_hash_node_t * right;

	/**
	 * Start with
	 *    |
	 *   node
	 *   /  \
	 *  a   right
	 *      /   \
	 *     b     c
	 */
	right = node->right;

	/**
	 * Cut up the right branch
	 *    |
	 *   node     right
	 *   /  \         \
	 *  a    b         c
	 */
	node->right = right->left;
	if (right->left != tree->nil) {
		/* If b is not nil node, change it's parent */
		right->left->parent = node;
	}

	/**
	 * Move the parent to right
	 *              |
	 *   node     right
	 *   /  \         \
	 *  a    b         c
	 */
	right->parent = node->parent;
	if (node == tree->root) {
		/* Change root pointer if node is root */
		tree->root = right;

	} else if (node == node->parent->left) {
		node->parent->left = right;

	} else {
		node->parent->right = right;

	}

	/**
	 * Connect node to right node
	 *       |
	 *     right
	 *     /   \
	 *   node   c
	 *   /  \
	 *  a    b
	 */
	right->left  = node;
	node->parent = right;
}


/**
 *  uol_rbtree_rotateR -- right rotate the uol hash rb-tree
 *
 * The uol_rbtree_rotateR() is a anonymous function to right rotate the uol hash
 * rb-tree. ( Not commented because almost the same with uol_rbtree_rotateL() )
 *
 * Tips: Binary Tree Right Rotate
 *
 *       |           | 
 *      node        left
 *      /  \        /   \
 *   left   c  =>  a    node
 *   /  \               /  \
 *  a    b             b    c
 */
void uol_rbtree_rotateR(uol_hash_t * tree, uol_hash_node_t * node)
{
	uol_hash_node_t * left;

	left = node->left;

	node->left = left->right;
	if (left->right != tree->nil) {
		left->right->parent = node;
	}

	left->parent = node->parent;
	if (node == tree->root) {
		tree->root = left;

	} else if (node == node->parent->left) {
		node->parent->left = left;

	} else {
		node->parent->right = left;

	}

	left->right  = node;
	node->parent = left;
}


/**
 *  uol_rbtree_compare -- compare two item of the uol hash rb-tree
 *
 * The uol_rbtree_compare() is used to compare two item of the uol hash RB-Tree.
 * 
 * The uol_rbtree_compare() function return an integer greater than, equal to, 
 * or less than 0, according as the object key1 is greater than, equal to, or 
 * less than the object key2. 
 *
 * Problem: How will insert function deal with equal?
 */
int uol_rbtree_compare(uol_object_t * key1, uol_object_t * key2)
{

#ifdef UOL_HASH_NUMERICKEY

	if (key1->type != key2->type) {
		return (key1->type - key2->type)
	}

	if (key1->type == UOL_STRING) {

#endif /* UOL_HASH_NUMERICKEY */

		return uol_strcmp((uol_str_t *)key1, (uol_str_t *)key2);

#ifdef UOL_HASH_NUMERICKEY

	} else { /* TODO: Add support of using int or arr */
		return 0;
	}
#endif /* UOL_HASH_NUMERICKEY */
}

/**
 *  uol_rbtree_insert -- insert item into the uol hash rb-tree
 *
 * The uol_rbtree_insert() is used to insert one item into the uol hash RB-Tree.
 * 
 * The uol_rbtree_insert() function has no return value. It will always succeed.
 */
void uol_rbtree_insert(uol_hash_t * tree, uol_hash_node_t * node)
{
	uol_hash_node_t * uncle;

	if (tree->root == tree->nil) { /* A empty tree */
		node->left   = tree->nil;
		node->right  = tree->nil;
		node->parent = tree->nil;
		uol_rbtree_set_black(node);

		tree->root = node;

		return;
	}

	{ /* Insert the node into a leaf */
		uol_hash_node_t * last; /* the last branch node in the tree */
		uol_hash_node_t ** pos; /* the leaf to be replaced */

		last = tree->root;

		/* Find the leaf to replace */
		while (1) {
			if(uol_rbtree_compare(node->key, last->key) > 0) {
				pos = &last->left;
			} else {
				pos = &last->right;
			}

			if (*pos == tree->nil) {
				break;
			}

			last = *pos;
		}

		/* replace the leaf */
		*pos = node;
		node->parent = last;
		node->left   = tree->nil;
		node->right  = tree->nil;

		/* the new node is red */
		uol_rbtree_set_red(node);

	} /* End insert the node into a leaf */

	/* re-balance the tree */
	while (node != tree->root && uol_rbtree_is_red(node->parent)) {
		/* if the parent node is red, no need to adjust this tree */

		/**
		 *    GrandParent
		 *      /     \ 
		 *   Parent  Uncle
		 *    /
		 *  node
		 */

		/* Check parent is at which side of grandparent */
		if (node->parent == node->parent->parent->left) {
			uncle = node->parent->parent->right;

			if (uol_rbtree_is_red(uncle)) {
				/* If parent is red and uncle also red.
				 *
				 *      [B]GrandParent
				 *      /        \ 
				 *    [R]Parent  [R]Uncle
				 *    /
				 *  [R]node
				 */

				/* Re-color P and U to black, and GP to red */
				uol_rbtree_set_black(node->parent);
				uol_rbtree_set_black(uncle);
				uol_rbtree_set_red(node->parent->parent);

				/* Recursion with GP */
				node = node->parent->parent;

			} else {
				/* If parent is red and uncle also red.
				 *
				 *      [B]GrandParent
				 *      /        \ 
				 *    [R]Parent  [B]Uncle
				 *    /
				 *  [R]node
				 */
				if (node == node->parent->right) {
					/* Confirm the node is the left leaf of parent */
					/* Rotating to switch two node to keep in order */
					node = node->parent;
					uol_rbtree_rotateL(tree, node);
				}

				uol_rbtree_set_black(node->parent);
				uol_rbtree_set_red(node->parent->parent);
				uol_rbtree_rotateR(tree, node->parent->parent);

				/* After this will be
				 *
				 *    [B]Parent
				 *    /      \ 
				 *  [R]node  [R]GrandParent
				 *             \
				 *             [B]Uncle
				 *
				 * Exit loop because node->parent is black
				 */
			}

		} else {
			/* Same as before, just change direction */
			uncle = node->parent->parent->left;

			if (uol_rbtree_is_red(uncle)) {
				uol_rbtree_set_black(node->parent);
				uol_rbtree_set_black(uncle);
				uol_rbtree_set_red(node->parent->parent);

				node = node->parent->parent;

			} else {
				if (node == node->parent->left) {
					node = node->parent;
					uol_rbtree_rotateR(tree, node);
				}

				uol_rbtree_set_black(node->parent);
				uol_rbtree_set_red(node->parent->parent);
				uol_rbtree_rotateL(tree, node->parent->parent);
			}

		}

	}

	/* Confirm the root is black */
	uol_rbtree_set_black(tree->root);
}
