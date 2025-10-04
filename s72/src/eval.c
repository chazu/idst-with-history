#include "eval.h"
#include "object.h"
#include "types/number.h"
#include "types/string.h"
#include "types/symbol.h"
#include <stdio.h>

// Global environment
S72Env *s72_global_env = NULL;

// Evaluator initialization
void s72_eval_init(void) {
    s72_global_env = s72_env_new(NULL);
}

// Environment management
S72Env *s72_env_new(S72Env *parent) {
    S72Env *env = malloc(sizeof(S72Env));
    if (!env) {
        s72_error("Failed to allocate environment");
        return NULL;
    }
    
    env->parent = parent;
    return env;
}

void s72_env_free(S72Env *env) {
    if (!env) return;
    
    // For M0, nothing to free yet
    // Later milestones will need to free variable bindings
    free(env);
}

// Main evaluator
S72Value s72_eval(ASTNode *node, S72Env *env) {
    if (!node) {
        return S72_NIL;
    }
    
    switch (node->type) {
        case AST_ATOM:
            return s72_eval_atom(node, env);
            
        case AST_NUMBER:
            return s72_eval_number(node, env);
            
        case AST_STRING:
            return s72_eval_string(node, env);
            
        case AST_QUOTE:
            return s72_eval_quote(node, env);
            
        case AST_LIST:
            return s72_eval_list(node, env);
            
        case AST_BLOCK:
            return s72_eval_block(node, env);
            
        default:
            s72_error("Unknown AST node type: %d", node->type);
            return S72_NIL;
    }
}

// Evaluate atom (variable lookup or symbol)
S72Value s72_eval_atom(ASTNode *node, S72Env *env) {
    if (!node || node->type != AST_ATOM) {
        s72_error("Invalid atom node");
        return S72_NIL;
    }
    
    const char *name = node->data.atom.name;
    
    // For M0, we don't have variable bindings yet
    // All atoms are treated as symbols
    return s72_symbol_new(name);
}

// Evaluate number literal
S72Value s72_eval_number(ASTNode *node, S72Env *env) {
    if (!node || node->type != AST_NUMBER) {
        s72_error("Invalid number node");
        return S72_NIL;
    }
    
    return s72_number_new(node->data.number.value);
}

// Evaluate string literal
S72Value s72_eval_string(ASTNode *node, S72Env *env) {
    if (!node || node->type != AST_STRING) {
        s72_error("Invalid string node");
        return S72_NIL;
    }
    
    return s72_string_new(node->data.string.value);
}

// Evaluate quoted symbol
S72Value s72_eval_quote(ASTNode *node, S72Env *env) {
    if (!node || node->type != AST_QUOTE) {
        s72_error("Invalid quote node");
        return S72_NIL;
    }
    
    return s72_symbol_new(node->data.quote.symbol);
}

// Evaluate list (message send)
S72Value s72_eval_list(ASTNode *node, S72Env *env) {
    if (!node || node->type != AST_LIST) {
        s72_error("Invalid list node");
        return S72_NIL;
    }
    
    if (node->data.list.count == 0) {
        // Empty list evaluates to nil
        return S72_NIL;
    }
    
    // For M0, we only support simple message sends
    // Format: (receiver selector arg1 arg2 ...)
    
    if (node->data.list.count < 2) {
        s72_error("Message send requires at least receiver and selector");
        return S72_NIL;
    }
    
    // Evaluate receiver
    S72Value receiver = s72_eval(node->data.list.elements[0], env);
    if (s72_is_nil(receiver)) {
        s72_error("Cannot send message to nil");
        return S72_NIL;
    }
    
    // Evaluate selector (must be a symbol)
    S72Value selector_val = s72_eval(node->data.list.elements[1], env);
    if (!s72_is_symbol(selector_val)) {
        s72_error("Selector must be a symbol");
        return S72_NIL;
    }
    
    oop selector = s72_symbol_oop(selector_val);
    
    // Evaluate arguments
    int argc = node->data.list.count - 2;
    S72Value *argv = NULL;
    
    if (argc > 0) {
        argv = malloc(argc * sizeof(S72Value));
        if (!argv) {
            s72_error("Failed to allocate argument array");
            return S72_NIL;
        }
        
        for (int i = 0; i < argc; i++) {
            argv[i] = s72_eval(node->data.list.elements[i + 2], env);
        }
    }
    
    // Send the message
    S72Value result = s72_object_send(receiver, selector, argc, argv);
    
    if (argv) {
        free(argv);
    }
    
    return result;
}

// Evaluate block (for M0, just return a placeholder)
S72Value s72_eval_block(ASTNode *node, S72Env *env) {
    if (!node || node->type != AST_BLOCK) {
        s72_error("Invalid block node");
        return S72_NIL;
    }
    
    // For M0, blocks are not fully implemented
    // We'll return a placeholder symbol
    return s72_symbol_new("block");
}
