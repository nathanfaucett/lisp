#ifndef __LISP_LANG_LIST_H__
#define __LISP_LANG_LIST_H__


typedef struct lisp_ListNode {
    lisp_u32 ref_count;
    struct lisp_ListNode* next;
    struct lisp_Value* value;
} lisp_ListNode;

inline static lisp_ListNode* lisp_ListNode_new(lisp_ListNode*, lisp_ListNode*, struct lisp_Value*);
inline static void lisp_ListNode_delete(lisp_State*, lisp_ListNode*);

inline static void lisp_ListNode_ref(lisp_ListNode*);
inline static void lisp_ListNode_deref(lisp_State*, lisp_ListNode*);


typedef struct lisp_List {
    struct lisp_Value* value;
    lisp_ListNode* root;
    lisp_ListNode* tail;
    lisp_u32 size;
} lisp_List;

inline static lisp_List* lisp_List_new(lisp_List*, lisp_ListNode*, lisp_ListNode*, lisp_u32);
inline static void lisp_List_delete(lisp_State*, lisp_List*);

inline static lisp_ListNode* lisp_ListNode_find_node(lisp_List*, lisp_u32);
inline static struct lisp_Value* lisp_List_get_index(lisp_List*, lisp_u32);
inline static struct lisp_Value* lisp_List_get(lisp_State*, lisp_List*, struct lisp_Value*);

inline static struct lisp_Value* lisp_List_unshift(lisp_State*, lisp_List*, struct lisp_Value*);

inline static lisp_u8* lisp_List_to_cstring(lisp_List*);

#endif