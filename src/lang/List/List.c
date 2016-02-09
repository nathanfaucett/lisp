#ifndef __LISP_LANG_LIST_C__
#define __LISP_LANG_LIST_C__


static void lisp_List_alloc(lisp_State* state, lisp_Object* object) {
    lisp_List* list = (lisp_List*) object->data;
    list->self = object;
    list->root = NULL;
    list->tail = NULL;
    list->size = 0;
}
static void lisp_List_mark(lisp_Object* object) {
    lisp_List* list = (lisp_List*) object->data;

    if (list->root != NULL) {
        lisp_ListNode_mark(list->root);
    }
}

static lisp_Object* lisp_List_new(lisp_State* state) {
    lisp_Object* object = lisp_Object_boot_size(state, state->List, sizeof(lisp_List));
    lisp_List_alloc(state, object);
    return object;
}

static lisp_Object* lisp_List_find_node(lisp_List* list, lisp_size index) {
    if (index == 0) {
        return list->root;
    } else if (index == list->size - 1) {
        return list->tail;
    } else if (index < list->size) {
        return lisp_ListNode_find_node(list->root, index);
    } else {
        return NULL;
    }
}
static lisp_size lisp_List_index_of(lisp_State* state, lisp_List* list, lisp_Object* key) {
    lisp_Object* node_object = list->root;
    lisp_ListNode* node = NULL;
    lisp_size index = 1;

    while (node_object != NULL) {
        node = (lisp_ListNode*) node_object->data;

        if (lisp_Object_equal(state, node->object, key)) {
            return index;
        } else {
            index += 1;
            node_object = node->next;
        }
    }

    return 0;
}

static lisp_Object* lisp_List_concat(lisp_State* state, lisp_List* a, lisp_List* b) {
    if (a->size == 0) {
        return b->self;
    } else if (b->size == 0) {
        return a->self;
    } else {
        lisp_Object* object = lisp_List_new(state);
        lisp_List* list = (lisp_List*) object->data;

        lisp_Object* node_object = a->root;
        lisp_ListNode* node = NULL;

        while (node_object != NULL) {
            node = (lisp_ListNode*) node_object->data;
            lisp_List_mut_push(state, node->value);
            node_object = node->next;
        }
        node_object = b->root;
        while (node_object != NULL) {
            node = (lisp_ListNode*) node_object->data;
            lisp_List_mut_push(state, node->value);
            node_object = node->next;
        }

        return object;
    }
}

static void lisp_List_mut_set_size(lisp_State* state, lisp_List* list, lisp_size size) {
    lisp_Object* tail_object = NULL;
    lisp_Object* node_object = NULL;

    lisp_size i = 0, il = size;
    for (; i < il; i++) {
        node_object = lisp_ListNode_new(state);
        ((lisp_ListNode*) node_object->data)->object = state->nil;

        if (tail_object != NULL) {
            ((lisp_ListNode*) tail_object->data)->next = node_object;
        } else {
            list->root = node_object;
        }
        tail_object = node_object;
    }

    list->tail = tail_object;
    list->size = size;
}

static lisp_Object* lisp_List_get(lisp_State* state, lisp_List* list, lisp_size index) {
    lisp_Object* node = lisp_List_find_node(list, index);

    if (node != NULL) {
        return ((lisp_ListNode*) node->data)->object;
    } else {
        return state->nil;
    }
}
static lisp_Object* lisp_List_set(lisp_State* state, lisp_List* list, lisp_size index, lisp_Object* object) {
    if (index < list->size) {
        lisp_Object* node_object = lisp_List_find_node(list, index);
        lisp_ListNode* node = (lisp_ListNode*) node_object->data;

        if (lisp_Object_equal(state, node->object, object)) {
            return list->self;
        } else {
            lisp_Object* new_list_object = lisp_Object_alloc(state, state->List);
            lisp_List* new_list = (lisp_List*) new_list_object->data;

            lisp_Object* new_node_object = lisp_ListNode_new(state);
            lisp_ListNode* new_node = (lisp_ListNode*) new_node_object->data;

            new_node->next = node->next;
            new_node->object = object;

            new_list->root = lisp_ListNode_copy_from_to(state, list->root, node_object, new_node_object);
            new_list->tail = node->next == NULL ? new_node_object : list->tail;
            new_list->size = list->size;

            return new_list_object;
        }
    } else {
        return list->self;
    }
}
static void lisp_List_mut_set(lisp_List* list, lisp_size index, lisp_Object* object) {
    if (index < list->size) {
        lisp_Object* node_object = lisp_List_find_node(list, index);
        lisp_ListNode* node = (lisp_ListNode*) node_object->data;
        node->object = object;
    }
}

