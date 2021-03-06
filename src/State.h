#ifndef __LISP_STATE_H__
#define __LISP_STATE_H__


typedef struct lisp_State {
    struct lisp_GC* gc;

    struct lisp_Object* Type;

    struct lisp_Object* Any;
    struct lisp_Object* Nil;
    struct lisp_Object* Annotation;

    struct lisp_Object* Collection;
    struct lisp_Object* Indexed;
    struct lisp_Object* Keyed;

    struct lisp_Object* List;
    struct lisp_Object* ListNode;
    struct lisp_Object* Vector;
    struct lisp_Object* VectorNode;
    struct lisp_Object* Map;

    struct lisp_Object* Callable;
    struct lisp_Object* Native;
    struct lisp_Object* Function;
    struct lisp_Object* Macro;

    struct lisp_Object* Char;
    struct lisp_Object* Symbol;
    struct lisp_Object* Keyword;

    struct lisp_Object* Number;
    struct lisp_Object* Real;
    struct lisp_Object* Float;
    struct lisp_Object* Integer;
    struct lisp_Object* Signed;
    struct lisp_Object* Unsigned;

    struct lisp_Object* Float32;
    struct lisp_Object* Float64;

    struct lisp_Object* UInt;
    struct lisp_Object* UInt8;
    struct lisp_Object* UInt16;
    struct lisp_Object* UInt32;
    struct lisp_Object* UInt64;

    struct lisp_Object* Int;
    struct lisp_Object* Int8;
    struct lisp_Object* Int16;
    struct lisp_Object* Int32;
    struct lisp_Object* Int64;

    struct lisp_Object* Bool;
    struct lisp_Object* Reader;

    struct lisp_Object* nil;
    struct lisp_Object* tru;
    struct lisp_Object* fal;

    struct lisp_Object* AnyList;

    struct lisp_Object* AnyVector;
    struct lisp_Object* CharVector;
    struct lisp_Object* SymbolVector;
    struct lisp_Object* AnnotationVector;

    struct lisp_Object* AnyAnyMap;
    struct lisp_Object* AnyCallableMap;
    struct lisp_Object* SymbolAnnotationMap;
    struct lisp_Object* AnnotationTypeMap;
} lisp_State;

lisp_State* lisp_State_new(void);
void lisp_State_delete(lisp_State* state);

lisp_GCHeader* lisp_State_alloc(lisp_State* state, uintsize bytes);
void lisp_State_dealloc(lisp_State* state, lisp_GCHeader* gc_header);

void* lisp_State_assoc(lisp_State* state, lisp_GCHeader* gc_header, uintsize bytes);
void lisp_State_dissoc(lisp_State* state, lisp_GCHeader* gc_header, void* data);


#endif
