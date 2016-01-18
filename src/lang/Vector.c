#ifndef __LISP_LANG_VECTOR_C__
#define __LISP_LANG_VECTOR_C__


static lisp_VectorMutList* lisp_VectorMutList_constructor(lisp_VectorMutList* mut_list, lisp_Value** values) {
    mut_list->ref_count = 1;
    mut_list->values = values;
    return mut_list;
}

static void lisp_VectorMutList_destructor(lisp_State* state, lisp_VectorMutList* mut_list) {
    lisp_u32 index = 0;
    lisp_Value* value = mut_list->values[index];

    while (value != NULL) {
        lisp_Value_deref(state, value);
        index += 1;
        value = mut_list->values[index];
    }
    free(mut_list->values);
}

static lisp_VectorMutList* lisp_VectorMutList_new(void) {
    return lisp_VectorMutList_constructor(
        (lisp_VectorMutList*) malloc(sizeof(lisp_VectorMutList)),
        lisp_Vector_alloc_values()
    );
}

static void lisp_VectorMutList_delete(lisp_State* state, lisp_VectorMutList* mut_list) {
    lisp_VectorMutList_destructor(state, mut_list);
    free(mut_list);
}

static void lisp_VectorMutList_copy(lisp_VectorMutList* a, lisp_VectorMutList* b, lisp_u32 size) {
    lisp_values_copy(a->values, b->values, 0, size, 0);
}

static void lisp_VectorMutList_ref(lisp_VectorMutList* mut_list) {
    mut_list->ref_count += 1;
}

static void lisp_VectorMutList_deref(lisp_State* state, lisp_VectorMutList* mut_list) {
    mut_list->ref_count -= 1;

    if (mut_list->ref_count == 0) {
        lisp_VectorMutList_delete(state, mut_list);
    }
}


static struct lisp_Value** lisp_Vector_alloc_values(void) {
    lisp_Value** values = (lisp_Value**) malloc(LISP_VECTOR_SIZE * sizeof(lisp_Value*));
    for (lisp_u32 i = 0, il = LISP_VECTOR_SIZE; i < il; i++) {
        values[i] = NULL;
    }
    return values;
}

static lisp_VectorNode** lisp_Vector_alloc_nodes(void) {
    lisp_VectorNode** nodes = (lisp_VectorNode**) malloc(LISP_VECTOR_SIZE * sizeof(lisp_VectorNode*));
    for (lisp_u32 i = 0, il = LISP_VECTOR_SIZE; i < il; i++) {
        nodes[i] = NULL;
    }
    return nodes;
}


static lisp_VectorNode* lisp_VectorNode_constructor(
    lisp_VectorNode* node, lisp_VectorMutList* values, lisp_VectorNode** mut_list
) {
    node->ref_count = 1;
    if (values != NULL) {
        node->values = values;
        node->is_values = LISP_TRUE;
    } else {
        node->mut_list = mut_list;
        node->is_values = LISP_FALSE;
    }
    return node;
}

static void lisp_VectorNode_destructor(lisp_State* state, lisp_VectorNode* node) {
    if (node->is_values) {
        lisp_VectorMutList_deref(state, node->values);
    } else {
        lisp_u32 index = 0;
        lisp_VectorNode* value = node->mut_list[index];

        while (value != NULL) {
            lisp_VectorNode_deref(state, value);
            index += 1;
            value = node->mut_list[index];
        }
        free(node->mut_list);
    }
}

static lisp_VectorNode* lisp_VectorNode_new_values(void) {
    return lisp_VectorNode_constructor(
        (lisp_VectorNode*) malloc(sizeof(lisp_VectorNode)),
        lisp_VectorMutList_new(),
        NULL
    );
}

static lisp_VectorNode* lisp_VectorNode_new_mut_list(void) {
    return lisp_VectorNode_constructor(
        (lisp_VectorNode*) malloc(sizeof(lisp_VectorNode)),
        NULL,
        lisp_Vector_alloc_nodes()
    );
}

static lisp_VectorNode* lisp_VectorNode_new_tail(lisp_VectorMutList* mut_list) {
    return lisp_VectorNode_constructor(
        (lisp_VectorNode*) malloc(sizeof(lisp_VectorNode)),
        mut_list,
        NULL
    );
}

static void lisp_VectorNode_delete(lisp_State* state, lisp_VectorNode* node) {
    lisp_VectorNode_destructor(state, node);
    free(node);
}

