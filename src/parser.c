#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/*
 * ==========================================
 * ISO EBNF Parser in C (Complete Version)
 * ==========================================
 *
 * This program implements a lexer, parser, and AST generator
 * for ISO EBNF grammars. It reads an input file containing
 * EBNF definitions and prints a structured AST representation.
 *
 * Features:
 * - Supports identifiers, strings, integers, hex chars
 * - Recognizes comments, directives, special sequences
 * - Supports EBNF operators: '=', ';', ',', '|', '*', '+', '?', '-', '..'
 * - Handles optional, repeat, group, and quantifiers
 * - Provides detailed error messages with line/column
 * - Clean memory management
 *
 * Author: Generated for user
 * Language: C (C99 compatible)
 */

///////////////////////////
// LEXER DEFINITIONS
///////////////////////////

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_HEX_CHAR,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_PIPE,
    TOKEN_COMMA,
    TOKEN_ASTERISK,
    TOKEN_PLUS,
    TOKEN_QUESTION,
    TOKEN_MINUS,
    TOKEN_L_BRACKET,
    TOKEN_R_BRACKET,
    TOKEN_L_BRACE,
    TOKEN_R_BRACE,
    TOKEN_L_PAREN,
    TOKEN_R_PAREN,
    TOKEN_DOT_DOT,
    TOKEN_HASH,
    TOKEN_PERCENT,
    TOKEN_COMMENT,
    TOKEN_DIRECTIVE,
    TOKEN_SPECIAL_SEQUENCE,
    TOKEN_CHAR_RANGE,
    TOKEN_WHITESPACE,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

typedef struct {
    const char* input;
    int position;
    int line;
    int column;
    Token current_token;
} Lexer;

///////////////////////////
// AST DEFINITIONS
///////////////////////////

typedef enum {
    NODE_SYNTAX,
    NODE_RULE,
    NODE_EXPRESSION,
    NODE_TERM,
    NODE_FACTOR,
    NODE_PRIMARY,
    NODE_IDENTIFIER,
    NODE_TERMINAL,
    NODE_INTEGER,
    NODE_HEX_CHAR,
    NODE_OPTIONAL,
    NODE_REPEAT,
    NODE_GROUP,
    NODE_SPECIAL_SEQUENCE,
    NODE_CHAR_RANGE,
    NODE_DIRECTIVE,
    NODE_QUANTIFIER
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* value;
    struct ASTNode** children;
    int children_count;
    int line;
    int column;
} ASTNode;

///////////////////////////
// LEXER IMPLEMENTATION
///////////////////////////

char lexer_peek(Lexer* lexer) {
    return lexer->input[lexer->position];
}

char lexer_peek_next(Lexer* lexer, int offset) {
    return lexer->input[lexer->position + offset];
}

