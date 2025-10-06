#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Need access to _libid
extern struct __libid *_libid;

// Global vtables (defined in main.c)
extern oop s72_object_vtable;

// Object system initialization
void s72_object_init(void) {
    // Create the base Object vtable if not already created
    if (!s72_object_vtable) {
        // This will be properly initialized in main.c with libid
        s72_error("Object system not properly initialized");
    }

    // Install base object methods
    S72_METHOD(s72_object_vtable, SEL_DOES_NOT_UNDERSTAND, s72_object_does_not_understand);

    // Add a test method to demonstrate inheritance
    oop sel_class = _libid->intern("class");
    S72_METHOD(s72_object_vtable, sel_class, s72_object_class);
}

// Object creation
S72Value s72_object_new(void) {
    // Allocate object using libid
    oop obj_oop = S72_ALLOC(s72_object_vtable, sizeof(S72Object));
    if (!obj_oop) {
        s72_error("Failed to allocate object");
        return S72_NIL;
    }
    
    S72Object *obj = (S72Object *)obj_oop;
    obj->parent = NULL;
    obj->method_dict = NULL;  // Will create on first method addition
    
    S72Value result = {obj_oop};
    return result;
}

S72Value s72_object_new_with_parent(S72Value parent) {
    S72Value obj = s72_object_new();
    if (!s72_is_nil(obj)) {
        s72_object_set_parent(obj, parent);
    }
    return obj;
}

// Method installation
void s72_object_add_method(S72Value obj, oop selector, _imp_t method) {
    if (s72_is_nil(obj)) {
        s72_error("Cannot add method to nil object");
        return;
    }
    
    // For M0, we'll use libid's method installation directly
    // This is a simplified approach - full implementation will use method dictionaries
    _libid->method(obj.obj, selector, method);
}

void s72_object_add_block_method(S72Value obj, oop selector, oop block) {
    // TODO: Implement block methods in later milestones
    s72_error("Block methods not yet implemented");
}

// Method lookup
S72Method *s72_object_lookup_method(S72Value obj, oop selector) {
    // For M0, we rely on libid's built-in lookup
    // This is simplified - full implementation will traverse prototype chain
    
    if (s72_is_nil(obj)) {
        return NULL;
    }
    
    // Use libid's bind to check if method exists
    struct __closure *closure = _libid->bind(selector, obj.obj);
    if (closure && closure->method) {
        // Create a temporary method structure
        static S72Method temp_method;
        temp_method.selector = selector;
        temp_method.impl.native_func = closure->method;
        temp_method.is_native = true;
        return &temp_method;
    }
    
    return NULL;
}

// Message sending (core dispatch)
S72Value s72_object_send(S72Value receiver, oop selector, int argc, S72Value *argv) {
    // Allow messages to nil (it has methods now)
    if (!receiver.obj) {
        s72_error("Cannot send message to NULL receiver");
        return S72_NIL;
    }

    printf("DEBUG: About to dispatch selector=%p to receiver=%p, argc=%d\n",
           selector, receiver.obj, argc);

    // Use enhanced libid dispatch macros directly
    oop result_oop = NULL;

    switch (argc) {
        case 0:
            printf("DEBUG: Using _send0\n");
            result_oop = _send0(selector, receiver.obj);
            break;
        case 1:
            printf("DEBUG: Using _send1 with arg=%p\n", argv[0].obj);
            result_oop = _send1(selector, receiver.obj, argv[0].obj);
            break;
        case 2:
            printf("DEBUG: Using _send2 with args=%p, %p\n", argv[0].obj, argv[1].obj);
            result_oop = _send2(selector, receiver.obj, argv[0].obj, argv[1].obj);
            break;
        case 3:
            printf("DEBUG: Using _send3 with args=%p, %p, %p\n", argv[0].obj, argv[1].obj, argv[2].obj);
            result_oop = _send3(selector, receiver.obj, argv[0].obj, argv[1].obj, argv[2].obj);
            break;
        default:
            s72_error("Too many arguments for message send (max 3 for M0)");
            return S72_NIL;
    }

    printf("DEBUG: Method returned %p\n", result_oop);

    S72Value result = {result_oop};
    return result;
}

// Prototype chain manipulation
void s72_object_set_parent(S72Value obj, S72Value parent) {
    if (s72_is_nil(obj)) {
        s72_error("Cannot set parent of nil object");
        return;
    }
    
    S72Object *s72_obj = (S72Object *)obj.obj;
    s72_obj->parent = parent.obj;
}

S72Value s72_object_get_parent(S72Value obj) {
    if (s72_is_nil(obj)) {
        return S72_NIL;
    }
    
    S72Object *s72_obj = (S72Object *)obj.obj;
    S72Value result = {s72_obj->parent};
    return result;
}

// Object introspection
bool s72_object_responds_to(S72Value obj, oop selector) {
    return s72_object_lookup_method(obj, selector) != NULL;
}

// Default doesNotUnderstand: handler
oop s72_object_does_not_understand(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop selector = va_arg(args, oop);
    va_end(args);

    // For now, just print an error message and return nil
    // TODO: In M2, this should create a proper error message with the selector name
    printf("ERROR: Object does not understand message\n");
    s72_error("Message not understood");
    return NULL;
}

// Basic class method - returns a string representation of the object's type
oop s72_object_class(oop closure, oop state, oop receiver) {
    // For now, just return a simple string
    // TODO: In M2, this should return a proper Class object
    printf("Object");
    return receiver;  // Return self for now
}
