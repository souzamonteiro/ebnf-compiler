#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NODE_SYNTAX,
    NODE_RULE,
    NODE_DIRECTIVE,
    NODE_DEFINITIONS_LIST,
    NODE_SINGLE_DEFINITION,
    NODE_SYNTACTIC_TERM,
    NODE_SYNTACTIC_FACTOR,
    NODE_SYNTACTIC_PRIMARY,
    NODE_OPTIONAL_SEQUENCE,
    NODE_REPEATED_SEQUENCE,
    NODE_GROUPED_SEQUENCE,
    NODE_META_IDENTIFIER,
    NODE_INTEGER,
    NODE_TERMINAL_STRING,
    NODE_SPECIAL_SEQUENCE,
    NODE_EMPTY_SEQUENCE,
    NODE_CHAR_RANGE,
    NODE_HEX_CHAR,
    NODE_CONCATENATION,
    NODE_ALTERNATIVE
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* value;
    struct ASTNode** children;
    int children_count;
} ASTNode;

ASTNode* create_node(NodeType type, const char* value);
void free_ast(ASTNode* node);
void add_child(ASTNode* parent, ASTNode* child);
const char* node_type_to_string(NodeType type);
char* xml_escape(const char* input);
void print_ast_xml(ASTNode* node, int depth, FILE* output);
void print_ast(ASTNode* node, int depth);

#endif