#include "object.h"
#include <stdio.h>
#include <stdlib.h>

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
    if (s72_is_nil(receiver)) {
        s72_error("Cannot send message to nil");
        return S72_NIL;
    }
    
    // For M0, use libid's dispatch directly
    // Convert S72Value array to oop array
    oop *libid_args = NULL;
    if (argc > 0) {
        libid_args = malloc(argc * sizeof(oop));
        for (int i = 0; i < argc; i++) {
            libid_args[i] = argv[i].obj;
        }
    }
    
    // Use libid's public API correctly
    oop result_oop = NULL;

    printf("DEBUG: About to dispatch selector=%p to receiver=%p, argc=%d\n",
           selector, receiver.obj, argc);

    // Use libid's bind function to find the method
    printf("DEBUG: Calling _libid->bind(%p, %p)\n", selector, receiver.obj);
    struct __closure *closure = _libid->bind(selector, receiver.obj);
    printf("DEBUG: _libid->bind returned %p\n", closure);

    if (!closure) {
        printf("DEBUG: No closure found for selector\n");
        s72_error("Method not found");
        if (libid_args) free(libid_args);
        return S72_NIL;
    }

    printf("DEBUG: closure->method = %p\n", closure->method);
    if (!closure->method) {
        printf("DEBUG: Closure has no method\n");
        s72_error("Method not found");
        if (libid_args) free(libid_args);
        return S72_NIL;
    }

    printf("DEBUG: Found method %p\n", closure->method);

    // Create a proper send structure for the method call
    struct __send send_struct = {
        .selector = selector,
        .nArgs = argc + 1,  // +1 for receiver
        .receiver = receiver.obj,
        .state = NULL,
        .closure = closure
    };

    // Call the method with the proper signature
    switch (argc) {
        case 0:
            result_oop = closure->method(&send_struct, receiver.obj, receiver.obj);
            break;
        case 1:
            result_oop = closure->method(&send_struct, receiver.obj, receiver.obj, libid_args[0]);
            break;
        case 2:
            result_oop = closure->method(&send_struct, receiver.obj, receiver.obj, libid_args[0], libid_args[1]);
            break;
        case 3:
            result_oop = closure->method(&send_struct, receiver.obj, receiver.obj, libid_args[0], libid_args[1], libid_args[2]);
            break;
        default:
            s72_error("Too many arguments for message send (max 3 for M1)");
            if (libid_args) free(libid_args);
            return S72_NIL;
    }

    printf("DEBUG: Method returned %p\n", result_oop);
    
    if (libid_args) free(libid_args);
    
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
