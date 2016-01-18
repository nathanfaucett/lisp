#ifndef __LISP_LANG_FUNCTION_C__
#define __LISP_LANG_FUNCTION_C__


static lisp_Function* lisp_Function_constructor(
    lisp_Function* fn, lisp_Value* name, lisp_Value* params, lisp_Value* body
) {
    fn->name = name;
    fn->params = params;
    fn->body = body;

    fn->symbol = NULL;
    fn->native = NULL;

    return fn;
}

static void lisp_Function_destructor(lisp_State* state, lisp_Function* fn) {
    if (fn->symbol != NULL) {
        lisp_Value_deref(state, fn->symbol);
    } else {
        lisp_Value_deref(state, fn->name);
        lisp_Value_deref(state, fn->params);
        lisp_Value_deref(state, fn->body);
    }
}

static lisp_Value* lisp_Function_call(lisp_State* state, lisp_Function* fn, lisp_Value* values, lisp_Scope* scope) {
    if (fn->native != NULL) {
        lisp_MutList* mut_list = lisp_MutList_new();
        lisp_ListNode* node = values->list.root;

        while (node != NULL) {
            lisp_MutList_push(mut_list, node->value);
            node = node->next;
        }

        lisp_Value* new_list = lisp_Value_list_from_mut_list(state, mut_list);
        lisp_MutList_delete(mut_list);

        lisp_Value* value = fn->native(state, new_list, scope);

        return value;
    } else {
        lisp_Scope* fn_scope = lisp_Scope_new(state, scope);
        lisp_bool rest = LISP_FALSE;

        lisp_Scope_def(fn_scope, fn->name, fn->value);

        for (lisp_u32 i = 0, il = lisp_Vector_size(&fn->params->vector); i < il; i++) {
            lisp_Value* param = lisp_Vector_get(state, &fn->params->vector, i);

            if (rest) {
                lisp_Value* new_list = lisp_List_after(state, &values->list, i - 1);
                lisp_Value* new_vector = lisp_Vector_from_list(state, &new_list->list);
                lisp_Scope_def(fn_scope, param, lisp_State_eval(state, new_vector, scope));
                break;
            } else {
                lisp_u8* cstring = lisp_String_to_cstring(&param->symbol.string->string);

                if (lisp_cstring_equal(cstring, "...")) {
                    rest = LISP_TRUE;
                } else {
                    lisp_Scope_def(fn_scope, param, lisp_State_eval(state, lisp_List_get(state, &values->list, i), scope));
                }

                free(cstring);
            }
        }

        lisp_Value* value = lisp_State_eval(state, fn->body, fn_scope);
        lisp_Scope_delete(fn_scope);

        return value;
    }
}

static struct lisp_Value* lisp_Function_to_string(lisp_State* state, lisp_Function* fn) {
    return lisp_Value_to_string(state, fn->name);
}

static lisp_bool lisp_Function_equal(lisp_Function* a, lisp_Function* b) {
    return a == b;
}


#endif
