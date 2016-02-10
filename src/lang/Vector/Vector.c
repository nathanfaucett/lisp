#ifndef __LISP_LANG_VECTOR_C__
#define __LISP_LANG_VECTOR_C__


static void lisp_Vector_alloc(lisp_State* state, lisp_Object* object) {
    lisp_Vector* vector = (lisp_Vector*) object->data;
    vector->self = object;
    vector->root = NULL;
    vector->tail = NULL;
    vector->size = 0;
    vector->shift = LISP_VECTOR_SHIFT;
}
static void lisp_Vector_mark(lisp_Object* object) {
    lisp_Vector* vector = (lisp_Vector*) object->data;

    if (vector->root != NULL) {
        lisp_VectorNode_mark(vector->root);
    }
}

static lisp_Object* lisp_Vector_new(lisp_State* state) {
    lisp_Object* object = lisp_Object_boot_size(state, state->Vector, sizeof(lisp_Vector));
    lisp_Vector_alloc(state, object);
    return object;
}
static lisp_Object* lisp_Vector_clone(lisp_State* state, lisp_Vector* vector) {
    lisp_Object* object = lisp_Object_boot_size(state, state->Vector, sizeof(lisp_Vector));
    lisp_Vector* new_vector = (lisp_Vector*) object->data;
    new_vector->self = object;
    new_vector->root = vector->root;
    new_vector->tail = vector->tail;
    new_vector->size = vector->size;
    new_vector->shift = vector->shift;
    return object;
}

static void lisp_Vector_mut_set_size(lisp_State* state, lisp_Vector* vector, lisp_usize size) {
    
}

static lisp_usize lisp_Vector_tail_off(lisp_usize size) {
    if (size < LISP_VECTOR_SIZE) {
        return 0;
    } else {
        return ((size - 1) >> LISP_VECTOR_SHIFT) << LISP_VECTOR_SHIFT;
    }
}
static lisp_Object* lisp_Vector_find_node(lisp_Vector* vector, lisp_usize index) {
    if (index >= lisp_Vector_tail_off(vector->size)) {
        return vector->tail;
    } else {
        lisp_Object* node = vector->root;
        lisp_usize level = vector->shift;

        while (level > 0) {
            node = ((lisp_VectorNode*) node->data)->array[(index >> level) & LISP_VECTOR_MASK];
            level = level - LISP_VECTOR_SHIFT;
        }

        return node;
    }
}
static lisp_Object* lisp_Vector_get(lisp_State* state, lisp_Vector* vector, lisp_usize index) {
    if (index < vector->size) {
        return ((lisp_VectorNode*) lisp_Vector_find_node(vector, index)->data)->array[index & LISP_VECTOR_MASK];
    } else {
        return state->nil;
    }
}

