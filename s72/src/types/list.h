#ifndef S72_LIST_H
#define S72_LIST_H

#include "../s72.h"

// Pure libid list object structure (cons cell)
typedef struct t_List {
    oop first;     // First element (head) as oop
    oop rest;      // Rest of the list (tail) as oop - another list or nil
} *List;

// List type functions
void s72_list_init(void);
S72Value s72_list_empty(void);  // Create empty list (nil)
S72Value s72_list_cons(S72Value first, S72Value rest);  // Create cons cell
bool s72_is_list(S72Value val);

// List access functions
S72Value s72_list_first(S72Value list);
S72Value s72_list_rest(S72Value list);
bool s72_list_is_empty(S72Value list);
int s72_list_length(S72Value list);

// List methods (pure libid implementations with variadic signatures)
oop s72_list_is_empty_method(oop closure, oop state, oop self);
oop s72_list_first_method(oop closure, oop state, oop self);
oop s72_list_rest_method(oop closure, oop state, oop self);
oop s72_list_cons_method(oop closure, oop state, oop self, ...);
oop s72_list_length_method(oop closure, oop state, oop self);
oop s72_list_each_method(oop closure, oop state, oop self, ...);

// List object creation
oop s72_list_new_libid(S72Value first, S72Value rest);

#endif // S72_LIST_H
