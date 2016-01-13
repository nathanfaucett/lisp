#ifndef __LISP_LANG_NUMBER_C__
#define __LISP_LANG_NUMBER_C__


inline static lisp_Number* lisp_Number_new(lisp_Number* number, lisp_f64 float_value) {
    number->float_value = float_value;
    return number;
}

inline static lisp_u8* lisp_Number_to_cstring(lisp_Number* number) {
    return str_clone("Number");
}


#endif