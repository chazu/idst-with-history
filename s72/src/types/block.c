#include "block.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External references
extern struct __libid *_libid;
extern oop s72_block_vtable;

// Global selectors for block operations
static oop SEL_BLOCK_VALUE = NULL;
static oop SEL_BLOCK_VALUE_WITH = NULL;
static oop SEL_BLOCK_VALUE_WITH_WITH = NULL;

// Block type initialization
void s72_block_init(void) {
    // Intern selectors
    SEL_BLOCK_VALUE = _libid->intern("value");
    SEL_BLOCK_VALUE_WITH = _libid->intern("value:");
    SEL_BLOCK_VALUE_WITH_WITH = _libid->intern("value:value:");

    // Install methods on block vtable
    S72_METHOD(s72_block_vtable, SEL_BLOCK_VALUE, s72_block_value);
    S72_METHOD(s72_block_vtable, SEL_BLOCK_VALUE_WITH, s72_block_value_with);
    S72_METHOD(s72_block_vtable, SEL_BLOCK_VALUE_WITH_WITH, s72_block_value_with_with);

    printf("DEBUG: Installed block methods on vtable %p\n", s72_block_vtable);
}

// Block creation
S72Value s72_block_new(ASTNode *body, S72Env *env, int param_count, char **param_names) {
    printf("DEBUG: s72_block_new - body=%p, env=%p, param_count=%d\n", body, env, param_count);
    
    // Allocate block object
    oop block_oop = S72_ALLOC(s72_block_vtable, sizeof(S72Block));
    if (!block_oop) {
        s72_error("Failed to allocate block");
        return S72_NIL;
    }
    
    S72Block *block = (S72Block *)block_oop;
    
    // Store the body AST (we'll need to copy it to avoid ownership issues)
    block->body = body;  // For now, assume caller manages lifetime
    
    // Capture the current environment
    block->captured_env = env;
    
    // Store parameter information
    block->param_count = param_count;
    if (param_count > 0 && param_names) {
        block->param_names = malloc(param_count * sizeof(char*));
        if (!block->param_names) {
            s72_error("Failed to allocate parameter names");
            return S72_NIL;
        }
        
        for (int i = 0; i < param_count; i++) {
            block->param_names[i] = strdup(param_names[i]);
            if (!block->param_names[i]) {
                s72_error("Failed to copy parameter name");
                return S72_NIL;
            }
        }
    } else {
        block->param_names = NULL;
    }
    
    printf("DEBUG: Created block at %p\n", block_oop);
    
    S72Value result = {block_oop};
    return result;
}

// Block type checking
bool s72_is_block(S72Value val) {
    if (s72_is_nil(val)) return false;

    // Check if object has the block vtable
    if (!val.obj) return false;

    // Get the vtable from the libid object (libid stores vtable at position -1)
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];

    // Compare with block vtable - s72_block_vtable is a prototype, so get its actual vtable
    oop expected_vtable = s72_block_vtable->_vtable[-1];

    // printf("DEBUG: s72_is_block - obj=%p, obj_vtable=%p, expected_vtable=%p\n",
    //        val.obj, obj_vtable, expected_vtable);

    if (obj_vtable == expected_vtable) {
        return true;
    }

    return false;
}

// Block execution with parameter binding
S72Value s72_block_execute(S72Value block_val, int argc, S72Value *argv) {
    if (!s72_is_block(block_val)) {
        s72_error("Cannot execute non-block value");
        return S72_NIL;
    }
    
    S72Block *block = (S72Block *)block_val.obj;
    
    printf("DEBUG: s72_block_execute - block=%p, argc=%d, param_count=%d\n", 
           block, argc, block->param_count);
    
    // Create new environment for block execution
    S72Env *block_env = s72_env_new(block->captured_env);
    
    // Bind parameters to arguments
    int bind_count = (argc < block->param_count) ? argc : block->param_count;
    for (int i = 0; i < bind_count; i++) {
        if (block->param_names && block->param_names[i]) {
            printf("DEBUG: Binding parameter '%s' to value %p\n", 
                   block->param_names[i], argv[i].obj);
            s72_env_bind(block_env, block->param_names[i], argv[i]);
        }
    }
    
    // Execute the block body
    S72Value result = S72_NIL;
    if (block->body) {
        result = s72_eval(block->body, block_env);
    }
    
    // Clean up block environment (but not the captured environment)
    s72_env_free(block_env);
    
    return result;
}

// Block method implementations

// value - execute block with no arguments
oop s72_block_value(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_block_value called - receiver=%p\n", receiver);
    
    S72Value block_val = {receiver};
    S72Value result = s72_block_execute(block_val, 0, NULL);
    
    return result.obj;
}

// value: - execute block with one argument
oop s72_block_value_with(oop closure, oop state, oop receiver, oop arg1) {
    printf("DEBUG: s72_block_value_with called - receiver=%p, arg=%p\n", receiver, arg1);
    
    S72Value block_val = {receiver};
    S72Value args[1] = {{arg1}};
    S72Value result = s72_block_execute(block_val, 1, args);
    
    return result.obj;
}

// value:value: - execute block with two arguments
oop s72_block_value_with_with(oop closure, oop state, oop receiver, oop arg1, oop arg2) {
    printf("DEBUG: s72_block_value_with_with called - receiver=%p, arg1=%p, arg2=%p\n", 
           receiver, arg1, arg2);
    
    S72Value block_val = {receiver};
    S72Value args[2] = {{arg1}, {arg2}};
    S72Value result = s72_block_execute(block_val, 2, args);
    
    return result.obj;
}
