#ifndef __LISP_LANG_ANNOTATION_C__
#define __LISP_LANG_ANNOTATION_C__


static void lisp_Annotation_boot(lisp_State* state) {
    lisp_Value* Annotation = state->Annotation;
    lisp_Array* attributes = (lisp_Array*) lisp_Array_get(state, (lisp_Array*) Annotation->values->data, LISP_IDX_TYPE_ATTRIBUTES)->data;

    lisp_Array_set(attributes, 0, lisp_Symbol_from_ascii(state, "type"));
    lisp_Array_set(attributes, 1, lisp_Symbol_from_ascii(state, "template"));
}


#endif