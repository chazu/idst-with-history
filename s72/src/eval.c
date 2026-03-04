#include "eval.h"
#include "env.h"
#include "object.h"
#include "types/number.h"
#include "types/string.h"
#include "types/symbol.h"
#include "types/block.h"
#include "types/transcript.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Need access to _libid
extern struct __libid *_libid;

// Global variable management using the environment system
static void s72_def_global(const char *name, S72Value value) {
    S72Env *global_env = s72_env_get_global();
    if (global_env) {
        s72_env_bind(global_env, name, value);
        printf("DEBUG: def bound global variable '%s' to %p\n", name, value.obj);
    } else {
        s72_error("Global environment not initialized");
    }
}

static S72Value s72_get_global(const char *name) {
    S72Env *global_env = s72_env_get_global();
    if (global_env) {
        return s72_env_lookup(global_env, name);
    }
    return S72_NIL;
}

static bool s72_set_global(const char *name, S72Value value) {
    S72Env *global_env = s72_env_get_global();
    if (global_env) {
        // Try to set existing variable first
        if (s72_env_set(global_env, name, value)) {
            printf("DEBUG: set updated global variable '%s' to %p\n", name, value.obj);
            return true;
        } else {
            // Variable doesn't exist, create it
            s72_env_bind(global_env, name, value);
            printf("DEBUG: set created global variable '%s' with value %p\n", name, value.obj);
            return true;
        }
    }
    return false;
}

// Evaluator initialization
void s72_eval_init(void) {
    // Use the new environment system
    s72_env_init_global();
}

