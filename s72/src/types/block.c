#include "block.h"
#include "number.h"
#include "array.h"
#include "string.h"
#include <stdio.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern oop s72_block_vtable;
extern S72Value S72_TRUE, S72_FALSE, S72_NIL;

// ============================================================================
// Pure libid Block Implementation
// ============================================================================

oop s72_block_new_libid(ASTNode *body, S72Env *env, int param_count, char **param_names) {
    if (!s72_block_vtable) {
        fprintf(stderr, "Error: Block vtable not initialized\n");
        return 0;
    }
    
    // Allocate block object using libid
    oop block_obj = S72_ALLOC(s72_block_vtable, sizeof(struct t_Block));
    if (!block_obj) {
        fprintf(stderr, "Error: Failed to allocate block object\n");
        return 0;
    }
    
    Block block = (Block)block_obj;
    
    // Store body AST as raw pointer for now (could be enhanced to be a proper object)
    block->body = (oop)body;
    
    // Store captured environment as raw pointer for now (could be enhanced)
    block->captured_env = (oop)env;
    
    // Store parameter count as Number object
    block->param_count = s72_number_new((double)param_count).obj;
    
    // Store parameter names as Array of String objects (not malloc!)
    if (param_count > 0 && param_names) {
        S72Value param_array = s72_array_new(param_count);
        if (s72_is_nil(param_array)) {
            fprintf(stderr, "Error: Failed to create parameter names array\n");
            return 0;
        }
        
        // Fill array with String objects for each parameter name
        for (int i = 0; i < param_count; i++) {
            S72Value param_name_str = s72_string_new(param_names[i]);
            if (s72_is_nil(param_name_str)) {
                fprintf(stderr, "Error: Failed to create parameter name string\n");
                return 0;
            }
            s72_array_set(param_array, i, param_name_str);
        }
        
        block->param_names = param_array.obj;
    } else {
        // Empty array for no parameters
        S72Value empty_array = s72_array_new(0);
        block->param_names = empty_array.obj;
    }
    
    return block_obj;
}

// Block creation and testing
S72Value s72_block_new(ASTNode *body, S72Env *env, int param_count, char **param_names) {
    oop block_obj = s72_block_new_libid(body, env, param_count, param_names);
    if (!block_obj) {
        return S72_NIL;
    }
    
    S72Value result = {block_obj};
    return result;
}

bool s72_is_block(S72Value val) {
    if (s72_is_nil(val)) return false;
    
    // Check if object has the block vtable
    if (!val.obj) return false;
    
    // Get the vtable from the libid object
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];
    
    // Compare with block vtable
    oop expected_vtable = s72_block_vtable->_vtable[-1];
    return (obj_vtable == expected_vtable);
}

// Block execution
S72Value s72_block_execute(S72Value block_val, int argc, S72Value *argv) {
    if (!s72_is_block(block_val)) {
        s72_error("Value is not a block");
        return S72_NIL;
    }
    
    Block block = (Block)block_val.obj;
    
    // Get parameter count
    S72Value param_count_val = {block->param_count};
    int param_count = (int)s72_number_value(param_count_val);
    
    // Check argument count matches parameter count
    if (argc != param_count) {
        s72_error("Block argument count mismatch");
        return S72_NIL;
    }
    
    // Get AST body and environment
    ASTNode *body = (ASTNode*)block->body;
    S72Env *captured_env = (S72Env*)block->captured_env;
    
    if (!body || !captured_env) {
        s72_error("Block has invalid body or environment");
        return S72_NIL;
    }
    
    // Create new environment for block execution
    S72Env *block_env = s72_env_new(captured_env);
    if (!block_env) {
        s72_error("Failed to create block environment");
        return S72_NIL;
    }
    
    // Bind parameters to arguments
    S72Value param_names_val = {block->param_names};
    for (int i = 0; i < param_count; i++) {
        S72Value param_name_val = s72_array_get(param_names_val, i);
        if (!s72_is_string(param_name_val)) {
            s72_error("Block parameter name is not a string");
            return S72_NIL;
        }
        
        const char *param_name = s72_string_data(param_name_val);
        s72_env_bind(block_env, param_name, argv[i]);
    }
    
    // Execute block body in the new environment
    return s72_eval(body, block_env);
}

// ============================================================================
// Block Methods (Pure libid implementations)
// ============================================================================

oop s72_block_value(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    S72Value self_val = {self};
    S72Value result = s72_block_execute(self_val, 0, NULL);
    
    return result.obj;
}

oop s72_block_value_with(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;
    
    va_list args;
    va_start(args, self);
    oop arg1 = va_arg(args, oop);
    va_end(args);
    
    if (!arg1) {
        s72_error("value: requires one argument");
        return S72_NIL.obj;
    }
    
    S72Value argv[1] = {{arg1}};
    S72Value self_val = {self};
    S72Value result = s72_block_execute(self_val, 1, argv);
    
    return result.obj;
}

oop s72_block_value_with_with(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;

    va_list args;
    va_start(args, self);
    oop arg1 = va_arg(args, oop);
    oop arg2 = va_arg(args, oop);
    va_end(args);

    if (!arg1 || !arg2) {
        s72_error("value:with: requires two arguments");
        return S72_NIL.obj;
    }

    S72Value argv[2] = {{arg1}, {arg2}};
    S72Value self_val = {self};
    S72Value result = s72_block_execute(self_val, 2, argv);

    return result.obj;
}

oop s72_block_print(oop closure, oop state, oop self) {
    (void)closure; (void)state; (void)self;

    // Blocks display as a single quote character
    printf("'");
    return self;
}

// ============================================================================
// Initialization
// ============================================================================

void s72_block_init(void) {
    // Vtable should already be created in main.c
    if (!s72_block_vtable) {
        fprintf(stderr, "Error: Block vtable not initialized\n");
        return;
    }
    
    // Install block methods
    oop sel_value = S72_INTERN("value");
    oop sel_value_with = S72_INTERN("value:");
    oop sel_value_with_with = S72_INTERN("value:with:");
    oop sel_print = S72_INTERN("print");

    S72_METHOD(s72_block_vtable, sel_value, s72_block_value);
    S72_METHOD(s72_block_vtable, sel_value_with, s72_block_value_with);
    S72_METHOD(s72_block_vtable, sel_value_with_with, s72_block_value_with_with);
    S72_METHOD(s72_block_vtable, sel_print, s72_block_print);
    
    // Pure libid block system initialized successfully
}
