#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Helper functions
static bool is_atom_char(char c) {
    return isalnum(c) || c == '_' || c == '+' || c == '-' || c == '*' || 
           c == '/' || c == '=' || c == '<' || c == '>' || c == ':' || c == '?';
}

static void skip_whitespace(Reader *reader) {
    while (reader->pos < strlen(reader->input)) {
        char c = reader->input[reader->pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            reader->pos++;
            reader->column++;
        } else if (c == '\n') {
            reader->pos++;
            reader->line++;
            reader->column = 1;
        } else if (c == ';') {
            // Skip comment to end of line
            while (reader->pos < strlen(reader->input) && 
                   reader->input[reader->pos] != '\n') {
                reader->pos++;
            }
        } else {
            break;
        }
    }
}

// Reader creation and destruction

Reader *reader_create(const char *input) {
    Reader *reader = malloc(sizeof(Reader));
    if (!reader) {
        s72_error("Out of memory creating reader");
        return NULL;
    }
    
    reader->input = input;
    reader->pos = 0;
    reader->line = 1;
    reader->column = 1;
    reader->has_token = false;
    reader->current_token.value = NULL;
    
    return reader;
}

void reader_destroy(Reader *reader) {
    if (!reader) return;
    
    if (reader->current_token.value) {
        free(reader->current_token.value);
    }
    
    free(reader);
}

// Tokenization

Token reader_next_token(Reader *reader) {
    if (!reader) {
        Token error_token = {TOKEN_ERROR, NULL, 0, 0};
        return error_token;
    }
    
    // Free previous token value
    if (reader->current_token.value) {
        free(reader->current_token.value);
        reader->current_token.value = NULL;
    }
    
    skip_whitespace(reader);
    
    Token token;
    token.line = reader->line;
    token.column = reader->column;
    token.value = NULL;
    
    if (reader->pos >= strlen(reader->input)) {
        token.type = TOKEN_EOF;
        reader->current_token = token;
        return token;
    }
    
    char c = reader->input[reader->pos];
    
    switch (c) {
        case '(':
            token.type = TOKEN_LPAREN;
            token.value = strdup("(");
            reader->pos++;
            reader->column++;
            break;
            
        case ')':
            token.type = TOKEN_RPAREN;
            token.value = strdup(")");
            reader->pos++;
            reader->column++;
            break;
            
        case '[':
            token.type = TOKEN_LBRACKET;
            token.value = strdup("[");
            reader->pos++;
            reader->column++;
            break;
            
        case ']':
            token.type = TOKEN_RBRACKET;
            token.value = strdup("]");
            reader->pos++;
            reader->column++;
            break;
            
        case '\'':
            token.type = TOKEN_QUOTE;
            token.value = strdup("'");
            reader->pos++;
            reader->column++;
            break;
            
        case '"': {
            // String literal
            token.type = TOKEN_STRING;
            int start = ++reader->pos;  // Skip opening quote
            reader->column++;
            
            while (reader->pos < strlen(reader->input) && 
                   reader->input[reader->pos] != '"') {
                if (reader->input[reader->pos] == '\\') {
                    reader->pos++;  // Skip escape character
                    reader->column++;
                }
                reader->pos++;
                reader->column++;
            }
            
            if (reader->pos >= strlen(reader->input)) {
                token.type = TOKEN_ERROR;
                token.value = strdup("Unterminated string");
            } else {
                int len = reader->pos - start;
                token.value = malloc(len + 1);
                strncpy(token.value, reader->input + start, len);
                token.value[len] = '\0';
                reader->pos++;  // Skip closing quote
                reader->column++;
            }
            break;
        }
        
        default:
            if (isdigit(c) || (c == '-' && isdigit(reader->input[reader->pos + 1]))) {
                // Number literal
                token.type = TOKEN_NUMBER;
                int start = reader->pos;
                
                if (c == '-') {
                    reader->pos++;
                    reader->column++;
                }
                
                while (reader->pos < strlen(reader->input) && 
                       (isdigit(reader->input[reader->pos]) || 
                        reader->input[reader->pos] == '.')) {
                    reader->pos++;
                    reader->column++;
                }
                
                int len = reader->pos - start;
                token.value = malloc(len + 1);
                strncpy(token.value, reader->input + start, len);
                token.value[len] = '\0';
                
            } else if (is_atom_char(c)) {
                // Atom/identifier
                token.type = TOKEN_ATOM;
                int start = reader->pos;
                
                while (reader->pos < strlen(reader->input) && 
                       is_atom_char(reader->input[reader->pos])) {
                    reader->pos++;
                    reader->column++;
                }
                
                int len = reader->pos - start;
                token.value = malloc(len + 1);
                strncpy(token.value, reader->input + start, len);
                token.value[len] = '\0';
                
            } else {
                token.type = TOKEN_ERROR;
                char error_msg[64];
                snprintf(error_msg, sizeof(error_msg), "Unexpected character: '%c'", c);
                token.value = strdup(error_msg);
                reader->pos++;
                reader->column++;
            }
            break;
    }
    
    reader->current_token = token;
    reader->has_token = true;
    return token;
}