static lisp_VectorNode* lisp_VectorNode_copy(lisp_VectorNode* node, lisp_VectorNode* other, lisp_u32 size) {
    if (node->is_values) {
        lisp_VectorMutList_copy(node->values, other->values, size);
    } else {
        lisp_vector_nodes_copy(node->mut_list, other->mut_list, 0, size, 0);
    }
    return node;
}

static lisp_VectorNode* lisp_VectorNode_clone(lisp_VectorNode* node, lisp_u32 size) {
    if (node->is_values) {
        return lisp_VectorNode_copy(lisp_VectorNode_new_values(), node, size);
    } else {
        return lisp_VectorNode_copy(lisp_VectorNode_new_mut_list(), node, size);
    }
}

static void lisp_VectorNode_ref(lisp_VectorNode* node) {
    node->ref_count += 1;

    if (node->is_values) {
        lisp_VectorMutList_ref(node->values);
    } else {
        lisp_u32 index = 0;
        lisp_VectorNode* value = node->mut_list[index];

        while (value != NULL) {
            lisp_VectorNode_ref(value);
            index += 1;
            value = node->mut_list[index];
        }
    }
}

static void lisp_VectorNode_deref(lisp_State* state, lisp_VectorNode* node) {
    node->ref_count -= 1;

    if (node->is_values) {
        lisp_VectorMutList_deref(state, node->values);
    } else {
        lisp_u32 index = 0;
        lisp_VectorNode* value = node->mut_list[index];

        while (value != NULL) {
            lisp_VectorNode_deref(state, value);
            index += 1;
            value = node->mut_list[index];
        }
    }

    if (node->ref_count == 0) {
        lisp_VectorNode_delete(state, node);
    }
}


static lisp_Vector* lisp_Vector_constructor(
    lisp_Vector* vector, lisp_VectorNode* root, lisp_VectorMutList* tail, lisp_u32 size, lisp_u32 shift
) {
    vector->root = root;
    vector->tail = tail;
    vector->size = size;
    vector->shift = shift;
    return vector;
}

static void lisp_Vector_destructor(lisp_State* state, lisp_Vector* vector) {
    if (vector->root != NULL) {
        lisp_VectorNode_deref(state, vector->root);
    }
}

static lisp_Value* lisp_Vector_from_mut_list(lisp_State* state, lisp_MutList* mut_list) {
    lisp_u32 size = lisp_MutList_size(mut_list);

    if (size == 0) {
        return lisp_Value_vector(state);
    } else {
        lisp_Value* value = lisp_Vector_internal_new(state);
        lisp_Vector_push_mut_list(state, &value->vector, mut_list);
        return value;
    }
}

static struct lisp_Value* lisp_Vector_from_list(lisp_State* state, lisp_List* list) {
    lisp_u32 size = lisp_List_size(list);

    if (size == 0) {
        return lisp_Value_vector(state);
    } else {
        lisp_Value* value = lisp_Vector_internal_new(state);
        lisp_MutList* mut_list = lisp_MutList_new();
        lisp_ListNode* node = list->root;

        while (node != NULL) {
            lisp_MutList_push(mut_list, node->value);
            node = node->next;
        }

        lisp_Vector_push_mut_list(state, &value->vector, mut_list);
        lisp_MutList_delete(mut_list);

        return value;
    }
}

static lisp_Value* lisp_Vector_internal_new(lisp_State* state) {
    lisp_Value* value = lisp_State_alloc(state);
    value->type = LISP_TYPE_VECTOR;
    value->vector.value = value;
    value->ref_count = 1;
    lisp_Vector_constructor(&value->vector, NULL, NULL, 0, LISP_VECTOR_SHIFT);
    return value;
}

static lisp_Value* lisp_Vector_clone(lisp_State* state, lisp_Vector* vector) {
    lisp_Value* value = lisp_Vector_internal_new(state);
    lisp_Vector_constructor(&value->vector, vector->root, vector->tail, vector->size, vector->shift);
    return value;
}

static lisp_u32 lisp_Vector_size(lisp_Vector* vector) {
    return vector->size;
}

static lisp_u32 lisp_Vector_tail_off(lisp_u32 size) {
    if (size < LISP_VECTOR_SIZE) {
        return 0;
    } else {
        return ((size - 1) >> LISP_VECTOR_SHIFT) << LISP_VECTOR_SHIFT;
    }
}

static lisp_VectorNode* lisp_Vector_get_node(lisp_Vector* vector, lisp_u32 index) {
    lisp_VectorNode* node = vector->root;
    lisp_u32 level = vector->shift;

    while (level > 0) {
        node = node->mut_list[(index >> level) & LISP_VECTOR_MASK];
        level = level - LISP_VECTOR_SHIFT;
    }

    return node;
}

