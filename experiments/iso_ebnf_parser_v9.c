#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iso_ebnf_lexer_v6.c"

Lexer lexer;
Token current_token;

/* --- Estruturas de dados --- */
typedef struct ASTNode ASTNode;

struct ASTNode {
    char type[32];
    char value[256];
    ASTNode **children;
    int child_count;
};

typedef struct Rule {
    char name[64];
    ASTNode *definition;
} Rule;

Rule rules[256];
int rule_count = 0;

/* --- Funções utilitárias --- */
void next() {
    current_token = next_token(&lexer);
}

int check(TokenType type) {
    return current_token.type == type;
}

int accept(TokenType type) {
    if (check(type)) {
        next();
        return 1;
    }
    return 0;
}

void expect(TokenType type) {
    if (!check(type)) {
        fprintf(stderr, "Parse error at %d:%d: expected token %d, got %d ('%s')\n",
                current_token.line, current_token.col, type, 
                current_token.type, current_token.text);
        exit(1);
    }
    next();
}

ASTNode *create_node(const char *type, const char *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    strncpy(node->type, type, 31);
    if (value) strncpy(node->value, value, 255);
    else node->value[0] = '\0';
    node->children = NULL;
    node->child_count = 0;
    return node;
}

void add_child(ASTNode *parent, ASTNode *child) {
    parent->children = realloc(parent->children, 
                              (parent->child_count + 1) * sizeof(ASTNode*));
    parent->children[parent->child_count++] = child;
}

/* --- Funções de parsing --- */
ASTNode *parse_syntax();
ASTNode *parse_rule();
ASTNode *parse_expression();
ASTNode *parse_term();
ASTNode *parse_factor();
ASTNode *parse_primary();
ASTNode *parse_quantifier();
ASTNode *parse_optional();
ASTNode *parse_repeat();
ASTNode *parse_group();
ASTNode *parse_special_sequence();
ASTNode *parse_char_range();
ASTNode *parse_directive();
ASTNode *parse_literal();

/* syntax = { (rule | comment | directive | space) } ; */
ASTNode *parse_syntax() {
    ASTNode *syntax = create_node("syntax", NULL);
    
    while (!check(TOK_EOF)) {
        if (check(TOK_IDENTIFIER)) {
            add_child(syntax, parse_rule());
        } else if (check(TOK_PERCENT)) {
            add_child(syntax, parse_directive());
        } else {
            // Skip comments and spaces
            next();
        }
    }
    
    return syntax;
}

/* rule = identifier, spaces, equals, spaces, expression, spaces, semicolon ; */
ASTNode *parse_rule() {
    if (!check(TOK_IDENTIFIER)) return NULL;
    
    char rule_name[128];
    strcpy(rule_name, current_token.text);
    
    ASTNode *rule = create_node("rule", rule_name);
    next();
    
    expect(TOK_EQUALS);
    add_child(rule, parse_expression());
    expect(TOK_SEMICOLON);
    
    // Store in global rules array
    if (rule_count < 255) {
        strcpy(rules[rule_count].name, rule_name);
        rules[rule_count].definition = rule;
        rule_count++;
    }
    
    return rule;
}

/* expression = term, { spaces, pipe, spaces, term } ; */
ASTNode *parse_expression() {
    ASTNode *expr = create_node("expression", NULL);
    
    add_child(expr, parse_term());
    
    while (accept(TOK_PIPE)) {
        ASTNode *pipe = create_node("operator", "|");
        add_child(expr, pipe);
        add_child(expr, parse_term());
    }
    
    return expr;
}

/* term = factor, { spaces, comma, spaces, factor } ; */
ASTNode *parse_term() {
    ASTNode *term = create_node("term", NULL);
    
    add_child(term, parse_factor());
    
    while (accept(TOK_COMMA)) {
        ASTNode *comma = create_node("operator", ",");
        add_child(term, comma);
        add_child(term, parse_factor());
    }
    
    return term;
}

/* factor = [ integer, asterisk | quantifier ], primary ; */
ASTNode *parse_factor() {
    ASTNode *factor = create_node("factor", NULL);
    
    // Check for quantifier prefix
    if (check(TOK_INTEGER)) {
        Token integer_token = current_token;
        next();
        
        if (accept(TOK_ASTERISK)) {
            ASTNode *quant = create_node("quantifier", integer_token.text);
            add_child(factor, quant);
        } else {
            // Not a quantifier, put back the integer token
            // This is a simplification - in a real parser we'd need backtracking
            fprintf(stderr, "Warning: integer without asterisk at %d:%d\n", 
                    integer_token.line, integer_token.col);
        }
    } 
    else if (check(TOK_ASTERISK) || check(TOK_PLUS) || check(TOK_QUESTION) || check(TOK_LBRACE)) {
        add_child(factor, parse_quantifier());
    }
    
    add_child(factor, parse_primary());
    return factor;
}

/* quantifier = asterisk | plus | question | l_brace, integer, [ comma, [ integer ] ], r_brace ; */
ASTNode *parse_quantifier() {
    ASTNode *quant = create_node("quantifier", NULL);
    
    if (accept(TOK_ASTERISK)) {
        strcpy(quant->value, "*");
    } 
    else if (accept(TOK_PLUS)) {
        strcpy(quant->value, "+");
    } 
    else if (accept(TOK_QUESTION)) {
        strcpy(quant->value, "?");
    } 
    else if (accept(TOK_LBRACE)) {
        char range[64] = "{";
        
        if (check(TOK_INTEGER)) {
            strcat(range, current_token.text);
            next();
        }
        
        if (accept(TOK_COMMA)) {
            strcat(range, ",");
            if (check(TOK_INTEGER)) {
                strcat(range, current_token.text);
                next();
            }
        }
        
        strcat(range, "}");
        strcpy(quant->value, range);
        expect(TOK_RBRACE);
    }
    
    return quant;
}