Token reader_peek_token(Reader *reader) {
    if (!reader->has_token) {
        reader_next_token(reader);
    }
    return reader->current_token;
}

void reader_consume_token(Reader *reader) {
    if (reader->has_token) {
        if (reader->current_token.value) {
            free(reader->current_token.value);
            reader->current_token.value = NULL;
        }
        reader->has_token = false;
    }
}

// Parsing

ASTNode *reader_parse_expression(Reader *reader) {
    Token token = reader_peek_token(reader);
    
    switch (token.type) {
        case TOKEN_EOF:
            return NULL;
            
        case TOKEN_LPAREN:
            return reader_parse_list(reader);
            
        case TOKEN_LBRACKET:
            return reader_parse_block(reader);
            
        case TOKEN_QUOTE: {
            reader_consume_token(reader);  // consume '
            Token next = reader_next_token(reader);
            if (next.type != TOKEN_ATOM) {
                reader_error(reader, "Expected atom after quote");
                return NULL;
            }
            ASTNode *node = ast_make_quote(next.value);
            reader_consume_token(reader);
            return node;
        }
        
        case TOKEN_ATOM: {
            ASTNode *node = ast_make_atom(token.value);
            reader_consume_token(reader);
            return node;
        }
        
        case TOKEN_NUMBER: {
            double value = atof(token.value);
            ASTNode *node = ast_make_number(value);
            reader_consume_token(reader);
            return node;
        }
        
        case TOKEN_STRING: {
            ASTNode *node = ast_make_string(token.value);
            reader_consume_token(reader);
            return node;
        }
        
        default:
            reader_error(reader, "Unexpected token in expression");
            return NULL;
    }
}

ASTNode *reader_parse_list(Reader *reader) {
    Token token = reader_peek_token(reader);
    if (token.type != TOKEN_LPAREN) {
        reader_error(reader, "Expected '(' at start of list");
        return NULL;
    }
    reader_consume_token(reader);  // consume (
    
    ASTNode *list = ast_make_list();
    
    while (true) {
        token = reader_peek_token(reader);
        if (token.type == TOKEN_EOF) {
            reader_error(reader, "Unexpected EOF in list");
            ast_free(list);
            return NULL;
        }
        
        if (token.type == TOKEN_RPAREN) {
            reader_consume_token(reader);  // consume )
            break;
        }
        
        ASTNode *element = reader_parse_expression(reader);
        if (!element) {
            ast_free(list);
            return NULL;
        }
        
        ast_list_add(list, element);
    }
    
    return list;
}

ASTNode *reader_parse_block(Reader *reader) {
    Token token = reader_peek_token(reader);
    if (token.type != TOKEN_LBRACKET) {
        reader_error(reader, "Expected '[' at start of block");
        return NULL;
    }
    reader_consume_token(reader);  // consume [
    
    ASTNode *block = ast_make_block();
    
    while (true) {
        token = reader_peek_token(reader);
        if (token.type == TOKEN_EOF) {
            reader_error(reader, "Unexpected EOF in block");
            ast_free(block);
            return NULL;
        }
        
        if (token.type == TOKEN_RBRACKET) {
            reader_consume_token(reader);  // consume ]
            break;
        }
        
        ASTNode *element = reader_parse_expression(reader);
        if (!element) {
            ast_free(block);
            return NULL;
        }
        
        ast_block_add(block, element);
    }
    
    return block;
}

// High-level interface

ASTNode *s72_read_string(const char *input) {
    Reader *reader = reader_create(input);
    if (!reader) return NULL;

    ASTNode *result = reader_parse_expression(reader);
    reader_destroy(reader);

    return result;
}

ASTNode *s72_read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        s72_error("Cannot open file: %s", filename);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer
    char *content = malloc(size + 1);
    if (!content) {
        fclose(file);
        s72_error("Out of memory reading file: %s", filename);
        return NULL;
    }

    // Read file content
    size_t bytes_read = fread(content, 1, size, file);
    fclose(file);

    if (bytes_read != size) {
        free(content);
        s72_error("Error reading file: %s", filename);
        return NULL;
    }

    content[size] = '\0';

    // Parse content
    ASTNode *result = s72_read_string(content);
    free(content);

    return result;
}

// Error handling

void reader_error(Reader *reader, const char *message) {
    fprintf(stderr, "Parse error at line %d, column %d: %s\n", 
            reader->line, reader->column, message);
}
