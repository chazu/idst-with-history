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
    printf("DEBUG: s72_eval called with node type %d\n", node ? node->type : -1);

    if (!node) {
        return S72_NIL;
    }

    switch (node->type) {
        case AST_ATOM:
            printf("DEBUG: Evaluating atom\n");
            return s72_eval_atom(node, env);

        case AST_NUMBER:
            printf("DEBUG: Evaluating number\n");
            return s72_eval_number(node, env);

        case AST_STRING:
            printf("DEBUG: Evaluating string\n");
            return s72_eval_string(node, env);

        case AST_QUOTE:
            printf("DEBUG: Evaluating quote\n");
            return s72_eval_quote(node, env);

        case AST_LIST:
            printf("DEBUG: Evaluating list\n");
            return s72_eval_list(node, env);

        case AST_BLOCK:
            printf("DEBUG: Evaluating block\n");
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
    printf("DEBUG: s72_eval_list called\n");

    if (!node || node->type != AST_LIST) {
        s72_error("Invalid list node");
        return S72_NIL;
    }

    if (node->data.list.count == 0) {
        // Empty list evaluates to nil
        return S72_NIL;
    }

    printf("DEBUG: List has %d elements\n", node->data.list.count);

    // For M1, we support simple message sends
    // Format: (receiver selector arg1 arg2 ...)

    if (node->data.list.count < 2) {
        s72_error("Message send requires at least receiver and selector");
        return S72_NIL;
    }

    // Evaluate receiver
    printf("DEBUG: Evaluating receiver\n");
    S72Value receiver = s72_eval(node->data.list.elements[0], env);
    printf("DEBUG: Receiver evaluated to %p\n", receiver.obj);

    if (s72_is_nil(receiver)) {
        s72_error("Cannot send message to nil");
        return S72_NIL;
    }

    // Evaluate selector (must be a symbol)
    printf("DEBUG: Evaluating selector\n");
    printf("DEBUG: Selector AST node type: %d\n", node->data.list.elements[1]->type);
    S72Value selector_val = s72_eval(node->data.list.elements[1], env);
    printf("DEBUG: Selector evaluated to %p\n", selector_val.obj);

    printf("DEBUG: About to check if selector is symbol\n");
    if (!s72_is_symbol(selector_val)) {
        s72_error("Selector must be a symbol");
        return S72_NIL;
    }
    printf("DEBUG: Selector is confirmed to be a symbol\n");

    printf("DEBUG: About to call s72_symbol_oop\n");
    oop selector = s72_symbol_oop(selector_val);
    printf("DEBUG: s72_symbol_oop returned %p\n", selector);

    // Evaluate arguments
    int argc = node->data.list.count - 2;
    S72Value *argv = NULL;

    printf("DEBUG: Evaluating %d arguments\n", argc);

    if (argc > 0) {
        argv = malloc(argc * sizeof(S72Value));
        if (!argv) {
            s72_error("Failed to allocate argument array");
            return S72_NIL;
        }

        for (int i = 0; i < argc; i++) {
            printf("DEBUG: Evaluating argument %d\n", i);
            argv[i] = s72_eval(node->data.list.elements[i + 2], env);
            printf("DEBUG: Argument %d evaluated to %p\n", i, argv[i].obj);
        }
    }

    // Send the message
    printf("DEBUG: About to call s72_object_send\n");
    S72Value result = s72_object_send(receiver, selector, argc, argv);
    printf("DEBUG: s72_object_send returned\n");

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
