#ifndef __LISP_LANG_VALUE_C__
#define __LISP_LANG_VALUE_C__


static lisp_Value* lisp_Value_alloc(lisp_State* state, lisp_Value* Type) {
    lisp_Value* value = lisp_State_alloc(state, sizeof(lisp_Value));
    lisp_Type* type = (lisp_Type*) Type->data;
    lisp_Array* type_values = (lisp_Array*) Type->values->data;

    value->marked = LISP_FALSE;
    value->type = Type;

    lisp_Value* size_value = lisp_Array_get(state, type_values, LISP_IDX_TYPE_SIZE);
    lisp_size size = LISP_GET_DATA(size_value, lisp_size);
    if (size != 0) {
        value->data = lisp_State_alloc(state, size);
    } else {
        value->data = NULL;
    }

    lisp_Value* attributes_value = lisp_Array_get(state, type_values, LISP_IDX_TYPE_ATTRIBUTES);
    lisp_Value* values = lisp_boot_new_array(state);
    lisp_Array_set_size(state, (lisp_Array*) values->data, ((lisp_Array*) attributes_value->data)->size);
    value->values = values;

    if (type->alloc != NULL) {
        type->alloc(state, value);
    }

    return value;
}
static void lisp_Value_dealloc(lisp_State* state, lisp_Value* value) {
    lisp_Type* type = (lisp_Type*) value->type->data;

    if (type->dealloc != NULL) {
        type->dealloc(state, value);
    }

    lisp_State_dealloc(state, value);
}
static void lisp_Value_mark(lisp_Value* value) {
    if (value->marked == LISP_FALSE) {
        lisp_Type* type = (lisp_Type*) value->type->data;

        value->marked = LISP_TRUE;

        if (value->type != NULL) {
            lisp_Value_mark(value->type);
        }
        if (value->values != NULL) {
            lisp_Value_mark(value->values);
        }
        if (value->template != NULL) {
            lisp_Value_mark(value->template);
        }


        if (type->mark != NULL) {
            type->mark(value);
        }
    }
}

static lisp_bool lisp_Value_equal(lisp_State* state, lisp_Value* a, lisp_Value* b) {
    return a == b;
}


#endif
