
static cton_hash_item *cton_hash_search(cton_obj *h, cton_obj *k)
{
	cton_hash_item *current;

	current = h->payload.hash.root;

    while (current != NULL) {
    	if (cton_util_strcmp(k, current->key) == 0) {
    		break;
    	}

    	current = current->next;
    }

    return current;
}

static cton_obj *cton_hash_rmkey(cton_ctx *ctx, cton_obj *h, cton_obj *k)
{
	cton_hash_item *prev;
	cton_hash_item *current;

	cton_obj *v;

	prev    = NULL;
	current = h->payload.hash.root;

	while (current != NULL) {
    	if (cton_util_strcmp(k, current->key) == 0) {
    		break;
    	}

    	prev    = current;
    	current = current->next;
    }

    if (current == NULL) {
    	return NULL;
    }

    if (prev == NULL) {
    	h->payload.hash.root = current->next;
    } else {
    	prev->next = current->next;
    }

    if (current->next == NULL) {
    	h->payload.hash.root = prev;
    }

    v = current->value;

    cton_free(ctx, current->key);
    cton_free(ctx, current);

	return v;
}

cton_obj * cton_hash_get(cton_ctx *ctx, cton_obj *h, cton_obj *k)
{
	cton_hash_item *result;

	if (cton_object_gettype(ctx, h) != CTON_HASH) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    if (cton_object_gettype(ctx, k) != CTON_STRING) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    result = cton_hash_search(h,k);

    if (result == NULL) {
        fprintf(stderr, "I'm here\n");
    	return NULL;
    }

    return result->value;
}

cton_obj * cton_hash_set(cton_ctx *ctx, cton_obj *h, cton_obj *k, cton_obj *v)
{
	cton_hash_item *pos;
	cton_obj *v_ori;

	if (cton_object_gettype(ctx, h) != CTON_HASH) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    if (cton_object_gettype(ctx, k) != CTON_STRING) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    if (v == NULL) {
    	return cton_hash_rmkey(ctx, h, k);
    }

    if (cton_object_gettype(ctx, v) == CTON_INVALID) {
        cton_seterr(ctx, CTON_ERROR_TYPE);
        return NULL;
    }

    pos = cton_hash_search(h,k);

    if (pos != NULL) {
    	v_ori = pos->value;

    	pos->value = v;
    } else {
    	v_ori = NULL;

    	pos = cton_alloc(ctx, sizeof(cton_hash_item));
    	if (pos == NULL) {
    		cton_seterr(ctx, CTON_ERROR_EALLOC);
    		return NULL;
    	}

    	pos->key   = k;
    	pos->value = v;
    	pos->next  = NULL;

    	if (h->payload.hash.root == NULL) {
    		h->payload.hash.root = pos;
    	} else {
    		h->payload.hash.last->next = pos;
    	}

    	h->payload.hash.last = pos;
    }

    (void)v_ori;

	return v;
}