static lisp_Object* lisp_List_push(lisp_State* state, lisp_List* list, lisp_Object* object) {
    lisp_Object* new_list_object = lisp_Object_alloc(state, state->List);
    lisp_List* new_list = (lisp_List*) new_list_object->data;

    lisp_Object* node_object = lisp_ListNode_new(state);
    lisp_ListNode* node = (lisp_ListNode*) node_object->data;
    node->next = NULL;
    node->object = object;

    if (list->root != NULL) {
        new_list->root = lisp_ListNode_push(state, list->root, node_object);
        new_list->tail = node_object;
        new_list->size = list->size + 1;
    } else {
        new_list->root = node_object;
        new_list->tail = node_object;
        new_list->size = 1;
    }

    return new_list_object;
}
static void lisp_List_mut_push(lisp_State* state, lisp_List* list, lisp_Object* object) {
    lisp_Object* node_object = lisp_ListNode_new(state);

    if (list->root != NULL) {
        ((lisp_ListNode*) list->tail->data)->next = node_object;
        list->tail = node_object;
    } else {
        list->root = node_object;
        list->tail = node_object;
    }

    list->size += 1;
}

static lisp_Object* lisp_List_unshift(lisp_State* state, lisp_List* list, lisp_Object* object) {
    lisp_Object* new_list_object = lisp_Object_alloc(state, state->List);
    lisp_List* new_list = (lisp_List*) new_list_object->data;

    lisp_Object* node_object = lisp_ListNode_new(state);
    lisp_ListNode* node = (lisp_ListNode*) node_object->data;
    node->object = object;

    if (list->root != NULL) {
        node->next = list->root;
        new_list->tail = list->tail;
        new_list->size = list->size + 1;
    } else {
        new_list->tail = node_object;
        new_list->size = 1;
    }

    new_list->root = node_object;

    return new_list_object;
}

static lisp_Object* lisp_List_pop(lisp_State* state, lisp_List* list) {
    if (list->size > 1) {
        lisp_Object* new_list_object = lisp_Object_alloc(state, state->List);
        lisp_List* new_list = (lisp_List*) new_list_object->data;

        lisp_Object* tail_object = list->tail;

        lisp_Object* root_object = list->root;
        lisp_ListNode* root = (lisp_ListNode*) root_object->data;

        lisp_Object* new_root_object = lisp_ListNode_new(state);
        lisp_ListNode* new_root = (lisp_ListNode*) new_root_object->data;
        new_root->object = root->object;

        lisp_Object* new_tail_object = new_root_object;

        lisp_ListNode* new_tail = NULL;
        lisp_Object* tmp_object = NULL;
        lisp_ListNode* tmp = NULL;

        while (LISP_TRUE) {
            root_object = root->next;

            if (root_object == NULL || root_object == tail_object) {
                break;
            } else {
                root = (lisp_ListNode*) root_object->data;

                tmp_object = lisp_ListNode_new(state);
                tmp = (lisp_ListNode*) tmp_object->data;

                tmp->object = root->object;

                new_tail = (lisp_ListNode*) new_tail_object->data;
                new_tail->next = tmp_object;

                new_tail_object = tmp_object;
            }
        }

        new_list->root = new_root_object;
        new_list->tail = new_tail_object;
        new_list->size = list->size - 1;

        return new_list_object;
    } else {
        return state->empty_list;
    }
}
static lisp_Object* lisp_List_shift(lisp_State* state, lisp_List* list) {
    if (list->size > 1) {
        lisp_Object* new_list_object = lisp_Object_alloc(state, state->List);
        lisp_List* new_list = (lisp_List*) new_list_object->data;

        new_list->root = ((lisp_ListNode*) list->root)->next;
        new_list->tail = list->tail;
        new_list->size = list->size - 1;

        return new_list_object;
    } else {
        return state->empty_list;
    }
}
static lisp_Object* lisp_List_remove(lisp_State* state, lisp_List* list, lisp_size index) {
    if (index == 0) {
        return lisp_List_shift(state, list);
    } else if (index == list->size - 1) {
        return lisp_List_pop(state, list);
    } else if (index < list->size) {
        lisp_Object* new_list_object = lisp_Object_alloc(state, state->List);
        lisp_List* new_list = (lisp_List*) new_list_object->data;

        lisp_Object* removed_node = lisp_ListNode_find_node(list->root, index);
        lisp_Object* new_root = lisp_ListNode_copy_from_to(state, list->root, removed_node, ((lisp_ListNode*) removed_node->data)->next);

        new_list->root = new_root;
        new_list->tail = list->tail;
        new_list->size = list->size - 1;

        return new_list_object;
    } else {
        return list->self;
    }
}

