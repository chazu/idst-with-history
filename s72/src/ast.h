#ifndef AST_H
#define AST_H

#include "s72.h"

// AST Node types
typedef enum {
    AST_ATOM,       // Symbol or variable reference
    AST_NUMBER,     // Number literal
    AST_STRING,     // String literal  
    AST_QUOTE,      // 'symbol
    AST_LIST,       // (expr ...)
    AST_BLOCK       // [expr ...]
} ASTNodeType;

// Forward declaration
typedef struct ASTNode ASTNode;

// AST Node structure
struct ASTNode {
    ASTNodeType type;
    union {
        struct {
            char *name;
        } atom;
        
        struct {
            double value;
        } number;
        
        struct {
            char *value;
        } string;
        
        struct {
            char *symbol;
        } quote;
        
        struct {
            ASTNode **elements;
            int count;
            int capacity;
        } list;
        
        struct {
            ASTNode **body;
            int count;
            int capacity;
        } block;
    } data;
};

// AST creation functions
ASTNode *ast_make_atom(const char *name);
ASTNode *ast_make_number(double value);
ASTNode *ast_make_string(const char *value);
ASTNode *ast_make_quote(const char *symbol);
ASTNode *ast_make_list(void);
ASTNode *ast_make_block(void);

// AST manipulation
void ast_list_add(ASTNode *list, ASTNode *element);
void ast_block_add(ASTNode *block, ASTNode *element);

// AST cleanup
void ast_free(ASTNode *node);

// AST printing (for debugging)
void ast_print(ASTNode *node);
char *ast_to_string(ASTNode *node);

#endif // AST_H