char lexer_advance(Lexer* lexer) {
    char c = lexer_peek(lexer);
    if (c != '\0') {
        lexer->position++;
        lexer->column++;
        if (c == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
    }
    return c;
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (isspace((unsigned char)lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
}

bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_hex_digit(char c) {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

char* lexer_read_identifier(Lexer* lexer) {
    int start = lexer->position;
    if (is_letter(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
        lexer_advance(lexer);
        while (is_letter(lexer_peek(lexer)) || is_digit(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
            lexer_advance(lexer);
        }
    }
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_integer(Lexer* lexer) {
    int start = lexer->position;
    while (is_digit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_string(Lexer* lexer, char quote) {
    int start = lexer->position;
    lexer_advance(lexer); // Skip opening quote
    while (lexer_peek(lexer) != quote && lexer_peek(lexer) != '\0') {
        if (lexer_peek(lexer) == '\\') {
            lexer_advance(lexer);
            if (lexer_peek(lexer) != '\0') lexer_advance(lexer);
        } else {
            lexer_advance(lexer);
        }
    }
    if (lexer_peek(lexer) == quote) lexer_advance(lexer);
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_hex_char(Lexer* lexer) {
    int start = lexer->position;
    if (lexer_peek(lexer) == '#') {
        lexer_advance(lexer);
        while (is_hex_digit(lexer_peek(lexer))) lexer_advance(lexer);
    }
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_comment_block(Lexer* lexer) {
    int start = lexer->position;
    lexer_advance(lexer); // Skip '('
    lexer_advance(lexer); // Skip '*'
    int depth = 1;
    while (depth > 0 && lexer_peek(lexer) != '\0') {
        if (lexer_peek(lexer) == '(' && lexer_peek_next(lexer,1) == '*') { depth++; lexer_advance(lexer); lexer_advance(lexer);}
        else if (lexer_peek(lexer) == '*' && lexer_peek_next(lexer,1) == ')') { depth--; lexer_advance(lexer); lexer_advance(lexer);}
        else lexer_advance(lexer);
    }
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_comment_line(Lexer* lexer) {
    int start = lexer->position;
    lexer_advance(lexer); // Skip '/'
    lexer_advance(lexer); // Skip '/'
    while (lexer_peek(lexer) != '\n' && lexer_peek(lexer) != '\r' && lexer_peek(lexer) != '\0') lexer_advance(lexer);
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

Lexer* lexer_create(const char* input) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_token.type = TOKEN_EOF;
    lexer->current_token.value = NULL;
    return lexer;
}

void lexer_destroy(Lexer* lexer) {
    if (lexer->current_token.value) free(lexer->current_token.value);
    free(lexer);
}

Token lexer_next_token(Lexer* lexer) {
    if (lexer->current_token.value) {
        free(lexer->current_token.value);
    }
    
    lexer_skip_whitespace(lexer);
    
    int line = lexer->line;
    int column = lexer->column;
    char current = lexer_peek(lexer);
    
    if (current == '\0') {
        lexer->current_token.type = TOKEN_EOF;
        lexer->current_token.value = strdup("");
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    // Check for multi-character tokens first
    if (current == '(' && lexer_peek_next(lexer, 1) == '*') {
        char* value = lexer_read_comment_block(lexer);
        lexer->current_token.type = TOKEN_COMMENT;
        lexer->current_token.value = value;
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    if (current == '/' && lexer_peek_next(lexer, 1) == '/') {
        char* value = lexer_read_comment_line(lexer);
        lexer->current_token.type = TOKEN_COMMENT;
        lexer->current_token.value = value;
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    if (current == '.' && lexer_peek_next(lexer, 1) == '.') {
        lexer_advance(lexer);
        lexer_advance(lexer);
        lexer->current_token.type = TOKEN_DOT_DOT;
        lexer->current_token.value = strdup("..");
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    // Check for hex char (# followed by hex digits)
    if (current == '#' && is_hex_digit(lexer_peek_next(lexer, 1))) {
        char* value = lexer_read_hex_char(lexer);
        lexer->current_token.type = TOKEN_HEX_CHAR;
        lexer->current_token.value = value;
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    // Single character tokens
    switch (current) {
        case '=':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_EQUALS;
            lexer->current_token.value = strdup("=");
            break;
        case ';':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_SEMICOLON;
            lexer->current_token.value = strdup(";");
            break;
        case '|':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_PIPE;
            lexer->current_token.value = strdup("|");
            break;
        case ',':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_COMMA;
            lexer->current_token.value = strdup(",");
            break;
        case '*':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_ASTERISK;
            lexer->current_token.value = strdup("*");
            break;
        case '+':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_PLUS;
            lexer->current_token.value = strdup("+");
            break;
        case '?':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_QUESTION;
            lexer->current_token.value = strdup("?");
            break;
        case '-':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_MINUS;
            lexer->current_token.value = strdup("-");
            break;
        case '[':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_L_BRACKET;
            lexer->current_token.value = strdup("[");
            break;
        case ']':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_R_BRACKET;
            lexer->current_token.value = strdup("]");
            break;
        case '{':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_L_BRACE;
            lexer->current_token.value = strdup("{");
            break;
        case '}':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_R_BRACE;
            lexer->current_token.value = strdup("}");
            break;
        case '(':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_L_PAREN;
            lexer->current_token.value = strdup("(");
            break;
        case ')':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_R_PAREN;
            lexer->current_token.value = strdup(")");
            break;
        case '#':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_HASH;
            lexer->current_token.value = strdup("#");
            break;
        case '%':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_PERCENT;
            lexer->current_token.value = strdup("%");
            break;
        case '"':
        case '\'':
            lexer->current_token.type = TOKEN_STRING;
            lexer->current_token.value = lexer_read_string(lexer, current);
            break;
        default:
            if (is_digit(current)) {
                lexer->current_token.type = TOKEN_INTEGER;
                lexer->current_token.value = lexer_read_integer(lexer);
            } else if (is_letter(current) || current == '_') {
                lexer->current_token.type = TOKEN_IDENTIFIER;
                lexer->current_token.value = lexer_read_identifier(lexer);
            } else {
                // Unknown character
                char* value = malloc(2);
                value[0] = current;
                value[1] = '\0';
                lexer_advance(lexer);
                lexer->current_token.type = TOKEN_ERROR;
                lexer->current_token.value = value;
            }
            break;
    }
    
    lexer->current_token.line = line;
    lexer->current_token.column = column;
    return lexer->current_token;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_HEX_CHAR: return "HEX_CHAR";
        case TOKEN_EQUALS: return "EQUALS";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_PIPE: return "PIPE";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_ASTERISK: return "ASTERISK";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_QUESTION: return "QUESTION";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_L_BRACKET: return "L_BRACKET";
        case TOKEN_R_BRACKET: return "R_BRACKET";
        case TOKEN_L_BRACE: return "L_BRACE";
        case TOKEN_R_BRACE: return "R_BRACE";
        case TOKEN_L_PAREN: return "L_PAREN";
        case TOKEN_R_PAREN: return "R_PAREN";
        case TOKEN_DOT_DOT: return "DOT_DOT";
        case TOKEN_HASH: return "HASH";
        case TOKEN_PERCENT: return "PERCENT";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_DIRECTIVE: return "DIRECTIVE";
        case TOKEN_SPECIAL_SEQUENCE: return "SPECIAL_SEQUENCE";
        case TOKEN_CHAR_RANGE: return "CHAR_RANGE";
        case TOKEN_WHITESPACE: return "WHITESPACE";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static char lexer_peek_nonspace(Lexer* lexer) {
    int pos = lexer->position;
    while (lexer->input[pos] != '\0' && isspace((unsigned char)lexer->input[pos])) pos++;
    return lexer->input[pos];
}

///////////////////////////
// AST IMPLEMENTATION
///////////////////////////

ASTNode* create_ast_node(NodeType type, const char* value, int line, int column) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->children = NULL;
    node->children_count = 0;
    node->line = line;
    node->column = column;
    return node;
}

void add_child_to_node(ASTNode* parent, ASTNode* child) {
    parent->children = realloc(parent->children, sizeof(ASTNode*)*(parent->children_count+1));
    parent->children[parent->children_count++] = child;
}

void free_ast_node(ASTNode* node) {
    if (!node) return;
    if (node->value) free(node->value);
    for (int i=0;i<node->children_count;i++) free_ast_node(node->children[i]);
    if (node->children) free(node->children);
    free(node);
}

void print_ast(ASTNode* node, int depth) {
    if (!node) return;
    for(int i=0;i<depth;i++) printf("  ");
    const char* type_names[] = {
        "SYNTAX", "RULE", "EXPRESSION", "TERM", "FACTOR", "PRIMARY",
        "IDENTIFIER","TERMINAL","INTEGER","HEX_CHAR","OPTIONAL",
        "REPEAT","GROUP","SPECIAL_SEQUENCE","CHAR_RANGE","DIRECTIVE","QUANTIFIER"
    };
    printf("%s", type_names[node->type]);
    if (node->value) printf(": '%s'", node->value);
    printf(" (line %d,column %d)\n", node->line,node->column);
    for(int i=0;i<node->children_count;i++) print_ast(node->children[i], depth+1);
}

///////////////////////////
// PARSER DEFINITIONS
///////////////////////////

typedef struct {
    Lexer* lexer;
    Token current_token;
} Parser;

// Forward declarations
ASTNode* parse_syntax(Parser* parser);
ASTNode* parse_rule(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_term(Parser* parser);
ASTNode* parse_factor(Parser* parser);
ASTNode* parse_primary(Parser* parser);
ASTNode* parse_optional(Parser* parser);
ASTNode* parse_repeat(Parser* parser);
ASTNode* parse_group(Parser* parser);
ASTNode* parse_special_sequence(Parser* parser);
ASTNode* parse_directive(Parser* parser);
ASTNode* parse_quantifier(Parser* parser);

///////////////////////////
// PARSER IMPLEMENTATION
///////////////////////////

Parser* parser_create(Lexer* lexer) {
    Parser* parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = lexer_next_token(lexer);
    return parser;
}

void parser_destroy(Parser* parser) { free(parser); }

void parser_advance(Parser* parser) {
    parser->current_token = lexer_next_token(parser->lexer);
}

bool parser_expect(Parser* parser, TokenType expected_type, const char* error_msg) {
    if (parser->current_token.type == expected_type) return true;
    if (error_msg) {
        printf("Error: %s. Expected %s, got %s at line %d, column %d\n",
               error_msg,
               token_type_to_string(expected_type),  // Usar nomes, não números
               token_type_to_string(parser->current_token.type),
               parser->current_token.line,
               parser->current_token.column);
    }
    return false;
}

ASTNode* parse_syntax(Parser* parser) {
    ASTNode* syntax_node = create_ast_node(NODE_SYNTAX, NULL, 1, 1);
    
    while (parser->current_token.type != TOKEN_EOF) {
        switch (parser->current_token.type) {
            case TOKEN_IDENTIFIER:
                add_child_to_node(syntax_node, parse_rule(parser));
                break;
            case TOKEN_PERCENT:
                add_child_to_node(syntax_node, parse_directive(parser));
                break;
            case TOKEN_COMMENT:
                // Skip comments for now
                parser_advance(parser);
                break;
            default:
                printf("Unexpected token in syntax: %s\n", 
                       token_type_to_string(parser->current_token.type));
                parser_advance(parser);
                break;
        }
    }
    
    return syntax_node;
}

ASTNode* parse_rule(Parser* parser) {
    if (!parser_expect(parser, TOKEN_IDENTIFIER, "Expected identifier for rule")) {
        return NULL;
    }
    
    ASTNode* rule_node = create_ast_node(NODE_RULE, NULL, 
                                       parser->current_token.line, 
                                       parser->current_token.column);
    
    // Get identifier value before advancing
    char* identifier_value = strdup(parser->current_token.value);
    ASTNode* identifier_node = create_ast_node(NODE_IDENTIFIER, 
                                             identifier_value,
                                             parser->current_token.line,
                                             parser->current_token.column);
    add_child_to_node(rule_node, identifier_node);
    parser_advance(parser);
    
    if (!parser_expect(parser, TOKEN_EQUALS, "Expected '=' after identifier")) {
        free_ast_node(rule_node);
        return NULL;
    }
    parser_advance(parser);
    
    ASTNode* expression_node = parse_expression(parser);
    if (!expression_node) {
        free_ast_node(rule_node);
        return NULL;
    }
    add_child_to_node(rule_node, expression_node);
    
    if (!parser_expect(parser, TOKEN_SEMICOLON, "Expected ';' after expression")) {
        free_ast_node(rule_node);
        return NULL;
    }
    parser_advance(parser);
    
    return rule_node;
}

ASTNode* parse_expression(Parser* parser) {
    ASTNode* expression_node = create_ast_node(NODE_EXPRESSION, NULL,
                                             parser->current_token.line,
                                             parser->current_token.column);
    
    // Parse first term
    ASTNode* first_term = parse_term(parser);
    if (!first_term) {
        free_ast_node(expression_node);
        return NULL;
    }
    add_child_to_node(expression_node, first_term);
    
    // Handle pipe-separated terms (alternatives)
    while (parser->current_token.type == TOKEN_PIPE) {
        parser_advance(parser); // Skip the pipe
        
        ASTNode* next_term = parse_term(parser);
        if (!next_term) {
            free_ast_node(expression_node);
            return NULL;
        }
        add_child_to_node(expression_node, next_term);
    }
    
    return expression_node;
}

ASTNode* parse_term(Parser* parser) {
    ASTNode* term_node = create_ast_node(NODE_TERM, NULL,
                                       parser->current_token.line,
                                       parser->current_token.column);
    
    // Parse first factor
    ASTNode* first_factor = parse_factor(parser);
    if (!first_factor) {
        free_ast_node(term_node);
        return NULL;
    }
    add_child_to_node(term_node, first_factor);
    
    // Handle comma-separated factors
    while (parser->current_token.type == TOKEN_COMMA) {
        parser_advance(parser); // Skip the comma
        
        ASTNode* next_factor = parse_factor(parser);
        if (!next_factor) {
            free_ast_node(term_node);
            return NULL;
        }
        add_child_to_node(term_node, next_factor);
    }
    
    return term_node;
}

ASTNode* parse_factor(Parser* parser) {
    ASTNode* factor_node = create_ast_node(NODE_FACTOR, NULL,
                                           parser->current_token.line,
                                           parser->current_token.column);

    // 1) Detects PREFIX quantifier of type: integer '*' OR quantifier ( '*', '+', '?', '{n[,m]}' )
    if (parser->current_token.type == TOKEN_INTEGER) {
        // If it is an integer followed (in the input, skipping spaces) by '*' => treat as a prefix quantifier "N*"
        char nextch = lexer_peek_nonspace(parser->lexer);
        if (nextch == '*') {
            char* int_val = strdup(parser->current_token.value);
            // consumes the integer
            parser_advance(parser); // should now be in ASTERISK
            if (parser->current_token.type == TOKEN_ASTERISK) {
                ASTNode* qnode = create_ast_node(NODE_QUANTIFIER, NULL,
                                                 parser->current_token.line,
                                                 parser->current_token.column);
                add_child_to_node(qnode, create_ast_node(NODE_INTEGER, int_val,
                                                         parser->current_token.line,
                                                         parser->current_token.column));
                add_child_to_node(qnode, create_ast_node(NODE_SPECIAL_SEQUENCE, "*",
                                                         parser->current_token.line,
                                                         parser->current_token.column));
                parser_advance(parser); // consumes '*'
                add_child_to_node(factor_node, qnode);
            } else {
                // fallback (unlikely): treat integer as primary (we don't consume '*')
                add_child_to_node(factor_node, create_ast_node(NODE_INTEGER, int_val,
                                                               parser->current_token.line,
                                                               parser->current_token.column));
            }
            free(int_val);
        }
        // else: leave the INTEGER to be consumed by parse_primary (we don't move forward)
    } else if (parser->current_token.type == TOKEN_ASTERISK ||
               parser->current_token.type == TOKEN_PLUS ||
               parser->current_token.type == TOKEN_QUESTION ||
               parser->current_token.type == TOKEN_L_BRACE) {

        // Se token atual é '{' precisamos decidir: quantificador {n[,m]} ou primary repeat { ... } ?
        if (parser->current_token.type == TOKEN_L_BRACE) {
            // look at the next non-space character: if it is a digit, treat it as a quantifier; otherwise, it is primary (repeat)
            char nextch = lexer_peek_nonspace(parser->lexer);
            if (is_digit((unsigned char)nextch)) {
                ASTNode* qnode = parse_quantifier(parser);
                if (!qnode) { free_ast_node(factor_node); return NULL; }
                add_child_to_node(factor_node, qnode);
            }
            // otherwise, do not consume here — parse_primary will handle repeat
        } else {
            ASTNode* qnode = parse_quantifier(parser);
            if (!qnode) { free_ast_node(factor_node); return NULL; }
            add_child_to_node(factor_node, qnode);
        }
    }

    // 2) Now parse the primary (required)
    ASTNode* primary_node = parse_primary(parser);
    if (!primary_node) {
        free_ast_node(factor_node);
        return NULL;
    }
    add_child_to_node(factor_node, primary_node);

    return factor_node;
}

ASTNode* parse_quantifier(Parser* parser) {
    ASTNode* quantifier_node = create_ast_node(NODE_QUANTIFIER, NULL,
                                             parser->current_token.line,
                                             parser->current_token.column);
    
    switch (parser->current_token.type) {
        case TOKEN_ASTERISK:
            add_child_to_node(quantifier_node, 
                create_ast_node(NODE_SPECIAL_SEQUENCE, "*", 
                              parser->current_token.line, 
                              parser->current_token.column));
            parser_advance(parser);
            break;
        case TOKEN_PLUS:
            add_child_to_node(quantifier_node, 
                create_ast_node(NODE_SPECIAL_SEQUENCE, "+", 
                              parser->current_token.line, 
                              parser->current_token.column));
            parser_advance(parser);
            break;
        case TOKEN_QUESTION:
            add_child_to_node(quantifier_node, 
                create_ast_node(NODE_SPECIAL_SEQUENCE, "?", 
                              parser->current_token.line, 
                              parser->current_token.column));
            parser_advance(parser);
            break;
        case TOKEN_L_BRACE:
            // Simplified quantifier parsing
            parser_advance(parser);
            if (parser->current_token.type == TOKEN_INTEGER) {
                add_child_to_node(quantifier_node, 
                    create_ast_node(NODE_INTEGER, parser->current_token.value,
                                  parser->current_token.line,
                                  parser->current_token.column));
                parser_advance(parser);
            }
            if (parser->current_token.type == TOKEN_COMMA) {
                parser_advance(parser);
                if (parser->current_token.type == TOKEN_INTEGER) {
                    add_child_to_node(quantifier_node, 
                        create_ast_node(NODE_INTEGER, parser->current_token.value,
                                      parser->current_token.line,
                                      parser->current_token.column));
                    parser_advance(parser);
                }
            }
            if (!parser_expect(parser, TOKEN_R_BRACE, "Expected '}'")) {
                free_ast_node(quantifier_node);
                return NULL;
            }
            parser_advance(parser);
            break;
        default:
            free_ast_node(quantifier_node);
            return NULL;
    }
    
    return quantifier_node;
}

ASTNode* parse_primary(Parser* parser) {
    int line = parser->current_token.line;
    int column = parser->current_token.column;

    switch (parser->current_token.type) {
        case TOKEN_IDENTIFIER: {
            char* value = strdup(parser->current_token.value);
            parser_advance(parser);
            return create_ast_node(NODE_IDENTIFIER, value, line, column);
        }
        case TOKEN_STRING: {
            char* value = strdup(parser->current_token.value);
            parser_advance(parser);
            return create_ast_node(NODE_TERMINAL, value, line, column);
        }
        case TOKEN_INTEGER: {
            char* value = strdup(parser->current_token.value);
            parser_advance(parser);
            return create_ast_node(NODE_INTEGER, value, line, column);
        }
        case TOKEN_HEX_CHAR: {
            char* start_val = strdup(parser->current_token.value);
            parser_advance(parser);
            
            // Check if it is a range (hex_char .. hex_char)
            if (parser->current_token.type == TOKEN_DOT_DOT) {
                parser_advance(parser);
                if (parser->current_token.type == TOKEN_HEX_CHAR) {
                    char* end_val = strdup(parser->current_token.value);
                    ASTNode* range_node = create_ast_node(NODE_CHAR_RANGE, NULL, line, column);
                    add_child_to_node(range_node, create_ast_node(NODE_HEX_CHAR, start_val, line, column));
                    add_child_to_node(range_node, create_ast_node(NODE_HEX_CHAR, end_val, 
                                                                parser->current_token.line,
                                                                parser->current_token.column));
                    parser_advance(parser);
                    free(start_val);
                    free(end_val);
                    return range_node;
                }
            }
            // If not range, returns simple HEX_CHAR
            ASTNode* hexnode = create_ast_node(NODE_HEX_CHAR, start_val, line, column);
            free(start_val);
            return hexnode;
        }
        case TOKEN_L_BRACKET:
            return parse_optional(parser);
        case TOKEN_L_BRACE:
            return parse_repeat(parser);
        case TOKEN_L_PAREN:
            return parse_group(parser);
        case TOKEN_QUESTION:
            return parse_special_sequence(parser);
        default:
            // safely delete null value when printing if necessary
            printf("Unexpected token in primary: %s (%s)\n",
                   token_type_to_string(parser->current_token.type),
                   parser->current_token.value ? parser->current_token.value : "");
            return NULL;
    }
}

ASTNode* parse_optional(Parser* parser) {
    if (!parser_expect(parser, TOKEN_L_BRACKET, "Expected '['")) return NULL;
    
    ASTNode* optional_node = create_ast_node(NODE_OPTIONAL, NULL,
                                           parser->current_token.line,
                                           parser->current_token.column);
    parser_advance(parser);
    
    ASTNode* expression_node = parse_expression(parser);
    if (!expression_node) {
        free_ast_node(optional_node);
        return NULL;
    }
    add_child_to_node(optional_node, expression_node);
    
    if (!parser_expect(parser, TOKEN_R_BRACKET, "Expected ']'")) {
        free_ast_node(optional_node);
        return NULL;
    }
    parser_advance(parser);
    
    return optional_node;
}

ASTNode* parse_repeat(Parser* parser) {
    if (!parser_expect(parser, TOKEN_L_BRACE, "Expected '{'")) return NULL;
    
    ASTNode* repeat_node = create_ast_node(NODE_REPEAT, NULL,
                                         parser->current_token.line,
                                         parser->current_token.column);
    parser_advance(parser); // Skip '{'
    
    // Parse the expression inside the braces
    ASTNode* expression_node = parse_expression(parser);
    if (!expression_node) {
        free_ast_node(repeat_node);
        return NULL;
    }
    add_child_to_node(repeat_node, expression_node);
    
    if (!parser_expect(parser, TOKEN_R_BRACE, "Expected '}'")) {
        free_ast_node(repeat_node);
        return NULL;
    }
    parser_advance(parser); // Skip '}'
    
    return repeat_node;
}

ASTNode* parse_group(Parser* parser) {
    if (!parser_expect(parser, TOKEN_L_PAREN, "Expected '('")) return NULL;
    
    ASTNode* group_node = create_ast_node(NODE_GROUP, NULL,
                                        parser->current_token.line,
                                        parser->current_token.column);
    parser_advance(parser);
    
    ASTNode* expression_node = parse_expression(parser);
    if (!expression_node) {
        free_ast_node(group_node);
        return NULL;
    }
    add_child_to_node(group_node, expression_node);
    
    if (!parser_expect(parser, TOKEN_R_PAREN, "Expected ')'")) {
        free_ast_node(group_node);
        return NULL;
    }
    parser_advance(parser);
    
    return group_node;
}

ASTNode* parse_special_sequence(Parser* parser) {
    if (!parser_expect(parser, TOKEN_QUESTION, "Expected '?'")) return NULL;

    ASTNode* seq_node = create_ast_node(NODE_SPECIAL_SEQUENCE, NULL,
                                      parser->current_token.line,
                                      parser->current_token.column);
    parser_advance(parser);

    // Parse todo o conteúdo até o próximo '?'
    while (parser->current_token.type != TOKEN_QUESTION && 
           parser->current_token.type != TOKEN_EOF) {
        
        // Adicionar qualquer token como parte da sequência
        add_child_to_node(seq_node,
            create_ast_node(NODE_SPECIAL_SEQUENCE, parser->current_token.value,
                          parser->current_token.line,
                          parser->current_token.column));
        parser_advance(parser);
    }

    if (!parser_expect(parser, TOKEN_QUESTION, "Expected closing '?'")) {
        free_ast_node(seq_node);
        return NULL;
    }
    parser_advance(parser);

    return seq_node;
}

ASTNode* parse_directive(Parser* parser) {
    if (!parser_expect(parser, TOKEN_PERCENT, "Expected '%'")) return NULL;

    ASTNode* directive_node = create_ast_node(NODE_DIRECTIVE, NULL,
                                            parser->current_token.line,
                                            parser->current_token.column);
    parser_advance(parser);

    // Parse identifier after %
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        add_child_to_node(directive_node,
            create_ast_node(NODE_IDENTIFIER, parser->current_token.value,
                          parser->current_token.line,
                          parser->current_token.column));
        parser_advance(parser);
    }

    // Parse optional content until next %
    while (parser->current_token.type != TOKEN_PERCENT && 
           parser->current_token.type != TOKEN_EOF) {
        // Add any content as special sequence
        add_child_to_node(directive_node,
            create_ast_node(NODE_SPECIAL_SEQUENCE, parser->current_token.value,
                          parser->current_token.line,
                          parser->current_token.column));
        parser_advance(parser);
    }

    if (!parser_expect(parser, TOKEN_PERCENT, "Expected '%' at end of directive")) {
        free_ast_node(directive_node);
        return NULL;
    }
    parser_advance(parser);

    return directive_node;
}

// ==================== DEBUG FUNCTIONS ====================
void print_current_token(Parser* parser) {
    printf("Current token: %s '%s' at line %d, column %d\n",
           token_type_to_string(parser->current_token.type),
           parser->current_token.value,
           parser->current_token.line,
           parser->current_token.column);
}

///////////////////////////
// MAIN FUNCTION
///////////////////////////

int main(int argc, char* argv[]) {
    if (argc < 2) { fprintf(stderr,"Usage: %s <filename>\n", argv[0]); return 1; }
    FILE* fp = fopen(argv[1],"rb");
    if(!fp){ perror("fopen"); return 1; }
    fseek(fp,0,SEEK_END);
    long filesize = ftell(fp);
    fseek(fp,0,SEEK_SET);
    char* buffer = malloc(filesize+1);
    if(!buffer){ perror("malloc"); fclose(fp); return 1; }
    fread(buffer,1,filesize,fp);
    buffer[filesize]='\0';
    fclose(fp);

    Lexer* lexer = lexer_create(buffer);
    Parser* parser = parser_create(lexer);

    ASTNode* syntax_tree = parse_syntax(parser);
    if(syntax_tree){ print_ast(syntax_tree,0); free_ast_node(syntax_tree);}
    else printf("Parsing failed.\n");

    parser_destroy(parser);
    lexer_destroy(lexer);
    free(buffer);
    return 0;
}