static void lisp_List_mut_pop(lisp_List* list) {
    if (list->size > 1) {
        list->tail = lisp_List_find_node(list, list->size - 2);
        list->size -= 1;
    } else if (list->size == 1) {
        list->root = NULL;
        list->tail = NULL;
        list->size = 0;
    }
}
static void lisp_List_mut_shift(lisp_List* list) {
    if (list->size > 1) {
        list->root = ((lisp_ListNode*) list->root)->next;
        list->size -= 1;
    } else if (list->size == 1) {
        list->root = NULL;
        list->tail = NULL;
        list->size = 0;
    }
}
static void lisp_List_mut_remove(lisp_List* list, lisp_size index) {
    if (index == 0) {
        lisp_List_mut_shift(list);
    } else if (index == list->size - 1) {
        lisp_List_mut_pop(list);
    } else if (index < list->size) {
        lisp_Object* node_object = lisp_List_find_node(list, index - 1);
        lisp_ListNode* node = (lisp_ListNode*) node_object->data;
        node->next = ((lisp_ListNode*) node->next->data)->next;
        list->size -= 1;
    }
}

static lisp_bool lisp_List_equal(lisp_State* state, lisp_List* a, lisp_List* b) {
    if (a == b) {
        return LISP_TRUE;
    } else if (a->size != b->size) {
        return LISP_FALSE;
    } else {
        lisp_Object* anode_object = a->root;
        lisp_Object* bnode_object = b->root;
        lisp_ListNode* anode = NULL;
        lisp_ListNode* bnode = NULL;

        while (anode_object != NULL) {
            anode = (lisp_ListNode*) anode_object->data;
            bnode = (lisp_ListNode*) bnode_object->data;

            if (lisp_Object_equal(state, anode->object, bnode->object)) {
                anode_object = anode->next;
                bnode_object = bnode->next;
            } else {
                return LISP_FALSE;
            }
        }

        return LISP_TRUE;
    }
}

static lisp_Object* lisp_List_export_index_of(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);
    lisp_Object* value = lisp_List_get(state, list, 1);

    if (self->type == state->List) {
        return lisp_Number_Int(state, ((lisp_isize) lisp_List_index_of(state, (lisp_List*) self->data, value)) - 1);
    } else {
        return self;
    }
}

static lisp_Object* lisp_List_export_size(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);

    if (self->type == state->List) {
        return lisp_Number_UInt(state, ((lisp_List*) self->data)->size);
    } else {
        return self;
    }
}