static lisp_VectorMutList* lisp_Vector_get_mut_list_for(lisp_Vector* vector, lisp_u32 index) {
    if (index >= lisp_Vector_tail_off(vector->size)) {
        return vector->tail;
    } else {
        return lisp_Vector_get_node(vector, index)->values;
    }
}

static lisp_Value* lisp_Vector_find_index(lisp_Vector* vector, lisp_u32 index) {
    return lisp_Vector_get_mut_list_for(vector, index)->values[index & LISP_VECTOR_MASK];
}

static lisp_Value* lisp_Vector_get(lisp_State* state, lisp_Vector* vector, lisp_u32 index) {
    if (index >= vector->size) {
        return state->nil;
    } else {
        return lisp_Vector_find_index(vector, index);
    }
}

static lisp_VectorNode* lisp_Vector_new_path_set(
    lisp_State* state, lisp_VectorNode* node, lisp_u32 size, lisp_u32 index, lisp_Value* value, lisp_u32 level
) {
    lisp_VectorNode* new_node = lisp_VectorNode_clone(node, ((size - 1) >> level) & LISP_VECTOR_MASK);

    if (level == 0) {
        new_node->values->values[index & LISP_VECTOR_MASK] = value;
    } else {
        lisp_u32 sub_index = (index >> level) & LISP_VECTOR_MASK;
        new_node->mut_list[sub_index] = lisp_Vector_new_path_set(
            state, node->mut_list[sub_index], size, index, value, level - LISP_VECTOR_SHIFT
        );
    }

    return new_node;
}

static lisp_Value* lisp_Vector_set(lisp_State* state, lisp_Vector* vector, lisp_u32 index, lisp_Value* value) {
    if (index >= vector->size) {
        return vector->value;
    } else {
        if (index >= lisp_Vector_tail_off(vector->size)) {
            lisp_VectorMutList* tail = vector->tail;
            lisp_u32 masked_index = index & LISP_VECTOR_MASK;

            if (lisp_Value_equal(tail->values[masked_index], value)) {
                return vector->value;
            } else {
                lisp_VectorMutList* new_tail = lisp_VectorMutList_new();
                lisp_VectorMutList_copy(new_tail, tail, (vector->size + 1) & LISP_VECTOR_MASK);
                new_tail->values[masked_index] = value;
                lisp_Value* new_vector = lisp_Vector_clone(state, vector);
                new_vector->vector.tail = new_tail;
                return new_vector;
            }
        } else if (lisp_Value_equal(lisp_Vector_find_index(vector, index), value)) {
            return vector->value;
        } else {
            lisp_Value* new_vector = lisp_Vector_clone(state, vector);
            new_vector->vector.root = lisp_Vector_new_path_set(state, vector->root, vector->size, index, value, vector->shift);
            return new_vector;
        }
    }
}

static lisp_VectorNode* lisp_Vector_new_path(lisp_State* state, lisp_VectorNode* node, lisp_u32 level) {
    if (level == 0) {
        return node;
    } else {
        lisp_VectorNode* new_node = lisp_VectorNode_new_mut_list();
        new_node->mut_list[0] = lisp_Vector_new_path(state, node, level - LISP_VECTOR_SHIFT);
        return new_node;
    }
}

static lisp_VectorNode* lisp_Vector_push_tail(
    lisp_State* state, lisp_VectorNode* parent, lisp_VectorNode* tail, lisp_u32 size, lisp_u32 level
) {
    lisp_u32 sub_index = ((size - 1) >> level) & LISP_VECTOR_MASK;
    lisp_VectorNode* new_node = lisp_VectorNode_clone(parent, sub_index);
    lisp_VectorNode* insert_node;

    if (level == LISP_VECTOR_SHIFT) {
        insert_node = tail;
    } else {
        lisp_VectorNode* child = parent->mut_list[sub_index];

        if (child != NULL) {
            insert_node = lisp_Vector_push_tail(state, child, tail, size, level - LISP_VECTOR_SHIFT);
        } else {
            insert_node = lisp_Vector_new_path(state, tail, level - LISP_VECTOR_SHIFT);
        }
    }

    new_node->mut_list[sub_index] = insert_node;

    return new_node;
}