/* primary = identifier | terminal | optional | repeat | group | special_sequence | char_range | hex_char ; */
ASTNode *parse_primary() {
    if (check(TOK_IDENTIFIER)) {
        ASTNode *ident = create_node("identifier", current_token.text);
        next();
        return ident;
    } 
    else if (check(TOK_STRING)) {
        ASTNode *terminal = create_node("terminal", current_token.text);
        next();
        return terminal;
    } 
    else if (check(TOK_HEX_CHAR)) {
        ASTNode *hex = create_node("hex_char", current_token.text);
        next();
        return hex;
    } 
    else if (check(TOK_LBRACKET)) {
        return parse_optional();
    } 
    else if (check(TOK_LBRACE)) {
        return parse_repeat();
    } 
    else if (check(TOK_LPAREN)) {
        return parse_group();
    } 
    else if (check(TOK_QUESTION)) {
        return parse_special_sequence();
    } 
    else if (check(TOK_HASH)) {
        return parse_char_range();
    } 
    else {
        fprintf(stderr, "Parse error at %d:%d: unexpected token '%s' in primary\n",
                current_token.line, current_token.col, current_token.text);
        exit(1);
    }
}

/* optional = l_bracket, expression, r_bracket ; */
ASTNode *parse_optional() {
    ASTNode *optional = create_node("optional", NULL);
    expect(TOK_LBRACKET);
    add_child(optional, parse_expression());
    expect(TOK_RBRACKET);
    return optional;
}

/* repeat = l_brace, expression, r_brace ; */
ASTNode *parse_repeat() {
    ASTNode *repeat = create_node("repeat", NULL);
    expect(TOK_LBRACE);
    add_child(repeat, parse_expression());
    expect(TOK_RBRACE);
    return repeat;
}

/* group = l_paren, expression, r_paren ; */
ASTNode *parse_group() {
    ASTNode *group = create_node("group", NULL);
    expect(TOK_LPAREN);
    add_child(group, parse_expression());
    expect(TOK_RPAREN);
    return group;
}

/* special_sequence = question, { character_set }, question ; */
ASTNode *parse_special_sequence() {
    ASTNode *seq = create_node("special_sequence", NULL);
    expect(TOK_QUESTION);
    
    // Collect content until closing question mark
    while (!check(TOK_QUESTION) && !check(TOK_EOF)) {
        ASTNode *char_node = create_node("char", current_token.text);
        add_child(seq, char_node);
        next();
    }
    
    expect(TOK_QUESTION);
    return seq;
}

/* char_range = hex_char, spaces, dot_dot, spaces, hex_char ; */
ASTNode *parse_char_range() {
    ASTNode *range = create_node("char_range", NULL);
    
    if (check(TOK_HEX_CHAR)) {
        ASTNode *start = create_node("hex_char", current_token.text);
        add_child(range, start);
        next();
    }
    
    if (accept(TOK_DOTDOT)) {
        ASTNode *dots = create_node("operator", "..");
        add_child(range, dots);
    }
    
    if (check(TOK_HEX_CHAR)) {
        ASTNode *end = create_node("hex_char", current_token.text);
        add_child(range, end);
        next();
    }
    
    return range;
}

/* directive = percent, identifier, [ l_paren, literal, r_paren ], percent ; */
ASTNode *parse_directive() {
    ASTNode *directive = create_node("directive", NULL);
    expect(TOK_PERCENT);
    
    if (check(TOK_IDENTIFIER)) {
        ASTNode *ident = create_node("identifier", current_token.text);
        add_child(directive, ident);
        next();
    }
    
    if (accept(TOK_LPAREN)) {
        add_child(directive, parse_literal());
        expect(TOK_RPAREN);
    }
    
    expect(TOK_PERCENT);
    return directive;
}

/* literal = string | identifier | integer | hex_char ; */
ASTNode *parse_literal() {
    if (check(TOK_STRING) || check(TOK_IDENTIFIER) || 
        check(TOK_INTEGER) || check(TOK_HEX_CHAR)) {
        ASTNode *lit = create_node("literal", current_token.text);
        next();
        return lit;
    } else {
        fprintf(stderr, "Parse error at %d:%d: expected literal, got '%s'\n",
                current_token.line, current_token.col, current_token.text);
        exit(1);
    }
}

/* --- Função para imprimir JSON --- */
void print_json_ast(ASTNode *node, int depth) {
    for (int i = 0; i < depth * 2; i++) printf(" ");
    
    if (node->child_count == 0) {
        printf("\"%s\": \"%s\"", node->type, node->value);
    } else {
        printf("\"%s\": {", node->type);
        if (node->value[0] != '\0') {
            printf("\"value\": \"%s\", ", node->value);
        }
        printf("\"children\": [\n");
        
        for (int i = 0; i < node->child_count; i++) {
            print_json_ast(node->children[i], depth + 1);
            if (i < node->child_count - 1) printf(",\n");
        }
        
        printf("\n");
        for (int i = 0; i < depth * 2; i++) printf(" ");
        printf("]}");
    }
}

void print_json() {
    printf("{\n  \"ast\": ");
    ASTNode *syntax = parse_syntax();
    print_json_ast(syntax, 1);
    printf("\n}\n");
}

/* --- Main --- */
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s file.ebnf\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *input = malloc(len + 1);
    fread(input, 1, len, f);
    input[len] = '\0';
    fclose(f);
    
    // Initialize lexer
    lexer.input = input;
    lexer.pos = 0;
    lexer.line = 1;
    lexer.col = 1;
    
    // Get first token
    next();
    
    // Parse and print JSON
    print_json();
    
    free(input);
    return 0;
}