#ifndef READER_H
#define READER_H

#include "ast.h"

// Token types
typedef enum {
    TOKEN_EOF,
    TOKEN_LPAREN,      // (
    TOKEN_RPAREN,      // )
    TOKEN_LBRACKET,    // [
    TOKEN_RBRACKET,    // ]
    TOKEN_QUOTE,       // '
    TOKEN_ATOM,        // identifier/symbol
    TOKEN_NUMBER,      // numeric literal
    TOKEN_STRING,      // string literal
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char *value;       // Token text (owned by token)
    int line;          // Line number for error reporting
    int column;        // Column number for error reporting
} Token;

// Reader state
typedef struct {
    const char *input;
    int pos;
    int line;
    int column;
    Token current_token;
    bool has_token;
} Reader;

// Reader functions
Reader *reader_create(const char *input);
void reader_destroy(Reader *reader);

// Tokenization
Token reader_next_token(Reader *reader);
Token reader_peek_token(Reader *reader);
void reader_consume_token(Reader *reader);

// Parsing
ASTNode *reader_parse_expression(Reader *reader);
ASTNode *reader_parse_list(Reader *reader);
ASTNode *reader_parse_block(Reader *reader);

// High-level interface
ASTNode *s72_read_string(const char *input);

// Error handling
void reader_error(Reader *reader, const char *message);

#endif // READER_H
