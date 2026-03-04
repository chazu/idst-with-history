#ifndef S72_LIST_H
#define S72_LIST_H

#include "../s72.h"

// Forward declaration for circular reference
struct S72Value;

// List type structure (cons cell)
typedef struct {
    S72Value first;     // First element (head)
    S72Value rest;      // Rest of the list (tail) - another list or nil
} S72List;

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

// List methods (native implementations)
oop s72_list_is_empty_method(oop closure, oop state, oop receiver);
oop s72_list_first_method(oop closure, oop state, oop receiver);
oop s72_list_rest_method(oop closure, oop state, oop receiver);
oop s72_list_cons_method(oop closure, oop state, oop receiver, ...);
oop s72_list_length_method(oop closure, oop state, oop receiver);
oop s72_list_each_method(oop closure, oop state, oop receiver, ...);

#endif // S72_LIST_H
