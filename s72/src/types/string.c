#include "string.h"
#include "number.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern oop s72_string_vtable;
extern S72Value S72_TRUE, S72_FALSE, S72_NIL;

// ============================================================================
// Pure libid String Implementation
// ============================================================================

oop s72_string_new_libid(const char *str, size_t length) {
    if (!s72_string_vtable) {
        fprintf(stderr, "Error: String vtable not initialized\n");
        return 0;
    }
    
    // Allocate string object using libid
    oop string_obj = S72_ALLOC(s72_string_vtable, sizeof(struct t_String));
    if (!string_obj) {
        fprintf(stderr, "Error: Failed to allocate string object\n");
        return 0;
    }
    
    String string = (String)string_obj;
    
    // Store length as Number object
    string->length = s72_number_new((double)length).obj;
    
    // Allocate string data using libid (not malloc!)
    char *data = (char*)_libid->balloc(length + 1);
    if (!data) {
        fprintf(stderr, "Error: Failed to allocate string data\n");
        return 0;
    }
    
    // Copy string data
    if (str) {
        memcpy(data, str, length);
    }
    data[length] = '\0';
    
    // Store data pointer as raw oop (we could make this a proper object too)
    string->data = (oop)data;
    
    return string_obj;
}

// String creation and testing
S72Value s72_string_new(const char *str) {
    if (!str) {
        return S72_NIL;
    }
    
    return s72_string_new_with_length(str, strlen(str));
}

S72Value s72_string_new_with_length(const char *str, size_t length) {
    if (!str) {
        return S72_NIL;
    }
    
    oop string_obj = s72_string_new_libid(str, length);
    if (!string_obj) {
        return S72_NIL;
    }
    
    S72Value result = {string_obj};
    return result;
}

bool s72_is_string(S72Value val) {
    if (s72_is_nil(val)) return false;
    
    // Check if object has the string vtable
    if (!val.obj) return false;
    
    // Get the vtable from the libid object
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];
    
    // Compare with string vtable
    oop expected_vtable = s72_string_vtable->_vtable[-1];
    return (obj_vtable == expected_vtable);
}

const char *s72_string_data(S72Value val) {
    if (!s72_is_string(val)) {
        s72_error("Value is not a string");
        return "";
    }
    
    String string = (String)val.obj;
    return (const char*)string->data;
}

size_t s72_string_length(S72Value val) {
    if (!s72_is_string(val)) {
        s72_error("Value is not a string");
        return 0;
    }
    
    String string = (String)val.obj;
    S72Value length_val = {string->length};
    return (size_t)s72_number_value(length_val);
}

// ============================================================================
// String Methods (Pure libid implementations)
// ============================================================================

oop s72_string_print(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    String string = (String)self;
    const char *data = (const char*)string->data;
    
    printf("%s", data);
    return self;
}

oop s72_string_equals(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;
    
    va_list args;
    va_start(args, self);
    oop arg = va_arg(args, oop);
    va_end(args);
    
    if (!arg) {
        s72_error("= requires one argument");
        return S72_FALSE.obj;
    }
    
    S72Value self_val = {self};
    S72Value arg_val = {arg};
    
    if (!s72_is_string(self_val)) {
        s72_error("= called on non-string");
        return S72_FALSE.obj;
    }
    
    if (!s72_is_string(arg_val)) {
        return S72_FALSE.obj;  // Different types are not equal
    }
    
    const char *a = s72_string_data(self_val);
    const char *b = s72_string_data(arg_val);
    
    bool equal = strcmp(a, b) == 0;
    return equal ? S72_TRUE.obj : S72_FALSE.obj;
}

oop s72_string_length_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    String string = (String)self;
    return string->length;  // Return the Number object directly
}

// ============================================================================
// Initialization
// ============================================================================

void s72_string_init(void) {
    // Vtable should already be created in main.c
    if (!s72_string_vtable) {
        fprintf(stderr, "Error: String vtable not initialized\n");
        return;
    }
    
    // Install string methods
    oop sel_print = S72_INTERN("print");
    oop sel_equals = S72_INTERN("=");
    oop sel_length = S72_INTERN("length");
    
    S72_METHOD(s72_string_vtable, sel_print, s72_string_print);
    S72_METHOD(s72_string_vtable, sel_equals, s72_string_equals);
    S72_METHOD(s72_string_vtable, sel_length, s72_string_length_method);
    
    // Pure libid string system initialized successfully
}
