#ifndef S72_BLOCK_H
#define S72_BLOCK_H

#include "../s72.h"
#include "../ast.h"
#include "../eval.h"

// Block type structure
typedef struct {
    ASTNode *body;          // AST nodes to execute
    S72Env *captured_env;   // Environment captured when block was created
    int param_count;        // Number of parameters
    char **param_names;     // Parameter names (for binding)
} S72Block;

// Block type functions
void s72_block_init(void);
S72Value s72_block_new(ASTNode *body, S72Env *env, int param_count, char **param_names);
bool s72_is_block(S72Value val);

// Block execution
S72Value s72_block_execute(S72Value block, int argc, S72Value *argv);

// Block methods (native implementations)
oop s72_block_value(oop closure, oop state, oop receiver);
oop s72_block_value_with(oop closure, oop state, oop receiver, oop arg1);
oop s72_block_value_with_with(oop closure, oop state, oop receiver, oop arg1, oop arg2);

#endif // S72_BLOCK_H