static lisp_Object* lisp_Vector_new_path_set(
    lisp_State* state, lisp_Object* node_object, lisp_usize size, lisp_usize index, lisp_Object* value, lisp_usize level
) {
    lisp_Object* new_node_object = lisp_VectorNode_clone(state, node_object, ((size - 1) >> level) & LISP_VECTOR_MASK);
    lisp_VectorNode* new_node = (lisp_VectorNode*) new_node_object->data;

    if (level == 0) {
        new_node->array[index & LISP_VECTOR_MASK] = value;
    } else {
        lisp_usize sub_index = (index >> level) & LISP_VECTOR_MASK;
        new_node->array[sub_index] = lisp_Vector_new_path_set(
            state, ((lisp_VectorNode*) node_object->data)->array[sub_index], size, index, value, level - LISP_VECTOR_SHIFT
        );
    }

    return new_node_object;
}
static lisp_Object* lisp_Vector_set(lisp_State* state, lisp_Vector* vector, lisp_usize index, lisp_Object* value) {
    lisp_usize size = vector->size;

    if (index < size) {
        if (index >= lisp_Vector_tail_off(size)) {
            lisp_Object* tail_object = vector->tail;
            lisp_VectorNode* tail = (lisp_VectorNode*) tail_object->data;
            lisp_usize masked_index = index & LISP_VECTOR_MASK;

            if (lisp_Object_equal(state, tail->array[masked_index], value)) {
                return vector->self;
            } else {
                lisp_Object* new_vector_object = lisp_Vector_clone(state, vector);
                lisp_Vector* new_vector = (lisp_Vector*) new_vector_object->data;

                tail_object = lisp_VectorNode_clone(state, tail_object, (size + 1) & LISP_VECTOR_MASK);
                tail = (lisp_VectorNode*) tail_object->data;
                tail->array[masked_index] = value;
                new_vector->tail = tail_object;

                return new_vector_object;
            }
        } else if (lisp_Object_equal(state, lisp_Vector_get(state, vector, index), value)) {
            return vector->self;
        } else {
            lisp_Object* new_vector_object = lisp_Vector_clone(state, vector);
            lisp_Vector* new_vector = (lisp_Vector*) new_vector_object->data;
            new_vector->root = lisp_Vector_new_path_set(state, vector->root, size, index, value, vector->shift);
            return new_vector_object;
        }
    } else {
        return vector->self;
    }
}
static void lisp_Vector_mut_set(lisp_Vector* vector, lisp_usize index, lisp_Object* value) {
    lisp_Object* node_object = lisp_Vector_find_node(vector, index);

    if (node_object != NULL) {
        ((lisp_VectorNode*) node_object->data)->array[index & LISP_VECTOR_MASK] = value;
    }
}

static lisp_Object* lisp_Vector_new_path(lisp_State* state, lisp_Object* node_object, lisp_usize level) {
    if (level == 0) {
        return node_object;
    } else {
        lisp_Object* new_node_object = lisp_VectorNode_new(state);
        lisp_VectorNode* new_node = (lisp_VectorNode*) new_node_object->data;
        new_node->array[0] = lisp_Vector_new_path(state, node_object, level - LISP_VECTOR_SHIFT);
        return new_node_object;
    }
}
static lisp_Object* lisp_Vector_push_tail(
    lisp_State* state, lisp_Object* parent_object, lisp_Object* tail_object, lisp_usize size, lisp_usize level
) {
    lisp_usize sub_index = ((size - 1) >> level) & LISP_VECTOR_MASK;
    lisp_Object* new_node_object = lisp_VectorNode_new(state);
    lisp_Object* node_to_insert = NULL;

    if (level == LISP_VECTOR_SHIFT) {
        node_to_insert = tail_object;
    } else {
        lisp_Object* child = ((lisp_VectorNode*) parent_object->data)->array[sub_index];

        if (child == NULL) {
            node_to_insert = lisp_Vector_new_path(state, tail_object, level - LISP_VECTOR_SHIFT);
        } else {
            node_to_insert = lisp_Vector_push_tail(state, child, tail_object, size, level - LISP_VECTOR_SHIFT);
        }
    }

    ((lisp_VectorNode*) new_node_object->data)->array[sub_index] = node_to_insert;

    return new_node_object;
}
static void lisp_Vector_mut_push(lisp_State* state, lisp_Vector* vector, lisp_Object* value) {
    lisp_Object* root = vector->root;
    lisp_usize size = vector->size;
    lisp_usize shift = vector->shift;

    if (size - lisp_Vector_tail_off(size) < LISP_VECTOR_SIZE) {
        if (vector->tail == NULL) {
            vector->tail = lisp_VectorNode_new(state);
        }
        ((lisp_VectorNode*) vector->tail->data)->array[size & LISP_VECTOR_MASK] = value;
    } else {
        lisp_Object* new_root_object = NULL;
        lisp_VectorNode* new_root = NULL;
        lisp_usize new_shift = shift;

        if ((size >> LISP_VECTOR_SHIFT) > (1 << shift)) {
            new_root_object = lisp_VectorNode_new(state);
            new_root = (lisp_VectorNode*) new_root_object->data;
            new_root->array[0] = root;
            new_root->array[1] = lisp_Vector_new_path(state, vector->tail, shift);
            new_shift += LISP_VECTOR_SHIFT;
        } else {
            new_root_object = lisp_Vector_push_tail(state, root, vector->tail, size, shift);
            new_root = (lisp_VectorNode*) new_root_object->data;
        }

        lisp_Object* new_tail_object = lisp_VectorNode_new(state);
        ((lisp_VectorNode*) new_tail_object->data)->array[0] = value;
        vector->tail = new_tail_object;

        vector->root = new_root_object;
        vector->shift = new_shift;
    }

    vector->size = size + 1;
}
static lisp_Object* lisp_Vector_push(lisp_State* state, lisp_Vector* vector, lisp_Object* value) {
    lisp_usize size = vector->size;
    lisp_Object* new_vector_object = lisp_Vector_clone(state, vector);
    lisp_Vector* new_vector = (lisp_Vector*) new_vector_object->data;

    if (new_vector->tail == NULL) {
        new_vector->tail = lisp_VectorNode_new(state);
    } else if (size - lisp_Vector_tail_off(size) < LISP_VECTOR_SIZE) {
        new_vector->tail = lisp_VectorNode_clone(state, vector->tail, (size + 1) & LISP_VECTOR_MASK);
    }

    lisp_Vector_mut_push(state, new_vector, value);

    return new_vector_object;
}

