#ifndef EVAL_H
#define EVAL_H

#include "s72.h"
#include "ast.h"

// Variable binding structure
typedef struct S72Binding {
    char *name;
    S72Value value;
    struct S72Binding *next;
} S72Binding;

// Environment structure (enhanced for M2)
typedef struct S72Env {
    struct S72Env *parent;
    S72Binding *bindings;  // Linked list of variable bindings
} S72Env;

// Evaluator functions
S72Value s72_eval(ASTNode *node, S72Env *env);
S72Value s72_eval_atom(ASTNode *node, S72Env *env);
S72Value s72_eval_number(ASTNode *node, S72Env *env);
S72Value s72_eval_string(ASTNode *node, S72Env *env);
S72Value s72_eval_quote(ASTNode *node, S72Env *env);
S72Value s72_eval_list(ASTNode *node, S72Env *env);
S72Value s72_eval_block(ASTNode *node, S72Env *env);

// Environment management
S72Env *s72_env_new(S72Env *parent);
void s72_env_free(S72Env *env);
void s72_env_bind(S72Env *env, const char *name, S72Value value);
S72Value s72_env_lookup(S72Env *env, const char *name);
bool s72_env_has_binding(S72Env *env, const char *name);

// Global environment
extern S72Env *s72_global_env;

// Evaluator initialization
void s72_eval_init(void);

#endif // EVAL_H
