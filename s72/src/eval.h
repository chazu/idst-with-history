#ifndef EVAL_H
#define EVAL_H

#include "s72.h"
#include "ast.h"

// Environment structure (simplified for M0)
typedef struct S72Env {
    struct S72Env *parent;
    // For M0, we don't need variable bindings yet
    // This will be expanded in later milestones
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

// Global environment
extern S72Env *s72_global_env;

// Evaluator initialization
void s72_eval_init(void);

#endif // EVAL_H
