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
        case NODE_RULE: return "RULE";
        case NODE_EXPRESSION: return "EXPRESSION";
        case NODE_TERM: return "TERM";
        case NODE_FACTOR: return "FACTOR";
        case NODE_PRIMARY: return "PRIMARY";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_TERMINAL: return "TERMINAL";
        case NODE_OPTIONAL: return "OPTIONAL";
        case NODE_REPEATED: return "REPEATED";
        case NODE_GROUP: return "GROUP";
        case NODE_SPECIAL_SEQUENCE: return "SPECIAL_SEQUENCE";
        case NODE_CHAR_RANGE: return "CHAR_RANGE";
        case NODE_DIRECTIVE: return "DIRECTIVE";
        case NODE_REPETITION: return "REPETITION";
        case NODE_INTEGER: return "INTEGER";
        case NODE_HEX_CHAR: return "HEX_CHAR";
        case NODE_STRING: return "STRING";
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