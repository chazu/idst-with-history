#include "boolean.h"
#include "block.h"
#include <stdio.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern oop s72_boolean_vtable;
extern S72Value S72_TRUE;
extern S72Value S72_FALSE;
extern S72Value S72_NIL;

// Global selectors for boolean operations
static oop SEL_IF_TRUE = NULL;
static oop SEL_IF_FALSE = NULL;
static oop SEL_IF_TRUE_IF_FALSE = NULL;
static oop SEL_AND = NULL;
static oop SEL_OR = NULL;
static oop SEL_NOT = NULL;

// Boolean type initialization
void s72_boolean_init(void) {
    // Intern selectors
    SEL_IF_TRUE = _libid->intern("ifTrue:");
    SEL_IF_FALSE = _libid->intern("ifFalse:");
    SEL_IF_TRUE_IF_FALSE = _libid->intern("ifTrue:ifFalse:");
    SEL_AND = _libid->intern("and:");
    SEL_OR = _libid->intern("or:");
    SEL_NOT = _libid->intern("not");

    // Install methods on boolean vtable
    S72_METHOD(s72_boolean_vtable, SEL_IF_TRUE, s72_boolean_if_true);
    S72_METHOD(s72_boolean_vtable, SEL_IF_FALSE, s72_boolean_if_false);
    S72_METHOD(s72_boolean_vtable, SEL_IF_TRUE_IF_FALSE, s72_boolean_if_true_if_false);
    S72_METHOD(s72_boolean_vtable, SEL_AND, s72_boolean_and);
    S72_METHOD(s72_boolean_vtable, SEL_OR, s72_boolean_or);
    S72_METHOD(s72_boolean_vtable, SEL_NOT, s72_boolean_not);

    printf("DEBUG: Installed boolean methods on vtable %p\n", s72_boolean_vtable);
}

// Boolean type checking
bool s72_is_boolean(S72Value val) {
    return val.obj == S72_TRUE.obj || val.obj == S72_FALSE.obj;
}

// Boolean singleton accessors
S72Value s72_true(void) {
    return S72_TRUE;
}

S72Value s72_false(void) {
    return S72_FALSE;
}

S72Value s72_nil(void) {
    return S72_NIL;
}

// Boolean method implementations

// ifTrue: - execute block if receiver is true
oop s72_boolean_if_true(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop true_block = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_boolean_if_true called - receiver=%p, block=%p\n", receiver, true_block);

    // Check if receiver is true
    if (receiver == S72_TRUE.obj) {
        // Execute the block directly
        if (true_block && s72_is_block((S72Value){true_block})) {
            printf("DEBUG: Executing true block\n");
            S72Value block_val = {true_block};
            S72Value result = s72_block_execute(block_val, 0, NULL);
            printf("DEBUG: Block execution returned %p\n", result.obj);
            return result.obj;
        }
    }

    // Return nil if false or invalid block
    printf("DEBUG: Returning nil\n");
    return NULL;  // nil
}

// ifFalse: - execute block if receiver is false
oop s72_boolean_if_false(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop false_block = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_boolean_if_false called - receiver=%p, block=%p\n", receiver, false_block);

    // Check if receiver is false
    if (receiver == S72_FALSE.obj) {
        // Execute the block directly
        if (false_block && s72_is_block((S72Value){false_block})) {
            printf("DEBUG: Executing false block\n");
            S72Value block_val = {false_block};
            S72Value result = s72_block_execute(block_val, 0, NULL);
            printf("DEBUG: Block execution returned %p\n", result.obj);
            return result.obj;
        }
    }

    // Return nil if true or invalid block
    printf("DEBUG: Returning nil\n");
    return NULL;  // nil
}

// ifTrue:ifFalse: - execute appropriate block based on receiver
oop s72_boolean_if_true_if_false(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop true_block = va_arg(args, oop);
    oop false_block = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_boolean_if_true_if_false called - receiver=%p, true_block=%p, false_block=%p\n",
           receiver, true_block, false_block);

    oop block_to_execute = NULL;

    if (receiver == S72_TRUE.obj) {
        block_to_execute = true_block;
    } else if (receiver == S72_FALSE.obj) {
        block_to_execute = false_block;
    }

    if (block_to_execute && s72_is_block((S72Value){block_to_execute})) {
        printf("DEBUG: Executing chosen block\n");
        S72Value block_val = {block_to_execute};
        S72Value result = s72_block_execute(block_val, 0, NULL);
        printf("DEBUG: Block execution returned %p\n", result.obj);
        return result.obj;
    }

    printf("DEBUG: Returning nil\n");
    return NULL;  // nil
}

// and: - logical AND with another boolean
oop s72_boolean_and(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop other = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_boolean_and called - receiver=%p, other=%p\n", receiver, other);

    // Short-circuit: if receiver is false, return false
    if (receiver == S72_FALSE.obj) {
        return S72_FALSE.obj;
    }

    // If receiver is true, return the other value (should be boolean)
    if (receiver == S72_TRUE.obj) {
        if (other == S72_TRUE.obj || other == S72_FALSE.obj) {
            return other;
        }
    }

    // Invalid case - return false
    return S72_FALSE.obj;
}

// or: - logical OR with another boolean
oop s72_boolean_or(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop other = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_boolean_or called - receiver=%p, other=%p\n", receiver, other);

    // Short-circuit: if receiver is true, return true
    if (receiver == S72_TRUE.obj) {
        return S72_TRUE.obj;
    }

    // If receiver is false, return the other value (should be boolean)
    if (receiver == S72_FALSE.obj) {
        if (other == S72_TRUE.obj || other == S72_FALSE.obj) {
            return other;
        }
    }

    // Invalid case - return false
    return S72_FALSE.obj;
}

// not - logical NOT
oop s72_boolean_not(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_boolean_not called - receiver=%p\n", receiver);
    
    if (receiver == S72_TRUE.obj) {
        return S72_FALSE.obj;
    } else if (receiver == S72_FALSE.obj) {
        return S72_TRUE.obj;
    }
    
    // Invalid case - return false
    return S72_FALSE.obj;
}
