#include "array.h"
#include "block.h"
#include "number.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External references
extern struct __libid *_libid;

// Array vtable (extern declaration - defined in main.c)
extern oop s72_array_vtable;

// Global selectors for array operations
static oop SEL_ARRAY_SIZE = NULL;
static oop SEL_ARRAY_AT = NULL;
static oop SEL_ARRAY_AT_PUT = NULL;
static oop SEL_ARRAY_EACH = NULL;

// Array type initialization
void s72_array_init(void) {
    if (!s72_array_vtable) {
        s72_error("Array vtable not initialized");
        return;
    }

    // Intern selectors
    SEL_ARRAY_SIZE = _libid->intern("size");
    SEL_ARRAY_AT = _libid->intern("at:");
    SEL_ARRAY_AT_PUT = _libid->intern("at:put:");
    SEL_ARRAY_EACH = _libid->intern("each:");

    // Install methods on array vtable
    S72_METHOD(s72_array_vtable, SEL_ARRAY_SIZE, s72_array_size_method);
    S72_METHOD(s72_array_vtable, SEL_ARRAY_AT, s72_array_at_method);
    S72_METHOD(s72_array_vtable, SEL_ARRAY_AT_PUT, s72_array_at_put_method);
    S72_METHOD(s72_array_vtable, SEL_ARRAY_EACH, s72_array_each_method);

    printf("DEBUG: Installed array methods on vtable %p\n", s72_array_vtable);
}

// Array creation
S72Value s72_array_new(int size) {
    if (size < 0) {
        s72_error("Array size cannot be negative");
        return S72_NIL;
    }

    printf("DEBUG: s72_array_new - size=%d\n", size);
    
    // Allocate array object
    oop array_oop = S72_ALLOC(s72_array_vtable, sizeof(S72Array));
    if (!array_oop) {
        s72_error("Failed to allocate array");
        return S72_NIL;
    }
    
    S72Array *array = (S72Array *)array_oop;
    
    // Initialize array structure
    array->size = size;
    array->capacity = size;
    
    if (size > 0) {
        array->elements = malloc(size * sizeof(S72Value));
        if (!array->elements) {
            s72_error("Failed to allocate array elements");
            return S72_NIL;
        }
        
        // Initialize all elements to nil
        for (int i = 0; i < size; i++) {
            array->elements[i] = S72_NIL;
        }
    } else {
        array->elements = NULL;
    }
    
    printf("DEBUG: Created array at %p with size %d\n", array_oop, size);
    
    S72Value result = {array_oop};
    return result;
}

// Array creation with initial values
S72Value s72_array_new_with_values(int size, S72Value *values) {
    S72Value array = s72_array_new(size);
    if (s72_is_nil(array) || !values) {
        return array;
    }
    
    S72Array *arr = (S72Array *)array.obj;
    for (int i = 0; i < size; i++) {
        arr->elements[i] = values[i];
    }
    
    return array;
}

// Array type checking
bool s72_is_array(S72Value val) {
    if (s72_is_nil(val)) return false;

    // Check if object has the array vtable
    if (!val.obj) return false;

    // Get the vtable from the libid object (libid stores vtable at position -1)
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];

    // Compare with array vtable - s72_array_vtable is a prototype, so get its actual vtable
    oop expected_vtable = s72_array_vtable->_vtable[-1];

    if (obj_vtable == expected_vtable) {
        return true;
    }

    return false;
}

// Array access functions
int s72_array_size(S72Value array) {
    if (!s72_is_array(array)) {
        s72_error("Value is not an array");
        return 0;
    }
    
    S72Array *arr = (S72Array *)array.obj;
    return arr->size;
}

S72Value s72_array_at(S72Value array, int index) {
    if (!s72_is_array(array)) {
        s72_error("Value is not an array");
        return S72_NIL;
    }
    
    S72Array *arr = (S72Array *)array.obj;
    
    // Check bounds (1-based indexing like Smalltalk)
    if (index < 1 || index > arr->size) {
        s72_error("Array index out of bounds");
        return S72_NIL;
    }
    
    return arr->elements[index - 1]; // Convert to 0-based
}

void s72_array_at_put(S72Value array, int index, S72Value value) {
    if (!s72_is_array(array)) {
        s72_error("Value is not an array");
        return;
    }
    
    S72Array *arr = (S72Array *)array.obj;
    
    // Check bounds (1-based indexing like Smalltalk)
    if (index < 1 || index > arr->size) {
        s72_error("Array index out of bounds");
        return;
    }
    
    arr->elements[index - 1] = value; // Convert to 0-based
}

// Array iteration
void s72_array_each(S72Value array, S72Value block) {
    if (!s72_is_array(array)) {
        s72_error("Value is not an array");
        return;
    }
    
    if (!s72_is_block(block)) {
        s72_error("each: requires a block argument");
        return;
    }
    
    S72Array *arr = (S72Array *)array.obj;
    
    for (int i = 0; i < arr->size; i++) {
        S72Value args[1] = {arr->elements[i]};
        s72_block_execute(block, 1, args);
    }
}

// Array method implementations

// size - return the size of the array
oop s72_array_size_method(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_array_size_method called - receiver=%p\n", receiver);
    
    S72Value array_val = {receiver};
    int size = s72_array_size(array_val);
    
    S72Value size_val = s72_number_new((double)size);
    return size_val.obj;
}

// at: - get element at index
oop s72_array_at_method(oop closure, oop state, oop receiver, oop index) {
    printf("DEBUG: s72_array_at_method called - receiver=%p, index=%p\n", receiver, index);
    
    S72Value array_val = {receiver};
    S72Value index_val = {index};
    
    // Convert index to integer
    if (!s72_is_number(index_val)) {
        s72_error("Array index must be a number");
        return S72_NIL.obj;
    }
    
    int idx = (int)s72_number_value(index_val);
    S72Value result = s72_array_at(array_val, idx);
    
    return result.obj;
}

// at:put: - set element at index
oop s72_array_at_put_method(oop closure, oop state, oop receiver, oop index, oop value) {
    printf("DEBUG: s72_array_at_put_method called - receiver=%p, index=%p, value=%p\n", 
           receiver, index, value);
    
    S72Value array_val = {receiver};
    S72Value index_val = {index};
    S72Value value_val = {value};
    
    // Convert index to integer
    if (!s72_is_number(index_val)) {
        s72_error("Array index must be a number");
        return receiver; // Return self
    }
    
    int idx = (int)s72_number_value(index_val);
    s72_array_at_put(array_val, idx, value_val);
    
    return receiver; // Return self
}

// each: - iterate over array elements with a block
oop s72_array_each_method(oop closure, oop state, oop receiver, oop block) {
    printf("DEBUG: s72_array_each_method called - receiver=%p, block=%p\n", receiver, block);
    
    S72Value array_val = {receiver};
    S72Value block_val = {block};
    
    s72_array_each(array_val, block_val);
    
    return receiver; // Return self
}