// Environment management functions are now in env.c

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

    // Try global variable lookup
    S72Value global_value = s72_get_global(name);
    if (!s72_is_nil(global_value)) {
        printf("DEBUG: Found global variable '%s'\n", name);
        return global_value;
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

    // Check if this is a global function call
    // Format: (function_name arg1 arg2 ...)
    if (node->data.list.elements[0]->type == AST_ATOM) {
        const char *func_name = node->data.list.elements[0]->data.atom.name;

        // Handle built-in global functions
        if (strcmp(func_name, "show:") == 0 && node->data.list.count == 2) {
            S72Value arg = s72_eval(node->data.list.elements[1], env);
            S72Value transcript = s72_transcript_singleton();
            oop print_sel = _libid->intern("print:");
            oop cr_sel = _libid->intern("cr");
            S72Value args[1] = { arg };
            s72_object_send(transcript, print_sel, 1, args);
            s72_object_send(transcript, cr_sel, 0, NULL);
            return S72_NIL;
        }
        else if (strcmp(func_name, "println:") == 0 && node->data.list.count == 2) {
            S72Value arg = s72_eval(node->data.list.elements[1], env);
            S72Value transcript = s72_transcript_singleton();
            oop print_sel = _libid->intern("print:");
            oop cr_sel = _libid->intern("cr");
            S72Value args[1] = { arg };
            s72_object_send(transcript, print_sel, 1, args);
            s72_object_send(transcript, cr_sel, 0, NULL);
            return S72_NIL;
        }
        else if (strcmp(func_name, "def") == 0 && node->data.list.count == 3) {
            // Extract variable name and value
            ASTNode *name_node = node->data.list.elements[1];
            if (name_node->type != AST_ATOM) {
                fprintf(stderr, "Error: def expects a symbol name\n");
                return S72_NIL;
            }

            S72Value value = s72_eval(node->data.list.elements[2], env);
            const char *var_name = name_node->data.atom.name;

            // Store the global variable using the environment system
            s72_def_global(var_name, value);

            return S72_NIL;  // def should return nil like most Lisps
        }
        else if (strcmp(func_name, "let") == 0 && node->data.list.count == 3) {
            // let creates a local variable in the current environment
            ASTNode *name_node = node->data.list.elements[1];
            if (name_node->type != AST_ATOM) {
                fprintf(stderr, "Error: let expects a symbol name\n");
                return S72_NIL;
            }

            S72Value value = s72_eval(node->data.list.elements[2], env);
            const char *var_name = name_node->data.atom.name;

            // Bind in current environment (local variable)
            s72_env_bind(env, var_name, value);
            printf("DEBUG: let bound local variable '%s' to %p\n", var_name, value.obj);

            return S72_NIL;  // let should return nil like most Lisps
        }
        else if (strcmp(func_name, "set") == 0 && node->data.list.count == 3) {
            // set modifies an existing variable (searches environment chain)
            ASTNode *name_node = node->data.list.elements[1];
            if (name_node->type != AST_ATOM) {
                fprintf(stderr, "Error: set expects a symbol name\n");
                return S72_NIL;
            }

            S72Value value = s72_eval(node->data.list.elements[2], env);
            const char *var_name = name_node->data.atom.name;

            // Try to set in environment chain first
            if (s72_env_set(env, var_name, value)) {
                printf("DEBUG: set updated local variable '%s' to %p\n", var_name, value.obj);
                return S72_NIL;  // set should return nil like most Lisps
            }

            // If not found locally, try global
            if (s72_set_global(var_name, value)) {
                return S72_NIL;  // set should return nil like most Lisps
            }

            // Variable not found anywhere
            fprintf(stderr, "Error: set cannot find variable '%s'\n", var_name);
            return S72_NIL;
        }
        else if (strcmp(func_name, "not:") == 0 && node->data.list.count == 2) {
            S72Value arg = s72_eval(node->data.list.elements[1], env);
            if (arg.obj == S72_TRUE.obj) {
                return S72_FALSE;
            } else {
                return S72_TRUE;
            }
        }
        else if (strcmp(func_name, "and:") == 0 && node->data.list.count == 3) {
            S72Value a = s72_eval(node->data.list.elements[1], env);
            S72Value b = s72_eval(node->data.list.elements[2], env);
            if ((a.obj == S72_FALSE.obj || a.obj == S72_NIL.obj) ||
                (b.obj == S72_FALSE.obj || b.obj == S72_NIL.obj)) {
                return S72_FALSE;
            } else {
                return S72_TRUE;
            }
        }
        else if (strcmp(func_name, "or:") == 0 && node->data.list.count == 3) {
            S72Value a = s72_eval(node->data.list.elements[1], env);
            S72Value b = s72_eval(node->data.list.elements[2], env);
            if ((a.obj != S72_FALSE.obj && a.obj != S72_NIL.obj) ||
                (b.obj != S72_FALSE.obj && b.obj != S72_NIL.obj)) {
                return S72_TRUE;
            } else {
                return S72_FALSE;
            }
        }
        else if (strcmp(func_name, "abs:") == 0 && node->data.list.count == 2) {
            S72Value arg = s72_eval(node->data.list.elements[1], env);
            if (s72_is_number(arg)) {
                double val = s72_number_value(arg);
                return s72_number_new(val < 0 ? -val : val);
            }
            return S72_NIL;
        }
        else if (strcmp(func_name, "min:") == 0 && node->data.list.count == 3) {
            S72Value a = s72_eval(node->data.list.elements[1], env);
            S72Value b = s72_eval(node->data.list.elements[2], env);
            if (s72_is_number(a) && s72_is_number(b)) {
                double va = s72_number_value(a);
                double vb = s72_number_value(b);
                return s72_number_new(va < vb ? va : vb);
            }
            return S72_NIL;
        }
        else if (strcmp(func_name, "max:") == 0 && node->data.list.count == 3) {
            S72Value a = s72_eval(node->data.list.elements[1], env);
            S72Value b = s72_eval(node->data.list.elements[2], env);
            if (s72_is_number(a) && s72_is_number(b)) {
                double va = s72_number_value(a);
                double vb = s72_number_value(b);
                return s72_number_new(va > vb ? va : vb);
            }
            return S72_NIL;
        }
        else if (strcmp(func_name, "square:") == 0 && node->data.list.count == 2) {
            S72Value arg = s72_eval(node->data.list.elements[1], env);
            if (s72_is_number(arg)) {
                double val = s72_number_value(arg);
                return s72_number_new(val * val);
            }
            return S72_NIL;
        }
        else if (strcmp(func_name, "assert:") == 0 && node->data.list.count == 2) {
            S72Value condition = s72_eval(node->data.list.elements[1], env);
            if (condition.obj == S72_FALSE.obj || condition.obj == S72_NIL.obj) {
                S72Value transcript = s72_transcript_singleton();
                oop print_sel = _libid->intern("print:");
                oop cr_sel = _libid->intern("cr");
                S72Value msg = s72_string_new("ASSERTION FAILED!");
                S72Value args[1] = { msg };
                s72_object_send(transcript, print_sel, 1, args);
                s72_object_send(transcript, cr_sel, 0, NULL);
            }
            return S72_NIL;
        }
    }

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

    printf("DEBUG: s72_eval_block - creating block with %d statements and %d parameters\n",
           node->data.block.count, node->data.block.param_count);

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

    // Create block with parsed parameters
    return s72_block_new(body, env, node->data.block.param_count, node->data.block.parameters);
}