static lisp_Object* lisp_Vector_pop_tail(lisp_State* state, lisp_Object* node_object, lisp_usize size, lisp_usize level) {
    lisp_usize sub_index = ((size - 1) >> level) & LISP_VECTOR_MASK;

    if (level > LISP_VECTOR_SHIFT) {
        lisp_Object* new_child_object = lisp_Vector_pop_tail(
            state, ((lisp_VectorNode*) node_object->data)->array[sub_index], size, level - LISP_VECTOR_SHIFT
        );

        if (new_child_object == NULL) {
            return NULL;
        } else {
            lisp_Object* new_node_object = lisp_VectorNode_clone(state, node_object, sub_index);
            ((lisp_VectorNode*) new_node_object->data)->array[sub_index] = new_child_object;
            return new_node_object;
        }
    } else if (sub_index == 0) {
        return NULL;
    } else {
        return lisp_VectorNode_clone(state, node_object, sub_index);
    }
}
static lisp_Object* lisp_Vector_pop(lisp_State* state, lisp_Vector* vector) {
    lisp_usize size = vector->size;

    if (size == 0) {
        return vector->self;
    } else if (size == 1) {
        return state->empty_vector;
    } else {
        lisp_Object* new_vector_object = lisp_Vector_new(state);
        lisp_Vector* new_vector = (lisp_Vector*) new_vector_object->data;

        lisp_Object* new_root_object = NULL;
        lisp_Object* new_tail_object = NULL;
        lisp_usize new_shift;

        if ((size - lisp_Vector_tail_off(size)) > 1) {
            new_tail_object = lisp_VectorNode_clone(state, vector->tail, (size - 1) & LISP_VECTOR_MASK);
            new_root_object = vector->root;
            new_shift = vector->shift;
        } else {
            new_tail_object = lisp_Vector_find_node(vector, size - 2);

            lisp_usize shift = vector->shift;
            new_root_object = lisp_Vector_pop_tail(state, vector->root, size, shift);
            new_shift = shift;

            if (new_root_object != NULL) {
                lisp_VectorNode* new_root = (lisp_VectorNode*) new_root_object->data;

                if (shift > LISP_VECTOR_SHIFT && new_root->array[1] == NULL) {
                    new_root_object = new_root->array[0];
                    new_shift -= LISP_VECTOR_SHIFT;
                }
            }
        }

        new_vector->root = new_root_object;
        new_vector->tail = new_tail_object;
        new_vector->size = size - 1;
        new_vector->shift = new_shift;

        return new_vector_object;
    }
}

