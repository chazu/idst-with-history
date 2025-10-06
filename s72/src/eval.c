#include "eval.h"
#include "object.h"
#include "types/number.h"
#include "types/string.h"
#include "types/symbol.h"
#include "types/block.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Need access to _libid
extern struct __libid *_libid;

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
    env->bindings = NULL;
    return env;
}

void s72_env_free(S72Env *env) {
    if (!env) return;

    // Free all bindings
    S72Binding *binding = env->bindings;
    while (binding) {
        S72Binding *next = binding->next;
        free(binding->name);
        free(binding);
        binding = next;
    }

    free(env);
}

// Bind a variable in the environment
void s72_env_bind(S72Env *env, const char *name, S72Value value) {
    if (!env || !name) return;

    printf("DEBUG: s72_env_bind - binding '%s' to %p in env %p\n", name, value.obj, env);

    // Check if binding already exists and update it
    S72Binding *binding = env->bindings;
    while (binding) {
        if (strcmp(binding->name, name) == 0) {
            binding->value = value;
            return;
        }
        binding = binding->next;
    }

    // Create new binding
    binding = malloc(sizeof(S72Binding));
    if (!binding) {
        s72_error("Failed to allocate binding");
        return;
    }

    binding->name = strdup(name);
    if (!binding->name) {
        free(binding);
        s72_error("Failed to copy binding name");
        return;
    }

    binding->value = value;
    binding->next = env->bindings;
    env->bindings = binding;
}

// Look up a variable in the environment chain
S72Value s72_env_lookup(S72Env *env, const char *name) {
    if (!env || !name) return S72_NIL;

    printf("DEBUG: s72_env_lookup - looking up '%s' in env %p\n", name, env);

    // Search current environment
    S72Binding *binding = env->bindings;
    while (binding) {
        if (strcmp(binding->name, name) == 0) {
            printf("DEBUG: Found binding for '%s': %p\n", name, binding->value.obj);
            return binding->value;
        }
        binding = binding->next;
    }

    // Search parent environment
    if (env->parent) {
        return s72_env_lookup(env->parent, name);
    }

    printf("DEBUG: No binding found for '%s'\n", name);
    return S72_NIL;
}

// Check if environment has a binding
bool s72_env_has_binding(S72Env *env, const char *name) {
    S72Value result = s72_env_lookup(env, name);
    return !s72_is_nil(result);
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

    // Check for special constants first
    if (strcmp(name, "true") == 0) {
        return S72_TRUE;
    } else if (strcmp(name, "false") == 0) {
        return S72_FALSE;
    } else if (strcmp(name, "nil") == 0) {
        return S72_NIL;
    } else if (strcmp(name, "Transcript") == 0) {
        return s72_make_transcript();
    }

    // Try variable lookup first
    S72Value var_value = s72_env_lookup(env, name);
    if (!s72_is_nil(var_value)) {
        return var_value;
    }

    // If not found as variable, treat as symbol
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

    // Allow messages to nil (it has methods now)
    if (!receiver.obj) {
        s72_error("Cannot send message to NULL receiver");
        return S72_NIL;
    }

    // Evaluate selector - for M0/M1 we assume it's an atom that should be interned
    printf("DEBUG: Evaluating selector\n");
    printf("DEBUG: Selector AST node type: %d\n", node->data.list.elements[1]->type);

    oop selector = NULL;
    ASTNode *sel_node = node->data.list.elements[1];

    if (sel_node->type == AST_ATOM) {
        // For selectors, intern the atom directly as a libid selector
        printf("DEBUG: Interning atom '%s' as selector\n", sel_node->data.atom.name);
        selector = _libid->intern(sel_node->data.atom.name);
        printf("DEBUG: Interned selector: %p\n", selector);
    } else {
        s72_error("Selector must be an atom (for M0/M1)");
        return S72_NIL;
    }

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

// Evaluate block (create block object with captured environment)
S72Value s72_eval_block(ASTNode *node, S72Env *env) {
    if (!node || node->type != AST_BLOCK) {
        s72_error("Invalid block node");
        return S72_NIL;
    }

    printf("DEBUG: s72_eval_block - creating block with %d statements\n", node->data.block.count);

    // For M2, we create a simple block with no parameters
    // TODO: In later milestones, parse block parameters like [ :x :y | ... ]

    // Create a single AST node for the block body
    // If multiple statements, we'll need to create a compound statement
    ASTNode *body = NULL;
    if (node->data.block.count == 1) {
        body = node->data.block.body[0];
    } else if (node->data.block.count > 1) {
        // Create a list node to hold multiple statements
        body = ast_make_list();
        for (int i = 0; i < node->data.block.count; i++) {
            ast_list_add(body, node->data.block.body[i]);
        }
    }

    // Create block with no parameters for M2
    return s72_block_new(body, env, 0, NULL);
}
