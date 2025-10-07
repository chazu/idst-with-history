#include "object.h"
#include "env.h"
#include "types/block.h"
#include "types/string.h"
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
    S72_METHOD(s72_object_vtable, SEL_BECOME, s72_object_become);

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

// Block method wrapper structure
typedef struct {
    oop block;  // The block to execute
} S72BlockMethodWrapper;

// Block method wrapper function - calls the block with arguments
static oop s72_block_method_wrapper(oop closure, oop state, oop receiver, ...) {
    S72BlockMethodWrapper *wrapper = (S72BlockMethodWrapper *)closure;
    if (!wrapper || !wrapper->block) {
        s72_error("Invalid block method wrapper");
        return NULL;
    }

    // Get the block and execute it
    S72Value block_val = {wrapper->block};

    // For now, we'll support up to 3 arguments (common case)
    va_list args;
    va_start(args, receiver);

    // Count arguments by checking if they're non-null
    // This is a simplified approach - in a full implementation we'd need better argument handling
    S72Value argv[4];  // receiver + up to 3 args
    int argc = 1;

    argv[0].obj = receiver;  // First argument is always the receiver

    oop arg1 = va_arg(args, oop);
    if (arg1) {
        argv[argc++].obj = arg1;

        oop arg2 = va_arg(args, oop);
        if (arg2) {
            argv[argc++].obj = arg2;

            oop arg3 = va_arg(args, oop);
            if (arg3) {
                argv[argc++].obj = arg3;
            }
        }
    }

    va_end(args);

    // Execute the block with the arguments
    S72Value result = s72_block_execute(block_val, argc, argv);
    return result.obj;
}

void s72_object_add_block_method(S72Value obj, oop selector, oop block) {
    if (s72_is_nil(obj)) {
        s72_error("Cannot add method to nil object");
        return;
    }

    if (!block) {
        s72_error("Cannot add null block as method");
        return;
    }

    // Create a wrapper structure to hold the block
    S72BlockMethodWrapper *wrapper = malloc(sizeof(S72BlockMethodWrapper));
    if (!wrapper) {
        s72_error("Failed to allocate block method wrapper");
        return;
    }

    wrapper->block = block;

    // Install the wrapper function as a method, passing the wrapper as closure
    _libid->method(obj.obj, selector, (_imp_t)s72_block_method_wrapper);

    printf("DEBUG: Installed block method for selector %p on object %p\n", selector, obj.obj);
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

// Helper function to get selector name from oop
static const char* s72_selector_name(oop selector) {
    if (!selector) return "<unknown>";

    // In libid, selectors are interned strings
    // We can try to extract the string representation
    // This is a simplified approach - in a full implementation we'd have proper selector introspection

    // For now, we'll use a simple heuristic based on common selectors
    // In a real implementation, we'd maintain a reverse mapping of selectors to names
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "selector@%p", selector);
    return buffer;
}

// Create an error object with message and selector information
static oop s72_create_error_object(const char* message, oop selector, oop receiver) {
    // For now, create a simple string object with error information
    // In a full implementation, this would be a proper Error class instance

    char error_msg[512];
    const char* selector_name = s72_selector_name(selector);

    snprintf(error_msg, sizeof(error_msg),
             "MessageNotUnderstood: Object %p does not understand selector '%s': %s",
             receiver, selector_name, message);

    // Create a string object to hold the error message
    S72Value error_string = s72_string_new(error_msg);
    return error_string.obj;
}

// Default doesNotUnderstand: handler
oop s72_object_does_not_understand(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop selector = va_arg(args, oop);
    va_end(args);

    // Create a proper error object with selector information
    oop error_obj = s72_create_error_object("Message not understood", selector, receiver);

    // Print error message for debugging
    const char* selector_name = s72_selector_name(selector);
    printf("ERROR: Object %p does not understand selector '%s'\n", receiver, selector_name);

    // In a full implementation, this would throw/signal the error
    // For now, we'll return the error object
    return error_obj;
}

// Helper function to determine object type name
static const char* s72_object_type_name(oop obj) {
    if (!obj) return "Nil";

    // Get the vtable from the object
    oop *vtable_ptr = (oop *)obj;
    oop obj_vtable = vtable_ptr[-1];

    // Compare with known vtables to determine type
    extern oop s72_nil_vtable, s72_boolean_vtable, s72_number_vtable;
    extern oop s72_string_vtable, s72_symbol_vtable, s72_list_vtable;
    extern oop s72_array_vtable, s72_block_vtable, s72_transcript_vtable;
    extern oop s72_turtle_vtable;

    if (obj_vtable == s72_nil_vtable->_vtable[-1]) return "Nil";
    if (obj_vtable == s72_boolean_vtable->_vtable[-1]) return "Boolean";
    if (obj_vtable == s72_number_vtable->_vtable[-1]) return "Number";
    if (obj_vtable == s72_string_vtable->_vtable[-1]) return "String";
    if (obj_vtable == s72_symbol_vtable->_vtable[-1]) return "Symbol";
    if (obj_vtable == s72_list_vtable->_vtable[-1]) return "List";
    if (obj_vtable == s72_array_vtable->_vtable[-1]) return "Array";
    if (obj_vtable == s72_block_vtable->_vtable[-1]) return "Block";
    if (obj_vtable == s72_transcript_vtable->_vtable[-1]) return "Transcript";
    if (obj_vtable == s72_turtle_vtable->_vtable[-1]) return "Turtle";

    return "Object";  // Default fallback
}

// Basic class method - returns a string representation of the object's type
oop s72_object_class(oop closure, oop state, oop receiver) {
    const char* type_name = s72_object_type_name(receiver);

    // Create a string object with the type name
    S72Value class_name = s72_string_new(type_name);

    printf("DEBUG: Object %p has class '%s'\n", receiver, type_name);

    return class_name.obj;
}

// Smalltalk-72 authentic 'become:' method: (x become: newValue)
oop s72_object_become(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop new_value = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_object_become called - receiver=%p, new_value=%p\n", receiver, new_value);

    // In authentic Smalltalk-72, 'become:' was used to change object identity
    // This is a complex operation that requires updating all references
    // For our simplified implementation, we'll simulate it by updating variable bindings

    // This is a simplified implementation - in a full system, we'd need to:
    // 1. Find all references to the receiver object
    // 2. Replace them with references to new_value
    // 3. Handle the complex memory management implications

    // For now, we'll implement a limited version that works with global variables
    S72Env *global_env = s72_env_get_global();
    if (!global_env) {
        s72_error("Global environment not initialized for become:");
        return new_value;
    }

    // Search through global bindings and replace any that point to receiver
    bool found_replacement = false;
    for (int i = 0; i < global_env->binding_count; i++) {
        if (global_env->bindings[i].value.obj == receiver) {
            S72Value new_val = {new_value};
            global_env->bindings[i].value = new_val;
            printf("DEBUG: 'become:' updated global binding '%s' from %p to %p\n",
                   global_env->bindings[i].name, receiver, new_value);
            found_replacement = true;
        }
    }

    if (!found_replacement) {
        printf("DEBUG: 'become:' - no global bindings found for object %p\n", receiver);
    }

    // In Smalltalk-72, become: returns the new object
    return new_value;
}
