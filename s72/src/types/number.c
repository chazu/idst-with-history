#include "number.h"
#include <stdio.h>
#include <math.h>

// Need access to _libid
extern struct __libid *_libid;

// Global vtable for numbers
extern oop s72_number_vtable;

// Number type initialization
void s72_number_init(void) {
    if (!s72_number_vtable) {
        s72_error("Number vtable not initialized");
        return;
    }

    // Install native methods on the vtable
    S72_METHOD(s72_number_vtable, SEL_PLUS, s72_number_add);
    S72_METHOD(s72_number_vtable, SEL_MINUS, s72_number_subtract);
    S72_METHOD(s72_number_vtable, SEL_MULTIPLY, s72_number_multiply);
    S72_METHOD(s72_number_vtable, SEL_DIVIDE, s72_number_divide);
    S72_METHOD(s72_number_vtable, SEL_EQUALS, s72_number_equals);
    S72_METHOD(s72_number_vtable, SEL_PRINT, s72_number_print);

    printf("DEBUG: Installed methods on number vtable %p\n", s72_number_vtable);
}

// Number creation and testing
S72Value s72_number_new(double value) {
    printf("DEBUG: s72_number_new - s72_number_vtable=%p\n", s72_number_vtable);

    // Allocate using the number vtable
    oop num_oop = S72_ALLOC(s72_number_vtable, sizeof(S72Number));
    if (!num_oop) {
        s72_error("Failed to allocate number");
        return S72_NIL;
    }

    printf("DEBUG: Allocated number object at %p\n", num_oop);

    // Check the vtable that was set (libid stores vtable at position -1)
    oop *vtable_ptr = (oop *)num_oop;
    oop obj_vtable = vtable_ptr[-1];
    oop expected_vtable = s72_number_vtable->_vtable[-1];
    printf("DEBUG: Object's vtable is %p (expected %p)\n", obj_vtable, expected_vtable);

    S72Number *num = (S72Number *)num_oop;
    num->value = value;

    printf("DEBUG: Set number value to %f\n", value);

    S72Value result = {num_oop};
    return result;
}

bool s72_is_number(S72Value val) {
    if (s72_is_nil(val)) return false;

    // Check if object has the number vtable
    if (!val.obj) return false;

    // Get the vtable from the libid object (libid stores vtable at position -1)
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];

    // Compare with number vtable - s72_number_vtable is a prototype, so get its actual vtable
    oop expected_vtable = s72_number_vtable->_vtable[-1];
    if (obj_vtable == expected_vtable) {
        return true;
    }

    // For M0 debugging only - don't use heuristics as they cause false positives
    return false;
}

double s72_number_value(S72Value val) {
    if (!s72_is_number(val)) {
        s72_error("Value is not a number");
        return 0.0;
    }

    S72Number *num = (S72Number *)val.obj;
    return num->value;
}

// Number arithmetic methods

oop s72_number_add(struct __send *send, oop self, oop receiver, oop arg) {
    printf("DEBUG: s72_number_add called - receiver=%p, arg=%p\n", receiver, arg);

    if (!arg) {
        printf("ERROR: + requires one argument\n");
        return NULL;
    }

    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};

    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        printf("ERROR: + requires numeric arguments\n");
        return NULL;
    }

    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);

    printf("DEBUG: Adding %f + %f\n", a, b);

    S72Value result = s72_number_new(a + b);
    printf("DEBUG: Result = %p\n", result.obj);
    return result.obj;
}

oop s72_number_subtract(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("- requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        s72_error("- requires numeric arguments");
        return NULL;
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    S72Value result = s72_number_new(a - b);
    return result.obj;
}

oop s72_number_multiply(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("* requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        s72_error("* requires numeric arguments");
        return NULL;
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    S72Value result = s72_number_new(a * b);
    return result.obj;
}

oop s72_number_divide(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("/ requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        s72_error("/ requires numeric arguments");
        return NULL;
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    if (b == 0.0) {
        s72_error("Division by zero");
        return NULL;
    }
    
    S72Value result = s72_number_new(a / b);
    return result.obj;
}

oop s72_number_equals(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("= requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        return S72_FALSE.obj;  // Different types are not equal
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    // Use epsilon comparison for floating point
    bool equal = fabs(a - b) < 1e-15;
    return equal ? S72_TRUE.obj : S72_FALSE.obj;
}

// Number comparison methods
oop s72_number_less_than(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("< requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        s72_error("< requires numeric arguments");
        return NULL;
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    return (a < b) ? S72_TRUE.obj : S72_FALSE.obj;
}

oop s72_number_greater_than(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("> requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        s72_error("> requires numeric arguments");
        return NULL;
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    return (a > b) ? S72_TRUE.obj : S72_FALSE.obj;
}

oop s72_number_less_equal(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("<= requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        s72_error("<= requires numeric arguments");
        return NULL;
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    return (a <= b) ? S72_TRUE.obj : S72_FALSE.obj;
}

oop s72_number_greater_equal(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error(">= requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_number(recv_val) || !s72_is_number(arg_val)) {
        s72_error(">= requires numeric arguments");
        return NULL;
    }
    
    double a = s72_number_value(recv_val);
    double b = s72_number_value(arg_val);
    
    return (a >= b) ? S72_TRUE.obj : S72_FALSE.obj;
}

// Number printing
oop s72_number_print(struct __send *send, oop self, oop receiver) {
    S72Value recv_val = {receiver};
    
    if (!s72_is_number(recv_val)) {
        s72_error("print called on non-number");
        return NULL;
    }
    
    double value = s72_number_value(recv_val);
    
    // Print the number
    if (value == floor(value) && value >= -1e15 && value <= 1e15) {
        // Print as integer if it's a whole number
        printf("%.0f", value);
    } else {
        // Print as floating point
        printf("%.15g", value);
    }
    
    return receiver;  // Return self
}
