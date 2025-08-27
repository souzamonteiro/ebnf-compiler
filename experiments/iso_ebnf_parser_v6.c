#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iso_ebnf_lexer_v6.c"  // seu lexer

Lexer lexer;
Token current_token;

/* Avança para o próximo token */
void next() {
    current_token = next_token(&lexer);
}

/* Verifica se o token atual é do tipo esperado */
int check(TokenType type) {
    return current_token.type == type;
}

/* Espera o token atual ser do tipo type, avança, ou termina com erro */
void expect(TokenType type) {
    if (check(type)) next();
    else {
        fprintf(stderr, "Parse error at %d:%d: expected token %d, got %d ('%s')\n",
            current_token.line, current_token.col, type, current_token.type, current_token.text);
        exit(1);
    }
}

/* --- Estruturas para JSON --- */
typedef struct Rule {
    char name[64];
    char *productions[128];
    int prod_count;
} Rule;

Rule rules[256];
int rule_count = 0;

/* Adiciona uma produção à regra */
void add_production(const char *rulename, const char *prod) {
    int i;
    for (i = 0; i < rule_count; i++) {
        if (strcmp(rules[i].name, rulename) == 0) {
            rules[i].productions[rules[i].prod_count++] = strdup(prod);
            return;
        }
    }
    strcpy(rules[rule_count].name, rulename);
    rules[rule_count].productions[0] = strdup(prod);
    rules[rule_count].prod_count = 1;
    rule_count++;
}

/* --- Parsers recursivos --- */
void parse_syntax();
void parse_rule();
void parse_expression();
void parse_term();
void parse_factor();
void parse_primary();
void parse_optional();
void parse_repeat();
void parse_group();
void parse_special_sequence();
void parse_char_range();
void parse_directive();

/* syntax = { rule | comment | directive } ; */
void parse_syntax() {
    while (!check(TOK_EOF)) {
        if (check(TOK_IDENTIFIER)) parse_rule();
        else if (check(TOK_PERCENT)) parse_directive();
        else {
            // Ignora comentários e desconhecidos
            next();
        }
    }
}

/* rule = identifier, spaces, equals, spaces, expression, spaces, semicolon ; */
void parse_rule() {
    char name[128];
    if (!check(TOK_IDENTIFIER)) return;
    strcpy(name, current_token.text);
    next();
    expect(TOK_EQUALS);
    parse_expression();
    expect(TOK_SEMICOLON);
    add_production(name, "expression"); // simplificado para JSON
}

/* expression = term, { pipe, term } ; */
void parse_expression() {
    parse_term();
    while (check(TOK_PIPE)) {
        next();
        parse_term();
    }
}

/* term = factor, { comma, factor } ; */
void parse_term() {
    parse_factor();
    while (check(TOK_COMMA)) {
        next();
        parse_factor();
    }
}

/* factor = [ integer, asterisk | quantifier ], primary ; */
void parse_factor() {
    if (check(TOK_INTEGER) || check(TOK_ASTERISK) || check(TOK_PLUS) || check(TOK_QUESTION) || check(TOK_LBRACE)) {
        next(); // simplificado
    }
    parse_primary();
}

/* primary = identifier | terminal | optional | repeat | group | special_sequence | char_range | hex_char ; */
void parse_primary() {
    if (check(TOK_IDENTIFIER) || check(TOK_STRING) || check(TOK_HEX_CHAR)) {
        next();
    } else if (check(TOK_LBRACKET)) parse_optional();
    else if (check(TOK_LBRACE)) parse_repeat();
    else if (check(TOK_LPAREN)) parse_group();
    else if (check(TOK_QUESTION)) parse_special_sequence();
    else if (check(TOK_HASH)) parse_char_range();
    else {
        fprintf(stderr,"Parse error at %d:%d: unexpected token '%s'\n",
            current_token.line,current_token.col,current_token.text);
        exit(1);
    }
}

void parse_optional() {
    expect(TOK_LBRACKET);
    parse_expression();
    expect(TOK_RBRACKET);
}

void parse_repeat() {
    expect(TOK_LBRACE);
    parse_expression();
    expect(TOK_RBRACE);
}

void parse_group() {
    expect(TOK_LPAREN);
    parse_expression();
    expect(TOK_RPAREN);
}

void parse_special_sequence() {
    expect(TOK_QUESTION);
    while (!check(TOK_QUESTION) && !check(TOK_EOF)) next();
    expect(TOK_QUESTION);
}

void parse_char_range() {
    expect(TOK_HASH);
    if (check(TOK_HEX_CHAR)) next();
    if (check(TOK_DOTDOT)) next();
    if (check(TOK_HEX_CHAR)) next();
}

/* directive = percent, identifier, [ l_paren, literal, r_paren ], percent ; */
void parse_directive() {
    expect(TOK_PERCENT);
    expect(TOK_IDENTIFIER);
    if (check(TOK_LPAREN)) {
        next();
        if (!check(TOK_RPAREN)) parse_primary();
        expect(TOK_RPAREN);
    }
    expect(TOK_PERCENT);
}

/* --- Imprime JSON --- */
void print_json() {
    printf("{\n\"rules\": {\n");
    for (int i = 0; i < rule_count; i++) {
        printf("  \"%s\": {\"type\":\"rule\",\"production\":[", rules[i].name);
        for (int j = 0; j < rules[i].prod_count; j++) {
            printf("\"%s\"", rules[i].productions[j]);
            if (j+1 < rules[i].prod_count) printf(",");
        }
        printf("]}");
        if (i+1 < rule_count) printf(",");
        printf("\n");
    }
    printf("},\n\"tokens\": {\n");
    printf("  \"equals\": \"=\",\n");
    printf("  \"semicolon\": \";\",\n");
    printf("  \"pipe\": \"|\",\n");
    printf("  \"comma\": \",\",\n");
    printf("  \"asterisk\": \"*\",\n");
    printf("  \"plus\": \"+\",\n");
    printf("  \"question\": \"?\",\n");
    printf("  \"minus\": \"-\",\n");
    printf("  \"l_bracket\": \"[\",\n");
    printf("  \"r_bracket\": \"]\",\n");
    printf("  \"l_brace\": \"{\",\n");
    printf("  \"r_brace\": \"}\",\n");
    printf("  \"l_paren\": \"(\",\n");
    printf("  \"r_paren\": \")\",\n");
    printf("  \"dot_dot\": \"..\",\n");
    printf("  \"hash\": \"#\",\n");
    printf("  \"percent\": \"%%\"\n");
    printf("}\n}\n");
}

/* --- Main --- */
int main(int argc, char **argv) {
    if (argc < 2) { printf("Usage: %s file.ebnf\n", argv[0]); return 1; }
    FILE *f = fopen(argv[1],"r");
    if (!f) { perror("fopen"); return 1; }
    fseek(f,0,SEEK_END);
    long len = ftell(f); fseek(f,0,SEEK_SET);
    char *input = malloc(len+1);
    fread(input,1,len,f); input[len]='\0'; fclose(f);
    lexer.input = input; lexer.pos = 0; lexer.line = 1; lexer.col = 1;
    next();
    parse_syntax();
    print_json();
    free(input);
    return 0;
}
