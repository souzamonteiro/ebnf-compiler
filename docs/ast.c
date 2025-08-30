#include "ast.h"

ASTNode* create_node(NodeType type, const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->children = NULL;
    node->children_count = 0;
    return node;
}

void add_child(ASTNode* parent, ASTNode* child) {
    parent->children_count++;
    parent->children = (ASTNode**)realloc(parent->children, 
                                        parent->children_count * sizeof(ASTNode*));
    parent->children[parent->children_count - 1] = child;
}

const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_SYNTAX: return "SYNTAX";
        case NODE_RULE: return "RULE";
        case NODE_DIRECTIVE: return "DIRECTIVE";
        case NODE_DEFINITIONS_LIST: return "DEFINITIONS_LIST";
        case NODE_SINGLE_DEFINITION: return "SINGLE_DEFINITION";
        case NODE_SYNTACTIC_TERM: return "SYNTACTIC_TERM";
        case NODE_SYNTACTIC_FACTOR: return "SYNTACTIC_FACTOR";
        case NODE_SYNTACTIC_PRIMARY: return "SYNTACTIC_PRIMARY";
        case NODE_OPTIONAL_SEQUENCE: return "OPTIONAL_SEQUENCE";
        case NODE_REPEATED_SEQUENCE: return "REPEATED_SEQUENCE";
        case NODE_GROUPED_SEQUENCE: return "GROUPED_SEQUENCE";
        case NODE_META_IDENTIFIER: return "META_IDENTIFIER";
        case NODE_INTEGER: return "INTEGER";
        case NODE_TERMINAL_STRING: return "TERMINAL_STRING";
        case NODE_SPECIAL_SEQUENCE: return "SPECIAL_SEQUENCE";
        case NODE_EMPTY_SEQUENCE: return "EMPTY_SEQUENCE";
        case NODE_CHAR_RANGE: return "CHAR_RANGE";
        case NODE_HEX_CHAR: return "HEX_CHAR";
        case NODE_CONCATENATION: return "CONCATENATION";
        case NODE_ALTERNATIVE: return "ALTERNATIVE";
        default: return "UNKNOWN";
    }
}

void print_ast(ASTNode* node, int depth) {
    if (!node) return;
    
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    printf("%s", node_type_to_string(node->type));
    if (node->value) {
        printf(": %s", node->value);
    }
    printf("\n");
    
    for (int i = 0; i < node->children_count; i++) {
        print_ast(node->children[i], depth + 1);
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;
    
    if (node->value) {
        free(node->value);
    }
    
    for (int i = 0; i < node->children_count; i++) {
        free_ast(node->children[i]);
    }
    
    if (node->children) {
        free(node->children);
    }
    
    free(node);
}