#ifndef __LISP_LANG_STRING_H__
#define __LISP_LANG_STRING_H__


typedef struct lisp_String {
    lisp_size size;
    lisp_Value** chars;
} lisp_String;


static void lisp_String_alloc(lisp_State* state, lisp_Value* value);
static void lisp_String_dealloc(lisp_State* state, lisp_Value* value);
static void lisp_String_mark(lisp_Value* value);

static lisp_Value* lisp_String_new_ascii(lisp_State* state, lisp_char* ascii);
static lisp_char* lisp_String_to_ascii(lisp_String* string);

static lisp_bool lisp_String_equal(lisp_String* a, lisp_String* b);


#endif
