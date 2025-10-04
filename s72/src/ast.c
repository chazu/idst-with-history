#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AST Node creation functions

ASTNode *ast_make_atom(const char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        s72_error("Out of memory creating atom node");
        return NULL;
    }
    
    node->type = AST_ATOM;
    node->data.atom.name = strdup(name);
    if (!node->data.atom.name) {
        free(node);
        s72_error("Out of memory duplicating atom name");
        return NULL;
    }
    
    return node;
}

ASTNode *ast_make_number(double value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        s72_error("Out of memory creating number node");
        return NULL;
    }
    
    node->type = AST_NUMBER;
    node->data.number.value = value;
    
    return node;
}

ASTNode *ast_make_string(const char *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        s72_error("Out of memory creating string node");
        return NULL;
    }
    
    node->type = AST_STRING;
    node->data.string.value = strdup(value);
    if (!node->data.string.value) {
        free(node);
        s72_error("Out of memory duplicating string value");
        return NULL;
    }
    
    return node;
}

ASTNode *ast_make_quote(const char *symbol) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        s72_error("Out of memory creating quote node");
        return NULL;
    }
    
    node->type = AST_QUOTE;
    node->data.quote.symbol = strdup(symbol);
    if (!node->data.quote.symbol) {
        free(node);
        s72_error("Out of memory duplicating quote symbol");
        return NULL;
    }
    
    return node;
}

ASTNode *ast_make_list(void) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        s72_error("Out of memory creating list node");
        return NULL;
    }
    
    node->type = AST_LIST;
    node->data.list.elements = NULL;
    node->data.list.count = 0;
    node->data.list.capacity = 0;
    
    return node;
}

ASTNode *ast_make_block(void) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        s72_error("Out of memory creating block node");
        return NULL;
    }
    
    node->type = AST_BLOCK;
    node->data.block.body = NULL;
    node->data.block.count = 0;
    node->data.block.capacity = 0;
    
    return node;
}

// AST manipulation functions

void ast_list_add(ASTNode *list, ASTNode *element) {
    if (!list || list->type != AST_LIST) {
        s72_error("ast_list_add: not a list node");
        return;
    }
    
    if (list->data.list.count >= list->data.list.capacity) {
        int new_capacity = list->data.list.capacity == 0 ? 4 : list->data.list.capacity * 2;
        ASTNode **new_elements = realloc(list->data.list.elements, 
                                        new_capacity * sizeof(ASTNode*));
        if (!new_elements) {
            s72_error("Out of memory expanding list");
            return;
        }
        list->data.list.elements = new_elements;
        list->data.list.capacity = new_capacity;
    }
    
    list->data.list.elements[list->data.list.count++] = element;
}

void ast_block_add(ASTNode *block, ASTNode *element) {
    if (!block || block->type != AST_BLOCK) {
        s72_error("ast_block_add: not a block node");
        return;
    }
    
    if (block->data.block.count >= block->data.block.capacity) {
        int new_capacity = block->data.block.capacity == 0 ? 4 : block->data.block.capacity * 2;
        ASTNode **new_body = realloc(block->data.block.body, 
                                    new_capacity * sizeof(ASTNode*));
        if (!new_body) {
            s72_error("Out of memory expanding block");
            return;
        }
        block->data.block.body = new_body;
        block->data.block.capacity = new_capacity;
    }
    
    block->data.block.body[block->data.block.count++] = element;
}

// AST cleanup

void ast_free(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_ATOM:
            free(node->data.atom.name);
            break;
            
        case AST_STRING:
            free(node->data.string.value);
            break;
            
        case AST_QUOTE:
            free(node->data.quote.symbol);
            break;
            
        case AST_LIST:
            for (int i = 0; i < node->data.list.count; i++) {
                ast_free(node->data.list.elements[i]);
            }
            free(node->data.list.elements);
            break;
            
        case AST_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                ast_free(node->data.block.body[i]);
            }
            free(node->data.block.body);
            break;
            
        case AST_NUMBER:
            // Nothing to free
            break;
    }
    
    free(node);
}

// AST printing (for debugging)

void ast_print(ASTNode *node) {
    if (!node) {
        printf("(null)");
        return;
    }
    
    switch (node->type) {
        case AST_ATOM:
            printf("%s", node->data.atom.name);
            break;
            
        case AST_NUMBER:
            printf("%.15g", node->data.number.value);
            break;
            
        case AST_STRING:
            printf("\"%s\"", node->data.string.value);
            break;
            
        case AST_QUOTE:
            printf("'%s", node->data.quote.symbol);
            break;
            
        case AST_LIST:
            printf("(");
            for (int i = 0; i < node->data.list.count; i++) {
                if (i > 0) printf(" ");
                ast_print(node->data.list.elements[i]);
            }
            printf(")");
            break;
            
        case AST_BLOCK:
            printf("[");
            for (int i = 0; i < node->data.block.count; i++) {
                if (i > 0) printf(" ");
                ast_print(node->data.block.body[i]);
            }
            printf("]");
            break;
    }
}

char *ast_to_string(ASTNode *node) {
    // For M0, we'll use a simple static buffer approach
    // TODO: Make this more robust for production
    static char buffer[1024];
    
    if (!node) {
        strcpy(buffer, "(null)");
        return buffer;
    }
    
    switch (node->type) {
        case AST_ATOM:
            snprintf(buffer, sizeof(buffer), "%s", node->data.atom.name);
            break;
            
        case AST_NUMBER:
            snprintf(buffer, sizeof(buffer), "%.15g", node->data.number.value);
            break;
            
        case AST_STRING:
            snprintf(buffer, sizeof(buffer), "\"%s\"", node->data.string.value);
            break;
            
        case AST_QUOTE:
            snprintf(buffer, sizeof(buffer), "'%s", node->data.quote.symbol);
            break;
            
        case AST_LIST:
            strcpy(buffer, "(");
            for (int i = 0; i < node->data.list.count; i++) {
                if (i > 0) strcat(buffer, " ");
                // Simplified - just add atom names for now
                if (node->data.list.elements[i]->type == AST_ATOM) {
                    strcat(buffer, node->data.list.elements[i]->data.atom.name);
                }
            }
            strcat(buffer, ")");
            break;
            
        case AST_BLOCK:
            strcpy(buffer, "[...]");  // Simplified for M0
            break;
    }
    
    return buffer;
}
