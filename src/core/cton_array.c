/*******************************************************************************
 * CTON type dependent methods
 *
 *******************************************************************************
 *         Array
 *******************************************************************************
 *
 *   Array in cton is defined as an ordered data structure that can be access by
 * it's index. All elements in an array should takes the same type. Though, cton
 * object is an special type can be used, and will cause an array holds data in
 * different types. That means the array can storage elements with different
 * types de facto.
 *   The method to create an array object is not as easy as numeric object. You
 * need to call cton_object_create() firstly to create an object with the type
 * of array. Than call the cton_array_settype() to assign the array with a type.
 * You cannot change the type of an array after this step, and you cannot skip
 * this step unless you won't use this array. After assign the type to the array
 * you maybe want to declear the size of this array. Call cton_array_setlen()
 * will allow to set the size of the array, and the size of the array can be
 * changed during the life cycle of the object.
 *
 *******************************************************************************
 *
 *   In this part,all of the function should start with `cton_array_`, and
 * accepts at least 2 parameter in the order of `cton_ctx *ctx, cton_obj *arr`
 *
 ******************************************************************************/

static void cton_array_init(cton_ctx *ctx, cton_obj *arr)
{
    (void) ctx;
    arr->payload.arr.ptr  = NULL;
    arr->payload.arr.len  = 0;
    arr->payload.arr.used = 0;
    arr->payload.arr.sub_type = CTON_INVALID;

}

/*
 * cton_array_settype()
 *
 * DESCRIPTION
 *   Assign a sub-object type to an array object.
 *   This fuction will set the arr->payload.arr.sub_type to the type passed by
 * parameter.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object assign it's sub-type.
 *   type: The type the array will hold.
 *
 * RETURN
 *   Zero for success and other value for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *
 */
int cton_array_settype(cton_ctx *ctx, cton_obj *arr, cton_type type)
{
    if (cton_object_gettype(ctx, arr) != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return -1;
    }

    arr->payload.arr.sub_type = type;

    return 0;
}


cton_type cton_array_gettype(cton_ctx *ctx, cton_obj *arr)
{
    if (cton_object_gettype(ctx, arr) != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return CTON_INVALID;
    }

    return arr->payload.arr.sub_type;
}

/*
 * cton_array_getlen()
 *
 * DESCRIPTION
 *   Get how many sub-objects the array object can hold.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to get it's volume.
 *
 * RETURN
 *   The volume of this array object or zero for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 */
size_t cton_array_getlen(cton_ctx *ctx, cton_obj *arr)
{
    if (cton_object_gettype(ctx, arr) != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return 0;
    }

    return arr->payload.arr.used;
}

/*
 * cton_array_setlen()
 *
 * DESCRIPTION
 *   Change the number of objects an array can contain.
 *
 *   The behavior of this function is determined by the content of the given
 * array object. If the given array object holds a NULL pointer, this function
 * will do a new allocation for the array, and set all of the allocated object
 * to NULL pointer. Or if the array object is not NULL pointered, and given
 * length is bigger than the length allocated in the array, this function will
 * trying to expand the space of the array. If the given is less than the given
 * array object holds, this function won't shrink the space actually, it just
 * set the used flag to the length given. Also, this step won't trying to delete
 * the object in the space vanished.
 *   
 *   Although the type of the array is not referenced by this function, for
 * safety reason, this function still requests type is pre-seted.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to change it's size.
 *   len: The new size this array will be.
 *
 * RETURN
 *   The new length of the array or 0 for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *   CTON_ERROR_SUBTYPE: Sub-type is not assigned.
 *   CTON_ERROR_EALLOC: memory hook returns NULL pointer.
 */
size_t cton_array_setlen(cton_ctx *ctx, cton_obj *arr, size_t len)
{
    size_t index;
    cton_obj **ptr;

    if (cton_object_gettype(ctx, arr) != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return 0;
    }

    if (cton_object_gettype(ctx, arr) == CTON_INVALID) {
        cton_seterr(ctx, CTON_ERROR_SUBTYPE);
        return 0;
    }

    if (arr->payload.arr.ptr == NULL) {
        /* Space have not been allocated yet */
        ptr  = cton_alloc(ctx, len * sizeof(cton_obj *));

        if ( ptr == NULL ) {
            cton_seterr(ctx, CTON_ERROR_EALLOC);
            return 0;
        }

        arr->payload.arr.ptr  = ptr;
        arr->payload.arr.len  = len;

        for (index = 0; index < len; index ++) {
            arr->payload.arr.ptr[index] = NULL;
        }

        arr->payload.arr.used = len;

    } else if (arr->payload.arr.len < len) {

        ptr = cton_realloc(ctx,
            arr->payload.arr.ptr, arr->payload.arr.len * sizeof(cton_obj *),
            len * sizeof(cton_obj *));

        if ( ptr == NULL ) {
            cton_seterr(ctx, CTON_ERROR_EALLOC);
            return 0;
        }

        arr->payload.arr.ptr  = ptr;
        arr->payload.arr.len  = len;

        for (index = arr->payload.arr.used; index < len; index ++) {
            arr->payload.arr.ptr[index] = NULL;
        }

        arr->payload.arr.used = len;

    } else {
        /* Shink this array */
        arr->payload.arr.used = len;
        
    }

    return len;
}

/*
 * cton_array_get()
 *
 * DESCRIPTION
 *   Get the sub object in array container by index.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to get it's sub-object.
 *   index: The index of the object you want to get.
 *
 * RETURN
 *   The pointer to sub-object or NULL for any error.
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *   CTON_ERROR_INDEX: The requested index is out of the range.
 */
cton_obj * cton_array_get(cton_ctx *ctx, cton_obj *arr, size_t index)
{
    if (cton_object_gettype(ctx, arr) != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }
    
    if (cton_array_getlen(ctx, arr) <= index) {
        cton_seterr(ctx, CTON_ERROR_INDEX);
        return NULL;
    }

    return arr->payload.arr.ptr[index];
}

/*
 * cton_array_set()
 *
 * DESCRIPTION
 *   Put a object into an array container at the position by index.
 *
 * PARAMETER
 *   ctx: The cton context
 *   arr: The array object that you want to get it's sub-object.
 *   index: The index of the object you want to put.
 *
 * RETURN
 *   Zero for success or other for any kinds of error
 *
 * ERRORS
 *   CTON_ERROR_TYPE: Parameter arr is not an array object.
 *   CTON_ERROR_SUBTYPE: Given object is not match the type condition of array.
 *   CTON_ERROR_INDEX: The requested index is out of the range.
 */
int cton_array_set(cton_ctx *ctx, cton_obj *arr, cton_obj *obj, size_t index)
{
    if (cton_object_gettype(ctx, arr) != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return -1;
    }

    if (cton_array_gettype(ctx, arr) != CTON_OBJECT && \
        cton_array_gettype(ctx, arr) != cton_object_gettype(ctx, obj)) {
        cton_seterr(ctx, CTON_ERROR_SUBTYPE);
        return -1;
    }

    if (cton_array_getlen(ctx, arr) <= index) {
        cton_seterr(ctx, CTON_ERROR_INDEX);
        return -1;
    }

    arr->payload.arr.ptr[index] = obj;
    return 0;
}

void * cton_array_getptr(cton_ctx *ctx, cton_obj *arr)
{
    if (cton_object_gettype(ctx, arr) != CTON_ARRAY) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    return arr->payload.arr.ptr;
}
