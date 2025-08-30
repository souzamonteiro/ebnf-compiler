#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NODE_RULE,
    NODE_EXPRESSION,
    NODE_TERM,
    NODE_FACTOR,
    NODE_PRIMARY,
    NODE_IDENTIFIER,
    NODE_TERMINAL,
    NODE_OPTIONAL,
    NODE_REPEATED,
    NODE_GROUP,
    NODE_SPECIAL_SEQUENCE,
    NODE_CHAR_RANGE,
    NODE_DIRECTIVE,
    NODE_REPETITION,
    NODE_INTEGER,
    NODE_HEX_CHAR,
    NODE_STRING
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* value;
    struct ASTNode** children;
    int children_count;
} ASTNode;

ASTNode* create_node(NodeType type, const char* value);
void add_child(ASTNode* parent, ASTNode* child);
void print_ast(ASTNode* node, int depth);
void free_ast(ASTNode* node);

#endif