static void lisp_Vector_push_value(lisp_State* state, lisp_Vector* vector, lisp_Value* value) {
    lisp_VectorNode* root = vector->root;
    lisp_u32 size = vector->size;
    lisp_u32 shift = vector->shift;

    if (size - lisp_Vector_tail_off(size) < LISP_VECTOR_SIZE) {
        vector->tail->values[size & LISP_VECTOR_MASK] = value;
    } else {
        lisp_VectorNode* new_root;
        lisp_VectorNode* tail_node = lisp_VectorNode_new_tail(vector->tail);
        lisp_u32 new_shift = shift;

        if ((size >> LISP_VECTOR_SHIFT) > (1 << shift)) {
            new_root = lisp_VectorNode_new_mut_list();
            new_root->mut_list[0] = root;
            new_root->mut_list[1] = lisp_Vector_new_path(state, tail_node, shift);
            new_shift += LISP_VECTOR_SHIFT;
        } else {
            new_root = lisp_Vector_push_tail(state, root, tail_node, size, shift);
        }

        lisp_VectorMutList* new_tail = lisp_VectorMutList_new();
        new_tail->values[0] = value;

        lisp_VectorNode_deref(state, root);
        lisp_VectorMutList_deref(state, vector->tail);

        vector->tail = new_tail;
        vector->root = new_root;
        vector->shift = new_shift;
    }

    vector->size = size + 1;
}

static void lisp_Vector_push_mut_list(lisp_State* state, lisp_Vector* vector, lisp_MutList* mut_list) {
    lisp_VectorMutList* tail = vector->tail;
    lisp_u32 size = vector->size;

    if (tail == NULL) {
        vector->tail = lisp_VectorMutList_new();
    } else if (size - lisp_Vector_tail_off(size) < LISP_VECTOR_SIZE) {
        lisp_VectorMutList* new_tail = lisp_VectorMutList_new();
        lisp_VectorMutList_copy(tail, new_tail, (size + 1) & LISP_VECTOR_MASK);
        lisp_VectorMutList_deref(state, tail);
        vector->tail = new_tail;
    }

    for (lisp_u32 i = 0, il = mut_list->size; i < il; i++) {
        lisp_Vector_push_value(state, vector, lisp_MutList_get(mut_list, i));
    }
}

static lisp_Value* lisp_Vector_push(lisp_State* state, lisp_Vector* vector, lisp_Value* value) {
    lisp_u32 size = vector->size;
    lisp_VectorMutList* tail = vector->tail;

    lisp_Value* new_value = lisp_Vector_clone(state, vector);
    lisp_Vector* new_vector = &new_value->vector;

    if (tail == NULL) {
        new_vector->tail = lisp_VectorMutList_new();
    } else if (size - lisp_Vector_tail_off(size) < LISP_VECTOR_SIZE) {
        lisp_VectorMutList* new_tail = lisp_VectorMutList_new();
        lisp_VectorMutList_copy(tail, new_tail, (size + 1) & LISP_VECTOR_MASK);
        lisp_VectorMutList_deref(state, tail);
        new_vector->tail = new_tail;
    }

    lisp_Vector_push_value(state, new_vector, value);

    return new_value;
}

static lisp_Value* lisp_Vector_to_string(lisp_State* state, lisp_Vector* vector) {
    lisp_Value* value = lisp_Value_string_from_cstring(state, "[");

    for (lisp_u32 i = 0, il = lisp_Vector_size(vector); i < il; i++) {
        lisp_Value* value_to_string = lisp_Value_to_string(state, lisp_Vector_get(state, vector, i));
        lisp_Value* tmp;

        if (i != il - 1) {
            lisp_Value* separator = lisp_Value_string_from_cstring(state, " ");
            tmp = lisp_String_concat(state, &value_to_string->string, &separator->string);
            lisp_Value_deref(state, separator);
        } else {
            tmp = value_to_string;
        }

        lisp_Value* new_value = lisp_String_concat(state, &value->string, &tmp->string);
        lisp_Value_deref(state, value);
        lisp_Value_deref(state, tmp);

        value = new_value;
    }

    lisp_Value* end_bracket = lisp_Value_string_from_cstring(state, "]");
    lisp_Value* new_value = lisp_String_concat(state, &value->string, &end_bracket->string);
    lisp_Value_deref(state, end_bracket);
    lisp_Value_deref(state, value);

    return new_value;
}

static lisp_bool lisp_Vector_equal(lisp_Vector* a, lisp_Vector* b) {
    if (a->size != b->size) {
        return LISP_FALSE;
    } else {
        for (lisp_u32 i = 0, il = a->size; i < il; i++) {
            if (!lisp_Value_equal(lisp_Vector_find_index(a, i), lisp_Vector_find_index(b, i))) {
                return LISP_FALSE;
            }
        }
        return LISP_TRUE;
    }
}

#endif
