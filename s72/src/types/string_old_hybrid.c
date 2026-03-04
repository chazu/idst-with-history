#include "string.h"
#include "number.h"
#include <stdio.h>
#include <string.h>

// Need access to _libid
extern struct __libid *_libid;

// Global vtable for strings
extern oop s72_string_vtable;

// String type initialization
void s72_string_init(void) {
    if (!s72_string_vtable) {
        s72_error("String vtable not initialized");
        return;
    }

    // Install native methods
    S72_METHOD(s72_string_vtable, SEL_PRINT, s72_string_print);
    S72_METHOD(s72_string_vtable, SEL_EQUALS, s72_string_equals);

    // Install length method
    oop sel_length = _libid->intern("length");
    S72_METHOD(s72_string_vtable, sel_length, s72_string_length_method);
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

    oop str_oop = S72_ALLOC(s72_string_vtable, sizeof(S72String));
    if (!str_oop) {
        s72_error("Failed to allocate string");
        return S72_NIL;
    }

    S72String *s72_str = (S72String *)str_oop;
    s72_str->length = length;
    s72_str->data = malloc(length + 1);

    if (!s72_str->data) {
        s72_error("Failed to allocate string data");
        return S72_NIL;
    }

    memcpy(s72_str->data, str, length);
    s72_str->data[length] = '\0';

    S72Value result = {str_oop};
    return result;
}

bool s72_is_string(S72Value val) {
    if (s72_is_nil(val)) return false;

    // Check if object has the string vtable
    if (!val.obj) return false;

    // Get the vtable from the libid object (libid stores vtable at position -1)
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];

    // Compare with string vtable - s72_string_vtable is a prototype, so get its actual vtable
    oop expected_vtable = s72_string_vtable->_vtable[-1];
    if (obj_vtable == expected_vtable) {
        return true;
    }

    // For M0 debugging only - don't use heuristics as they cause false positives
    return false;
}

const char *s72_string_data(S72Value val) {
    if (!s72_is_string(val)) {
        s72_error("Value is not a string");
        return "";
    }

    S72String *str = (S72String *)val.obj;
    return str->data;
}

size_t s72_string_length(S72Value val) {
    if (!s72_is_string(val)) {
        s72_error("Value is not a string");
        return 0;
    }

    S72String *str = (S72String *)val.obj;
    return str->length;
}

// String methods

oop s72_string_print(struct __send *send, oop self, oop receiver) {
    S72Value recv_val = {receiver};
    
    if (!s72_is_string(recv_val)) {
        s72_error("print called on non-string");
        return NULL;
    }
    
    const char *data = s72_string_data(recv_val);
    printf("%s", data);
    
    return receiver;  // Return self
}

oop s72_string_equals(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("= requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_string(recv_val)) {
        s72_error("= called on non-string");
        return NULL;
    }
    
    if (!s72_is_string(arg_val)) {
        return S72_FALSE.obj;  // Different types are not equal
    }
    
    const char *a = s72_string_data(recv_val);
    const char *b = s72_string_data(arg_val);
    
    bool equal = strcmp(a, b) == 0;
    return equal ? S72_TRUE.obj : S72_FALSE.obj;
}

oop s72_string_length_method(struct __send *send, oop self, oop receiver) {
    S72Value recv_val = {receiver};
    
    if (!s72_is_string(recv_val)) {
        s72_error("length called on non-string");
        return NULL;
    }
    
    size_t len = s72_string_length(recv_val);
    S72Value result = s72_number_new((double)len);
    return result.obj;
}
