#ifndef S72_BLOCK_H
#define S72_BLOCK_H

#include "../s72.h"
#include "../ast.h"
#include "../eval.h"

// Pure libid block object structure
typedef struct t_Block {
    oop body;           // AST node stored as oop (could be enhanced)
    oop captured_env;   // Environment object (could be enhanced)
    oop param_count;    // Number object for parameter count
    oop param_names;    // Array object containing String objects for parameter names
} *Block;

// Block type functions
void s72_block_init(void);
S72Value s72_block_new(ASTNode *body, S72Env *env, int param_count, char **param_names);
bool s72_is_block(S72Value val);

// Block execution
S72Value s72_block_execute(S72Value block, int argc, S72Value *argv);

// Block methods (pure libid implementations with variadic signatures)
oop s72_block_value(oop closure, oop state, oop self);
oop s72_block_value_with(oop closure, oop state, oop self, ...);
oop s72_block_value_with_with(oop closure, oop state, oop self, ...);
oop s72_block_print(oop closure, oop state, oop self);

// Block object creation
oop s72_block_new_libid(ASTNode *body, S72Env *env, int param_count, char **param_names);

#endif // S72_BLOCK_H
