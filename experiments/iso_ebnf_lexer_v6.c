#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* -----------------------------
   Token Types
-------------------------------*/
typedef enum {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_INTEGER,
    TOK_HEX_CHAR,
    TOK_CHAR_RANGE,
    TOK_STRING,
    TOK_SPECIAL_SEQ,
    TOK_EQUALS,
    TOK_SEMICOLON,
    TOK_PIPE,
    TOK_COMMA,
    TOK_ASTERISK,
    TOK_PLUS,
    TOK_QUESTION,
    TOK_MINUS,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_DOTDOT,
    TOK_HASH,
    TOK_PERCENT,
    TOK_UNKNOWN
} TokenType;

/* -----------------------------
   Token structure
-------------------------------*/
typedef struct {
    TokenType type;
    char text[256];
    int line;
    int col;
} Token;

/* -----------------------------
   Lexer structure
-------------------------------*/
typedef struct {
    const char *input;
    int pos;
    int line;
    int col;
} Lexer;

/* -----------------------------
   Lexer utility functions
-------------------------------*/
char peek(Lexer *lex) {
    return lex->input[lex->pos];
}

char advance(Lexer *lex) {
    char c = lex->input[lex->pos];
    if (c == '\n') { lex->line++; lex->col = 1; }
    else lex->col++;
    lex->pos++;
    return c;
}

int match(Lexer *lex, char expected) {
    if (peek(lex) == expected) { advance(lex); return 1; }
    return 0;
}

void skip_spaces(Lexer *lex) {
    char c;
    while ((c = peek(lex))) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') advance(lex);
        else if (c == '(' && lex->input[lex->pos+1] == '*') { // block comment
            advance(lex); advance(lex);
            while (peek(lex) && !(peek(lex) == '*' && lex->input[lex->pos+1] == ')')) advance(lex);
            if (peek(lex)) { advance(lex); advance(lex); }
        }
        else if (c == '/' && lex->input[lex->pos+1] == '/') { // line comment
            advance(lex); advance(lex);
            while (peek(lex) && peek(lex) != '\n') advance(lex);
        }
        else break;
    }
}

int is_letter(char c) { return isalpha(c); }
int is_digit(char c) { return isdigit(c); }
int is_hex_digit(char c) { return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }

/* -----------------------------
   Lexer core
-------------------------------*/
Token next_token(Lexer *lex) {
    skip_spaces(lex);
    Token tok;
    tok.line = lex->line; tok.col = lex->col;
    tok.text[0] = '\0';

    char c = peek(lex);
    if (!c) { tok.type = TOK_EOF; return tok; }

    // Single character tokens
    switch (c) {
        case '=': advance(lex); tok.type = TOK_EQUALS; strcpy(tok.text,"="); return tok;
        case ';': advance(lex); tok.type = TOK_SEMICOLON; strcpy(tok.text,";"); return tok;
        case '|': advance(lex); tok.type = TOK_PIPE; strcpy(tok.text,"|"); return tok;
        case ',': advance(lex); tok.type = TOK_COMMA; strcpy(tok.text,","); return tok;
        case '*': advance(lex); tok.type = TOK_ASTERISK; strcpy(tok.text,"*"); return tok;
        case '+': advance(lex); tok.type = TOK_PLUS; strcpy(tok.text,"+"); return tok;
        case '?': advance(lex); tok.type = TOK_QUESTION; strcpy(tok.text,"?"); return tok;
        case '-': advance(lex); tok.type = TOK_MINUS; strcpy(tok.text,"-"); return tok;
        case '[': advance(lex); tok.type = TOK_LBRACKET; strcpy(tok.text,"["); return tok;
        case ']': advance(lex); tok.type = TOK_RBRACKET; strcpy(tok.text,"]"); return tok;
        case '{': advance(lex); tok.type = TOK_LBRACE; strcpy(tok.text,"{"); return tok;
        case '}': advance(lex); tok.type = TOK_RBRACE; strcpy(tok.text,"}"); return tok;
        case '(': advance(lex); tok.type = TOK_LPAREN; strcpy(tok.text,"("); return tok;
        case ')': advance(lex); tok.type = TOK_RPAREN; strcpy(tok.text,")"); return tok;
        case '#': { // hex char
            advance(lex);
            char hex1 = peek(lex); advance(lex);
            char hex2 = peek(lex); advance(lex);
            sprintf(tok.text,"#%c%c",hex1,hex2);
            tok.type = TOK_HEX_CHAR;
            return tok;
        }
        case '%': advance(lex); tok.type = TOK_PERCENT; strcpy(tok.text,"%"); return tok;
    }

    // Dot-dot ".."
    if (c == '.' && lex->input[lex->pos+1] == '.') {
        advance(lex); advance(lex);
        tok.type = TOK_DOTDOT; strcpy(tok.text,".."); return tok;
    }

    // Identifiers (letter | '_') { letter | digit | '_' }
    if (is_letter(c) || c == '_') {
        int start = lex->pos;
        while (is_letter(peek(lex)) || is_digit(peek(lex)) || peek(lex) == '_') advance(lex);
        int len = lex->pos - start;
        strncpy(tok.text, lex->input + start, len); tok.text[len]='\0';
        tok.type = TOK_IDENTIFIER;
        return tok;
    }

    // Integer
    if (is_digit(c)) {
        int start = lex->pos;
        while (is_digit(peek(lex))) advance(lex);
        int len = lex->pos - start;
        strncpy(tok.text, lex->input + start, len); tok.text[len]='\0';
        tok.type = TOK_INTEGER;
        return tok;
    }

    // String literal "..." or '...'
    if (c == '"' || c == '\'') {
        char quote = c; advance(lex);
        int i=0;
        while (peek(lex) && peek(lex) != quote) {
            if (peek(lex)=='\\') { tok.text[i++] = advance(lex); tok.text[i++] = advance(lex); }
            else tok.text[i++] = advance(lex);
        }
        advance(lex); tok.text[i]='\0';
        tok.type = TOK_STRING;
        return tok;
    }

    // Special sequence ? ... ?
    if (c == '?') {
        int start = lex->pos;
        advance(lex);
        while (peek(lex) && peek(lex) != '?') advance(lex);
        if (peek(lex)) advance(lex);
        int len = lex->pos - start;
        strncpy(tok.text, lex->input + start, len); tok.text[len]='\0';
        tok.type = TOK_SPECIAL_SEQ;
        return tok;
    }

    // Unknown
    advance(lex);
    tok.type = TOK_UNKNOWN;
    strcpy(tok.text,"?");
    return tok;
}

/* -----------------------------
   Main test
-------------------------------*/
#ifdef __LEXER_MAIN__
int main(int argc, char **argv) {
    if (argc < 2) { printf("Usage: %s file.ebnf\n", argv[0]); return 1; }

    FILE *f = fopen(argv[1],"r");
    if (!f) { perror("fopen"); return 1; }

    fseek(f,0,SEEK_END);
    long len = ftell(f); fseek(f,0,SEEK_SET);

    char *input = malloc(len+1);
    fread(input,1,len,f); input[len]='\0'; fclose(f);

    Lexer lex = { input, 0, 1, 1 };
    Token tok;
    do {
        tok = next_token(&lex);
        printf("TOKEN: %d\t'%s' (%d:%d)\n", tok.type, tok.text, tok.line, tok.col);
    } while (tok.type != TOK_EOF);

    free(input);
    return 0;
}
#endif
