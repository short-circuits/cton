/*******************************************************************************
 *         CTON Tree
 *******************************************************************************
 *   CTON can manage data in the form of a tree. This part is the interface used
 * to manage data in this way.
 ******************************************************************************/

#include <core/cton_core.h>

/**
 * cton_tree_setroot()
 *   - Set the root object for a cton_tree.
 *
 * PARAMETER
 *   ctx: the cton context.

 * RETURN VALUE
 *   0 for success and other value for any error.
 *
 * DESCRIPTION
 *   This function will make the given object to be the new root element of the
 * cton tree. Unless wrong parameter such as null pointer or invalid object is
 * passed to the function, this function will always success. But the function
 * will set cton error when the root is already not NULL. This error suggests
 * that the tree has been replaced, and maybe there are leaked object.
 *
 * ERRORs
 *   CTON_ERROR_REPLACE: The root is already set, and has been replaced by the
 *                      new passed in object.
 *   CTON_ERROR_TYPE: The object passed by parameter is not valid type for cton
 *                   tree or not valid cton object.
 *
 * TODO
 *   Some code in this function maybe not necessary.
 */
int cton_tree_setroot(cton_ctx *ctx, cton_obj *obj)
{
    cton_type type;

#if 1 /* Is this part necessary? */

    type = cton_object_gettype(ctx, obj);

    if (cton_geterr(ctx) != CTON_OK) {
        return -1;
    }

    if (type != CTON_HASH && type != CTON_ARRAY) {
        
        /**     TODO
         *  In JSON standard, not only hash and array, a number or other values
         * also seems as a valid JSON, so maybe it is no necessary to limit the
         * type of the root object?
         */ 
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return -1;
    }

#endif

    if (cton_tree_getroot(ctx) != NULL) {
        cton_seterr(ctx, CTON_ERROR_REPLACE);

        ctx->root = obj;
        return -2;
    }

    ctx->root = obj;
    return 0;
}


/**
 * cton_tree_getroot()
 *   - Get the root object of the cton tree
 */
cton_obj *cton_tree_getroot(cton_ctx *ctx)
{
    return ctx->root;
}


/**
 * cton_tree_get_by_path()
 *   - Search object by string.
 *
 * TODO
 */
cton_obj *cton_tree_get(cton_ctx *ctx, const char *path)
{
    cton_obj *root;
    cton_obj *obj;

    (void) path;

    root = cton_tree_getroot(ctx);

    obj = root;

    return obj;
}
