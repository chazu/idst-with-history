#include "src/s72.h"
#include <stdio.h>
#include <stdarg.h>

// Global libid instance
struct __libid *_libid;

// Global selectors
oop SEL_PLUS;

// Global vtables
oop s72_number_vtable;

// Test method - using variadic arguments like _imp_t
oop test_add(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop arg = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: test_add called - closure=%p, state=%p, receiver=%p, arg=%p\n",
           closure, state, receiver, arg);
    printf("DEBUG: Returning receiver=%p\n", receiver);
    return receiver; // Return the receiver
}

// Test method with no arguments
oop test_print(oop closure, oop state, oop receiver) {
    printf("DEBUG: test_print called - closure=%p, state=%p, receiver=%p\n",
           closure, state, receiver);
    printf("DEBUG: Returning receiver=%p\n", receiver);
    return receiver; // Return the receiver
}

int main(int argc, char **argv, char **envp) {
    // Initialize libid
    _libid = _libid_init(&argc, &argv, &envp);
    if (!_libid) {
        printf("Failed to initialize libid\n");
        return 1;
    }
    
    printf("DEBUG: libid initialized\n");
    
    // Create a vtable
    s72_number_vtable = _libid->proto(NULL);
    if (!s72_number_vtable) {
        printf("Failed to create vtable\n");
        return 1;
    }
    
    printf("DEBUG: Created vtable %p\n", s72_number_vtable);
    
    // Intern selector
    SEL_PLUS = _libid->intern("+");
    if (!SEL_PLUS) {
        printf("Failed to intern selector\n");
        return 1;
    }
    
    printf("DEBUG: Interned selector + = %p\n", SEL_PLUS);
    
    // Install methods
    _libid->method(s72_number_vtable, SEL_PLUS, (_imp_t)test_add);

    // Create a print selector and install print method
    oop SEL_PRINT = _libid->intern("print");
    _libid->method(s72_number_vtable, SEL_PRINT, (_imp_t)test_print);
    printf("DEBUG: Installed methods\n");
    
    // Create an object
    oop obj = _libid->alloc(s72_number_vtable, sizeof(double));
    if (!obj) {
        printf("Failed to allocate object\n");
        return 1;
    }
    
    printf("DEBUG: Created object %p\n", obj);
    
    // Create another object for the argument
    oop arg = _libid->alloc(s72_number_vtable, sizeof(double));
    if (!arg) {
        printf("Failed to allocate argument object\n");
        return 1;
    }
    
    printf("DEBUG: Created argument object %p\n", arg);
    
    // Test dispatch using _send0 (no arguments)
    printf("DEBUG: About to call _send0 (print)\n");
    oop result0 = _send0(SEL_PRINT, obj);
    printf("DEBUG: _send0 returned %p\n", result0);

    // Test dispatch using _send1
    printf("DEBUG: About to call _send1 with obj=%p, arg=%p\n", obj, arg);
    oop result = _send1(SEL_PLUS, obj, arg);
    printf("DEBUG: _send1 returned %p\n", result);
    
    return 0;
}
