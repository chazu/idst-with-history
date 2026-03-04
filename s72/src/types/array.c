#include "array.h"
#include "number.h"
#include <stdio.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern oop s72_array_vtable;
extern S72Value S72_TRUE, S72_FALSE, S72_NIL;

// ============================================================================
// Pure libid Array Implementation
// ============================================================================

oop s72_array_new_libid(int size) {
    if (!s72_array_vtable) {
        fprintf(stderr, "Error: Array vtable not initialized\n");
        return 0;
    }
    
    if (size < 0) {
        fprintf(stderr, "Error: Array size cannot be negative\n");
        return 0;
    }
    
    // Allocate array object using libid
    oop array_obj = S72_ALLOC(s72_array_vtable, sizeof(struct t_Array));
    if (!array_obj) {
        fprintf(stderr, "Error: Failed to allocate array object\n");
        return 0;
    }
    
    Array array = (Array)array_obj;
    
    // Store size as Number object
    array->size = s72_number_new((double)size).obj;
    
    // Allocate elements array using libid (not malloc!)
    oop *elements = (oop*)_libid->balloc(size * sizeof(oop));
    if (!elements && size > 0) {
        fprintf(stderr, "Error: Failed to allocate array elements\n");
        return 0;
    }
    
    // Initialize all elements to nil
    for (int i = 0; i < size; i++) {
        elements[i] = S72_NIL.obj;
    }
    
    // Store elements pointer as raw oop
    array->elements = (oop)elements;
    
    return array_obj;
}

// Array creation and testing
S72Value s72_array_new(int size) {
    oop array_obj = s72_array_new_libid(size);
    if (!array_obj) {
        return S72_NIL;
    }
    
    S72Value result = {array_obj};
    return result;
}

S72Value s72_array_new_with_values(S72Value *values, int count) {
    S72Value array_val = s72_array_new(count);
    if (s72_is_nil(array_val)) {
        return S72_NIL;
    }
    
    for (int i = 0; i < count; i++) {
        s72_array_set(array_val, i, values[i]);
    }
    
    return array_val;
}

bool s72_is_array(S72Value val) {
    if (s72_is_nil(val)) return false;
    
    // Check if object has the array vtable
    if (!val.obj) return false;
    
    // Get the vtable from the libid object
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];
    
    // Compare with array vtable
    oop expected_vtable = s72_array_vtable->_vtable[-1];
    return (obj_vtable == expected_vtable);
}

int s72_array_size(S72Value val) {
    if (!s72_is_array(val)) {
        s72_error("Value is not an array");
        return 0;
    }
    
    Array array = (Array)val.obj;
    S72Value size_val = {array->size};
    return (int)s72_number_value(size_val);
}

S72Value s72_array_get(S72Value array_val, int index) {
    if (!s72_is_array(array_val)) {
        s72_error("Value is not an array");
        return S72_NIL;
    }
    
    Array array = (Array)array_val.obj;
    int size = s72_array_size(array_val);
    
    if (index < 0 || index >= size) {
        s72_error("Array index out of bounds");
        return S72_NIL;
    }
    
    oop *elements = (oop*)array->elements;
    S72Value result = {elements[index]};
    return result;
}

void s72_array_set(S72Value array_val, int index, S72Value value) {
    if (!s72_is_array(array_val)) {
        s72_error("Value is not an array");
        return;
    }
    
    Array array = (Array)array_val.obj;
    int size = s72_array_size(array_val);
    
    if (index < 0 || index >= size) {
        s72_error("Array index out of bounds");
        return;
    }
    
    oop *elements = (oop*)array->elements;
    elements[index] = value.obj;
}

// ============================================================================
// Array Methods (Pure libid implementations)
// ============================================================================

oop s72_array_at_(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;
    
    va_list args;
    va_start(args, self);
    oop index_obj = va_arg(args, oop);
    va_end(args);
    
    if (!index_obj) {
        s72_error("at: requires one argument");
        return S72_NIL.obj;
    }
    
    S72Value index_val = {index_obj};
    if (!s72_is_number(index_val)) {
        s72_error("Array index must be a number");
        return S72_NIL.obj;
    }
    
    int index = (int)s72_number_value(index_val) - 1; // Smalltalk uses 1-based indexing
    
    S72Value self_val = {self};
    S72Value result = s72_array_get(self_val, index);
    
    return result.obj;
}

oop s72_array_at_put_(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;
    
    va_list args;
    va_start(args, self);
    oop index_obj = va_arg(args, oop);
    oop value_obj = va_arg(args, oop);
    va_end(args);
    
    if (!index_obj || !value_obj) {
        s72_error("at:put: requires two arguments");
        return S72_NIL.obj;
    }
    
    S72Value index_val = {index_obj};
    if (!s72_is_number(index_val)) {
        s72_error("Array index must be a number");
        return S72_NIL.obj;
    }
    
    int index = (int)s72_number_value(index_val) - 1; // Smalltalk uses 1-based indexing
    
    S72Value self_val = {self};
    S72Value value_val = {value_obj};
    
    s72_array_set(self_val, index, value_val);
    
    return value_obj; // Return the value that was set
}

oop s72_array_size_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Array array = (Array)self;
    return array->size; // Return the Number object directly
}

oop s72_array_print(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    S72Value self_val = {self};
    int size = s72_array_size(self_val);
    
    printf("#(");
    for (int i = 0; i < size; i++) {
        if (i > 0) printf(" ");
        
        S72Value element = s72_array_get(self_val, i);
        
        // Simple printing - could be enhanced to call print method on elements
        if (s72_is_nil(element)) {
            printf("nil");
        } else if (s72_is_number(element)) {
            printf("%.0f", s72_number_value(element));
        } else {
            printf("<object>");
        }
    }
    printf(")");
    
    return self;
}

// ============================================================================
// Initialization
// ============================================================================

void s72_array_init(void) {
    // Vtable should already be created in main.c
    if (!s72_array_vtable) {
        fprintf(stderr, "Error: Array vtable not initialized\n");
        return;
    }
    
    // Install array methods
    oop sel_at = S72_INTERN("at:");
    oop sel_at_put = S72_INTERN("at:put:");
    oop sel_size = S72_INTERN("size");
    oop sel_print = S72_INTERN("print");
    
    S72_METHOD(s72_array_vtable, sel_at, s72_array_at_);
    S72_METHOD(s72_array_vtable, sel_at_put, s72_array_at_put_);
    S72_METHOD(s72_array_vtable, sel_size, s72_array_size_method);
    S72_METHOD(s72_array_vtable, sel_print, s72_array_print);
    
    // Pure libid array system initialized successfully
}
