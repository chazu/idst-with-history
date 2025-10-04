#ifndef OBJECT_H
#define OBJECT_H

#include "s72.h"

// S72 Object structure (built on libid)
typedef struct S72Object {
    // libid vtable pointer is at offset -1
    oop parent;        // Prototype chain
    oop method_dict;   // Method dictionary (selector -> method)
} S72Object;

// Method structure
typedef struct S72Method {
    oop selector;
    union {
        _imp_t native_func;    // C function
        oop block;             // Block object (for interpreted methods)
    } impl;
    bool is_native;
} S72Method;

// Object system initialization
void s72_object_init(void);

// Object creation
S72Value s72_object_new(void);
S72Value s72_object_new_with_parent(S72Value parent);

// Method installation
void s72_object_add_method(S72Value obj, oop selector, _imp_t method);
void s72_object_add_block_method(S72Value obj, oop selector, oop block);

// Method lookup
S72Method *s72_object_lookup_method(S72Value obj, oop selector);

// Message sending (core dispatch)
S72Value s72_object_send(S72Value receiver, oop selector, int argc, S72Value *argv);

// Prototype chain manipulation
void s72_object_set_parent(S72Value obj, S72Value parent);
S72Value s72_object_get_parent(S72Value obj);

// Object introspection
bool s72_object_responds_to(S72Value obj, oop selector);

#endif // OBJECT_H
