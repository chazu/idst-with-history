#include "list.h"
#include "number.h"
#include "../object.h"
#include <stdio.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern oop s72_list_vtable;
extern oop s72_nil_vtable;
extern S72Value S72_TRUE, S72_FALSE, S72_NIL;

// ============================================================================
// Pure libid List Implementation
// ============================================================================

oop s72_list_new_libid(S72Value first, S72Value rest) {
    if (!s72_list_vtable) {
        fprintf(stderr, "Error: List vtable not initialized\n");
        return 0;
    }
    
    // Allocate list object using libid
    oop list_obj = S72_ALLOC(s72_list_vtable, sizeof(struct t_List));
    if (!list_obj) {
        fprintf(stderr, "Error: Failed to allocate list object\n");
        return 0;
    }
    
    List list = (List)list_obj;
    list->first = first.obj;
    list->rest = rest.obj;
    
    return list_obj;
}

// Create empty list (nil represents empty list)
S72Value s72_list_empty(void) {
    return S72_NIL;
}

// Create cons cell
S72Value s72_list_cons(S72Value first, S72Value rest) {
    oop list_obj = s72_list_new_libid(first, rest);
    if (!list_obj) {
        return S72_NIL;
    }
    
    S72Value result = {list_obj};
    return result;
}

// List type checking
bool s72_is_list(S72Value val) {
    // nil is considered an empty list
    if (s72_is_nil(val)) return true;
    
    if (!val.obj) return false;
    
    // Check if object has the list vtable
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];
    
    // Compare with list vtable
    oop expected_vtable = s72_list_vtable->_vtable[-1];
    return (obj_vtable == expected_vtable);
}

// List access functions
S72Value s72_list_first(S72Value list_val) {
    if (s72_is_nil(list_val)) {
        s72_error("Cannot get first of empty list");
        return S72_NIL;
    }
    
    if (!s72_is_list(list_val)) {
        s72_error("Value is not a list");
        return S72_NIL;
    }
    
    List list = (List)list_val.obj;
    S72Value result = {list->first};
    return result;
}

S72Value s72_list_rest(S72Value list_val) {
    if (s72_is_nil(list_val)) {
        return S72_NIL; // Rest of empty list is empty list
    }
    
    if (!s72_is_list(list_val)) {
        s72_error("Value is not a list");
        return S72_NIL;
    }
    
    List list = (List)list_val.obj;
    S72Value result = {list->rest};
    return result;
}

bool s72_list_is_empty(S72Value list_val) {
    return s72_is_nil(list_val);
}

int s72_list_length(S72Value list_val) {
    if (s72_is_nil(list_val)) {
        return 0;
    }
    
    if (!s72_is_list(list_val)) {
        s72_error("Value is not a list");
        return 0;
    }
    
    int count = 0;
    S72Value current = list_val;
    
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

// ============================================================================
// List Methods (Pure libid implementations)
// ============================================================================

oop s72_list_is_empty_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    S72Value self_val = {self};
    bool empty = s72_list_is_empty(self_val);
    
    return empty ? S72_TRUE.obj : S72_FALSE.obj;
}

oop s72_list_first_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    S72Value self_val = {self};
    S72Value first = s72_list_first(self_val);
    
    return first.obj;
}

oop s72_list_rest_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    S72Value self_val = {self};
    S72Value rest = s72_list_rest(self_val);
    
    return rest.obj;
}

oop s72_list_cons_method(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;
    
    va_list args;
    va_start(args, self);
    oop element = va_arg(args, oop);
    va_end(args);
    
    if (!element) {
        s72_error("cons: requires one argument");
        return S72_NIL.obj;
    }
    
    S72Value element_val = {element};
    S72Value self_val = {self};
    
    S72Value result = s72_list_cons(element_val, self_val);
    return result.obj;
}

oop s72_list_length_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    S72Value self_val = {self};
    int length = s72_list_length(self_val);
    
    S72Value length_val = s72_number_new((double)length);
    return length_val.obj;
}

oop s72_list_each_method(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;

    va_list args;
    va_start(args, self);
    oop block = va_arg(args, oop);
    va_end(args);

    if (!block) {
        s72_error("each: requires a block argument");
        return S72_NIL.obj;
    }

    // Iterate through the list and call the block for each element
    S72Value current = {self};

    while (!s72_list_is_empty(current)) {
        if (!s72_is_list(current)) {
            s72_error("Improper list structure in each:");
            break;
        }

        // Get the first element
        S72Value first = s72_list_first(current);

        // Call the block with the first element
        S72Value block_val = {block};
        S72Value args[1] = {first};
        s72_object_send(block_val, S72_INTERN("value:"), 1, args);

        // Move to the rest of the list
        current = s72_list_rest(current);
    }

    return self;
}

// ============================================================================
// Initialization
// ============================================================================

void s72_list_init(void) {
    // Vtables should already be created in main.c
    if (!s72_list_vtable || !s72_nil_vtable) {
        fprintf(stderr, "Error: List vtables not initialized\n");
        return;
    }
    
    // Install list methods on both list and nil vtables
    oop sel_is_empty = S72_INTERN("isEmpty");
    oop sel_first = S72_INTERN("first");
    oop sel_rest = S72_INTERN("rest");
    oop sel_cons = S72_INTERN("cons:");
    oop sel_length = S72_INTERN("length");
    oop sel_each = S72_INTERN("each:");
    
    // Install on list vtable
    S72_METHOD(s72_list_vtable, sel_is_empty, s72_list_is_empty_method);
    S72_METHOD(s72_list_vtable, sel_first, s72_list_first_method);
    S72_METHOD(s72_list_vtable, sel_rest, s72_list_rest_method);
    S72_METHOD(s72_list_vtable, sel_cons, s72_list_cons_method);
    S72_METHOD(s72_list_vtable, sel_length, s72_list_length_method);
    S72_METHOD(s72_list_vtable, sel_each, s72_list_each_method);
    
    // Install on nil vtable (empty list behavior)
    S72_METHOD(s72_nil_vtable, sel_is_empty, s72_list_is_empty_method);
    S72_METHOD(s72_nil_vtable, sel_first, s72_list_first_method);
    S72_METHOD(s72_nil_vtable, sel_rest, s72_list_rest_method);
    S72_METHOD(s72_nil_vtable, sel_cons, s72_list_cons_method);
    S72_METHOD(s72_nil_vtable, sel_length, s72_list_length_method);
    S72_METHOD(s72_nil_vtable, sel_each, s72_list_each_method);
    
    // Pure libid list system initialized successfully
}