static lisp_Object* lisp_Vector_unshift(lisp_State* state, lisp_Vector* vector, lisp_Object* value) {
    lisp_usize size = vector->size;

    if (size != 0) {
        lisp_Object* array_object = lisp_Array_new(state);
        lisp_Array* array = (lisp_Array*) array_object->data;
        lisp_Array_set_size(state, array, size + 1);

        lisp_usize i = 0, il = size, j = 1;
        for (; i < il; i++, j++) {
            lisp_Array_set(array, j, lisp_Vector_get(state, vector, j));
        }
        lisp_Array_set(array, 0, value);

        lisp_Object* new_vector_object = lisp_Vector_new(state);
        lisp_Vector* new_vector = (lisp_Vector*) new_vector_object->data;

        for (i = 0, il = size + 1; i < il; i++) {
            lisp_Vector_mut_push(state, new_vector, lisp_Array_get(state, array, i));
        }
        lisp_State_dealloc(state, array_object);

        return new_vector_object;
    } else {
        return lisp_Vector_push(state, vector, value);
    }
}

static lisp_Object* lisp_Vector_remove(lisp_State* state, lisp_Vector* vector, lisp_usize index) {
    lisp_usize size = vector->size;

    if (index < size) {
        lisp_Object* array_object = lisp_Array_new(state);
        lisp_Array* array = (lisp_Array*) array_object->data;
        lisp_Array_set_size(state, array, size);

        lisp_usize i = 0, il = index, j = 0;
        for (; i < il; i++, j++) {
            lisp_Array_set(array, j, lisp_Vector_get(state, vector, i));
        }
        for (i = index + 1, il = size; i < il; i++, j++) {
            lisp_Array_set(array, j, lisp_Vector_get(state, vector, i));
        }

        lisp_Object* new_vector_object = lisp_Vector_new(state);
        lisp_Vector* new_vector = (lisp_Vector*) new_vector_object->data;

        for (i = 0, il = size - 1; i < il; i++) {
            lisp_Vector_mut_push(state, new_vector, lisp_Array_get(state, array, i));
        }
        lisp_State_dealloc(state, array_object);

        return new_vector_object;
    } else {
        return vector->self;
    }
}
static void lisp_Vector_mut_remove(lisp_State* state, lisp_Vector* vector, lisp_usize index) {
    lisp_usize size = vector->size;

    if (index < size) {
        lisp_Object* array_object = lisp_Array_new(state);
        lisp_Array* array = (lisp_Array*) array_object->data;
        lisp_Array_set_size(state, array, size);

        lisp_usize i = 0, il = index, j = 0;
        for (; i < il; i++, j++) {
            lisp_Array_set(array, j, lisp_Vector_get(state, vector, i));
        }
        for (i = index + 1, il = size; i < il; i++, j++) {
            lisp_Array_set(array, j, lisp_Vector_get(state, vector, i));
        }

        vector->root = NULL;
        vector->tail = NULL;
        vector->size = 0;
        vector->shift = LISP_VECTOR_SHIFT;

        for (i = 0, il = size - 1; i < il; i++) {
            lisp_Vector_mut_push(state, vector, lisp_Array_get(state, array, i));
        }
        lisp_State_dealloc(state, array_object);
    }
}

static lisp_bool lisp_Vector_equal(lisp_State* state, lisp_Vector* a, lisp_Vector* b) {
    if (a == b) {
        return LISP_TRUE;
    } else if (a->size == b->size) {
        lisp_usize i = 0, il = a->size;

        for (; i < il; i++) {
            if (!lisp_Object_equal(state, lisp_Vector_get(state, a, i), lisp_Vector_get(state, b, i))) {
                return LISP_FALSE;
            }
        }

        return LISP_TRUE;
    } else {
        return LISP_FALSE;
    }
}


#endif
