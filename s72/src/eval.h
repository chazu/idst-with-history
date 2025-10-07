#ifndef EVAL_H
#define EVAL_H

#include "s72.h"
#include "ast.h"
#include "env.h"

// Evaluator functions
S72Value s72_eval(ASTNode *node, S72Env *env);
S72Value s72_eval_atom(ASTNode *node, S72Env *env);
S72Value s72_eval_number(ASTNode *node, S72Env *env);
S72Value s72_eval_string(ASTNode *node, S72Env *env);
S72Value s72_eval_quote(ASTNode *node, S72Env *env);
S72Value s72_eval_list(ASTNode *node, S72Env *env);
S72Value s72_eval_block(ASTNode *node, S72Env *env);

// Environment management is now in env.h

// Evaluator initialization
void s72_eval_init(void);

#endif // EVAL_H
