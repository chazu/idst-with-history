#include "list.h"
#include "block.h"
#include <stdio.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern oop s72_list_vtable;
extern oop s72_nil_vtable;
extern S72Value S72_NIL;

// Global selectors for list operations
static oop SEL_IS_EMPTY = NULL;
static oop SEL_FIRST = NULL;
static oop SEL_REST = NULL;
static oop SEL_CONS = NULL;
static oop SEL_LENGTH = NULL;
static oop SEL_EACH = NULL;

// List type initialization
void s72_list_init(void) {
    // Intern selectors
    SEL_IS_EMPTY = _libid->intern("isEmpty");
    SEL_FIRST = _libid->intern("first");
    SEL_REST = _libid->intern("rest");
    SEL_CONS = _libid->intern("cons:");
    SEL_LENGTH = _libid->intern("length");
    SEL_EACH = _libid->intern("each:");

    // Install methods on list vtable
    S72_METHOD(s72_list_vtable, SEL_IS_EMPTY, s72_list_is_empty_method);
    S72_METHOD(s72_list_vtable, SEL_FIRST, s72_list_first_method);
    S72_METHOD(s72_list_vtable, SEL_REST, s72_list_rest_method);
    S72_METHOD(s72_list_vtable, SEL_CONS, s72_list_cons_method);
    S72_METHOD(s72_list_vtable, SEL_LENGTH, s72_list_length_method);
    S72_METHOD(s72_list_vtable, SEL_EACH, s72_list_each_method);

    // Also install list methods on nil vtable since nil represents empty list
    S72_METHOD(s72_nil_vtable, SEL_IS_EMPTY, s72_list_is_empty_method);
    S72_METHOD(s72_nil_vtable, SEL_FIRST, s72_list_first_method);
    S72_METHOD(s72_nil_vtable, SEL_REST, s72_list_rest_method);
    S72_METHOD(s72_nil_vtable, SEL_CONS, s72_list_cons_method);
    S72_METHOD(s72_nil_vtable, SEL_LENGTH, s72_list_length_method);
    S72_METHOD(s72_nil_vtable, SEL_EACH, s72_list_each_method);

    printf("DEBUG: Installed list methods on vtable %p and nil vtable %p\n", s72_list_vtable, s72_nil_vtable);
}

// Create empty list (nil represents empty list)
S72Value s72_list_empty(void) {
    return S72_NIL;
}

// Create cons cell
S72Value s72_list_cons(S72Value first, S72Value rest) {
    printf("DEBUG: s72_list_cons - first=%p, rest=%p\n", first.obj, rest.obj);
    
    // Allocate list object
    oop list_oop = S72_ALLOC(s72_list_vtable, sizeof(S72List));
    if (!list_oop) {
        s72_error("Failed to allocate list");
        return S72_NIL;
    }
    
    S72List *list = (S72List *)list_oop;
    list->first = first;
    list->rest = rest;
    
    printf("DEBUG: Created list cons cell at %p\n", list_oop);
    
    S72Value result = {list_oop};
    return result;
}

// List type checking
bool s72_is_list(S72Value val) {
    // nil is considered an empty list
    if (s72_is_nil(val)) return true;
    
    if (!val.obj) return false;

    // Get the vtable from the libid object
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];

    // Compare with list vtable
    oop expected_vtable = s72_list_vtable->_vtable[-1];
    
    return obj_vtable == expected_vtable;
}

// List access functions
S72Value s72_list_first(S72Value list) {
    if (s72_is_nil(list)) {
        s72_error("Cannot get first of empty list");
        return S72_NIL;
    }
    
    if (!s72_is_list(list)) {
        s72_error("Value is not a list");
        return S72_NIL;
    }
    
    S72List *l = (S72List *)list.obj;
    return l->first;
}

S72Value s72_list_rest(S72Value list) {
    if (s72_is_nil(list)) {
        return S72_NIL;  // Rest of empty list is empty list
    }
    
    if (!s72_is_list(list)) {
        s72_error("Value is not a list");
        return S72_NIL;
    }
    
    S72List *l = (S72List *)list.obj;
    return l->rest;
}

bool s72_list_is_empty(S72Value list) {
    return s72_is_nil(list);
}

int s72_list_length(S72Value list) {
    if (s72_is_nil(list)) return 0;
    
    if (!s72_is_list(list)) {
        s72_error("Value is not a list");
        return 0;
    }
    
    int count = 0;
    S72Value current = list;
    
    while (!s72_is_nil(current)) {
        if (!s72_is_list(current)) {
            s72_error("Improper list structure");
            return count;
        }
        count++;
        current = s72_list_rest(current);
    }
    
    return count;
}

// List method implementations

// isEmpty - check if list is empty
oop s72_list_is_empty_method(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_list_is_empty_method called - receiver=%p\n", receiver);
    
    S72Value list_val = {receiver};
    bool empty = s72_list_is_empty(list_val);
    
    return empty ? S72_TRUE.obj : S72_FALSE.obj;
}

// first - get first element
oop s72_list_first_method(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_list_first_method called - receiver=%p\n", receiver);
    
    S72Value list_val = {receiver};
    S72Value first = s72_list_first(list_val);
    
    return first.obj;
}

// rest - get rest of list
oop s72_list_rest_method(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_list_rest_method called - receiver=%p\n", receiver);
    
    S72Value list_val = {receiver};
    S72Value rest = s72_list_rest(list_val);
    
    return rest.obj;
}

// cons: - create new list with element prepended
oop s72_list_cons_method(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop element = va_arg(args, oop);
    va_end(args);
    
    printf("DEBUG: s72_list_cons_method called - receiver=%p, element=%p\n", receiver, element);
    
    S72Value list_val = {receiver};
    S72Value element_val = {element};
    S72Value new_list = s72_list_cons(element_val, list_val);
    
    return new_list.obj;
}

// length - get list length
oop s72_list_length_method(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_list_length_method called - receiver=%p\n", receiver);
    
    S72Value list_val = {receiver};
    int length = s72_list_length(list_val);
    
    S72Value length_val = s72_make_number(length);
    return length_val.obj;
}

// each: - iterate over list with block
oop s72_list_each_method(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop block = va_arg(args, oop);
    va_end(args);
    
    printf("DEBUG: s72_list_each_method called - receiver=%p, block=%p\n", receiver, block);
    
    if (!block || !s72_is_block((S72Value){block})) {
        s72_error("each: requires a block argument");
        return receiver;
    }
    
    S72Value list_val = {receiver};
    S72Value current = list_val;
    
    // Iterate through the list
    while (!s72_is_nil(current)) {
        if (!s72_is_list(current)) {
            s72_error("Improper list structure in each:");
            break;
        }
        
        S72Value element = s72_list_first(current);
        
        // Execute block with current element
        S72Value block_val = {block};
        S72Value args_array[1] = {element};
        s72_block_execute(block_val, 1, args_array);
        
        current = s72_list_rest(current);
    }
    
    return receiver;  // Return self
}