static lisp_Object* lisp_List_export_get(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);
    lisp_Object* index = lisp_List_get(state, list, 1);

    if (self->type == state->List) {
        return lisp_List_get(state, (lisp_List*) self->data, lisp_Number_get_UInt(state, index));
    } else {
        return self;
    }
}
static lisp_Object* lisp_List_export_set(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);
    lisp_Object* index = lisp_List_get(state, list, 1);
    lisp_Object* value = lisp_List_get(state, list, 2);

    if (self->type == state->List) {
        return lisp_List_set(state, (lisp_List*) self->data, lisp_Number_get_UInt(state, index), value);
    } else {
        return self;
    }
}

static lisp_Object* lisp_List_export_push(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);
    lisp_Object* value = lisp_List_get(state, list, 1);

    if (self->type == state->List) {
        return lisp_List_push(state, (lisp_List*) self->data, value);
    } else {
        return self;
    }
}

static lisp_Object* lisp_List_export_unshift(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);
    lisp_Object* value = lisp_List_get(state, list, 1);

    if (self->type == state->List) {
        return lisp_List_unshift(state, (lisp_List*) self->data, value);
    } else {
        return self;
    }
}

static lisp_Object* lisp_List_export_pop(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_Object* self = lisp_List_get(state, (lisp_List*) args->data, 0);

    if (self->type == state->List) {
        return lisp_List_pop(state, (lisp_List*) self->data);
    } else {
        return self;
    }
}
static lisp_Object* lisp_List_export_shift(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_Object* self = lisp_List_get(state, (lisp_List*) args->data, 0);

    if (self->type == state->List) {
        return lisp_List_shift(state, (lisp_List*) self->data);
    } else {
        return self;
    }
}
static lisp_Object* lisp_List_export_remove(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);
    lisp_Object* index = lisp_List_get(state, list, 1);

    if (self->type == state->List) {
        return lisp_List_remove(state, (lisp_List*) self->data, lisp_Number_get_UInt(state, index));
    } else {
        return self;
    }
}

static lisp_Object* lisp_List_export_to_string(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_Object* self = lisp_List_get(state, (lisp_List*) args->data, 0);

    if (self->type == state->List) {
        return lisp_String_from_ascii(state, "()");
    } else {
        return lisp_Object_to_string(state, self);
    }
}
static lisp_Object* lisp_List_export_equal(lisp_State* state, lisp_Object* args, lisp_Object* scope) {
    lisp_List* list = (lisp_List*) args->data;
    lisp_Object* self = lisp_List_get(state, list, 0);
    lisp_Object* other = lisp_List_get(state, list, 1);

    if (self->type == state->List && other->type == state->List) {
        return lisp_List_equal(state, (lisp_List*) self->data, (lisp_List*) other->data) ? state->true : state->false;
    } else {
        return lisp_Object_equal(state, self, other) ? state->true : state->false;
    }
}

static void lisp_List_boot(lisp_State* state) {
    lisp_Object* List = state->List;
    lisp_List* values = (lisp_List*) List->values->data;
    lisp_Map* prototype = (lisp_Map*) lisp_List_get(state, values, LISP_IDX_TYPE_PROTOTYPE)->data;

    lisp_List_mut_set(values, LISP_IDX_TYPE_NAME, lisp_String_from_ascii(state, "List"));

    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "index-of"), lisp_Native_new(state, lisp_List_export_index_of));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "size"), lisp_Native_new(state, lisp_List_export_to_size));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "get"), lisp_Native_new(state, lisp_List_export_get));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "set"), lisp_Native_new(state, lisp_List_export_set));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "push"), lisp_Native_new(state, lisp_List_export_push));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "unshift"), lisp_Native_new(state, lisp_List_export_unshift));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "pop"), lisp_Native_new(state, lisp_List_export_pop));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "shift"), lisp_Native_new(state, lisp_List_export_shift));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "remove"), lisp_Native_new(state, lisp_List_export_remove));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "to-string"), lisp_Native_new(state, lisp_List_export_to_string));
    lisp_Map_mut_set(state, prototype, lisp_Symbol_from_ascii(state, "equal"), lisp_Native_new(state, lisp_List_export_equal));
}


#endif
