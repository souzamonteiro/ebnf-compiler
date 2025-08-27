/*
 * iso_ebnf.c — Lexer + Parser + AST + JSON para ISO EBNF (ISO/IEC 14977)
 *
 * Compilar:  gcc -Wall -Wextra -O2 -o iso_ebnf iso_ebnf.c
 * Executar:  ./iso_ebnf <arquivo.ebnf> [--json]
 *
 * Saída: AST em JSON no stdout.
 *
 * Recursos suportados:
 *  - Regras:        identifier '=' expression ';'
 *  - Expressão:     escolhas com '|', sequência com ','
 *  - Primários:     identifier | "string" | 'string' | '(' expr ')' | '[' expr ']' | '{' expr '}'
 *  - Comentários:   (* ... *) com suporte a aninhamento
 *  - Repetição n*:  INTEGER '*' primary   (extensão comum)
 *  - CharRange:     "A" .. "Z"  /  '0' .. '9'
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*============================= Util =======================================*/

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) { fprintf(stderr, "Out of memory\n"); exit(1); }
    return p;
}
static char *xstrdup(const char *s) {
    size_t n = strlen(s);
    char *p = (char*)xmalloc(n+1);
    memcpy(p, s, n+1);
    return p;
}

/*============================= Lexer ======================================*/

typedef enum {
    T_EOF=0,
    T_IDENT,      // [A-Za-z_][A-Za-z0-9_]*
    T_INTEGER,    // [0-9]+
    T_STRING,     // "..." | '...'
    T_EQ,         // =
    T_SEMI,       // ;
    T_OR,         // |
    T_COMMA,      // ,
    T_LPAREN,     // (
    T_RPAREN,     // )
    T_LBRACK,     // [
    T_RBRACK,     // ]
    T_LBRACE,     // {
    T_RBRACE,     // }
    T_STAR,       // *
    T_DOTDOT,     // ..
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;       // para IDENT, INTEGER, STRING guarda o texto (sem aspas no STRING)
    int line, col;
} Token;

typedef struct {
    const char *src;
    size_t len;
    size_t i;
    int line, col;
} Lexer;

static int l_peek(Lexer *L) {
    if (L->i >= L->len) return EOF;
    return (unsigned char)L->src[L->i];
}
static int l_peek2(Lexer *L) {
    if (L->i+1 >= L->len) return EOF;
    return (unsigned char)L->src[L->i+1];
}
static int l_get(Lexer *L) {
    if (L->i >= L->len) return EOF;
    int c = (unsigned char)L->src[L->i++];
    if (c == '\n') { L->line++; L->col = 1; } else { L->col++; }
    return c;
}
static void skip_space_and_comments(Lexer *L) {
    for (;;) {
        int c = l_peek(L);
        // espaços
        while (c == ' ' || c == '\t' || c == '\r' || c == '\n') { l_get(L); c = l_peek(L); }
        // comentários (* ... *) com aninhamento
        if (c == '(' && l_peek2(L) == '*') {
            l_get(L); l_get(L); // consume "(*"
            int depth = 1;
            while (depth > 0) {
                int d = l_get(L);
                if (d == EOF) { fprintf(stderr, "Lex error: unclosed comment\n"); exit(1); }
                if (d == '(' && l_peek(L) == '*') { l_get(L); depth++; }
                else if (d == '*' && l_peek(L) == ')') { l_get(L); depth--; }
            }
            continue; // pode haver mais espaço/comentários
        }
        break;
    }
}
static Token make_tok(TokenType t, const char *lex, int line, int col) {
    Token tk; tk.type=t; tk.lexeme=lex?xstrdup(lex):NULL; tk.line=line; tk.col=col; return tk;
}
static Token lex_token(Lexer *L) {
    skip_space_and_comments(L);
    int c = l_peek(L);
    int line=L->line, col=L->col;

    if (c == EOF) return make_tok(T_EOF, NULL, line, col);

    // Identificador
    if (isalpha(c) || c=='_') {
        size_t start = L->i;
        l_get(L);
        while (isalnum(l_peek(L)) || l_peek(L)=='_') l_get(L);
        size_t end = L->i;
        char *buf = (char*)xmalloc(end-start+1);
        memcpy(buf, L->src+start, end-start);
        buf[end-start]='\0';
        Token t = make_tok(T_IDENT, buf, line, col);
        free(buf);
        return t;
    }

    // Número
    if (isdigit(c)) {
        size_t start=L->i; l_get(L);
        while (isdigit(l_peek(L))) l_get(L);
        size_t end=L->i;
        char *buf=(char*)xmalloc(end-start+1);
        memcpy(buf, L->src+start, end-start); buf[end-start]='\0';
        Token t=make_tok(T_INTEGER, buf, line, col);
        free(buf);
        return t;
    }

    // String (aspas simples ou duplas)
    if (c=='\'' || c=='"') {
        int quote = l_get(L);
        size_t start=L->i;
        for (;;) {
            int d = l_get(L);
            if (d == EOF) { fprintf(stderr,"Lex error: unterminated string at %d:%d\n",line,col); exit(1); }
            if (d == quote) break;
            if (d == '\n') { /* permitido, mas não comum */ }
        }
        size_t end=L->i-1; // antes da aspa final
        char *buf=(char*)xmalloc(end-start+1);
        memcpy(buf, L->src+start, end-start); buf[end-start]='\0';
        Token t=make_tok(T_STRING, buf, line, col);
        free(buf);
        return t;
    }

    // Dois pontos (..)
    if (c=='.' && l_peek2(L)=='.') {
        l_get(L); l_get(L);
        return make_tok(T_DOTDOT, "..", line, col);
    }

    // Símbolos de 1 char
    switch (c) {
        case '=': l_get(L); return make_tok(T_EQ, "=", line, col);
        case ';': l_get(L); return make_tok(T_SEMI, ";", line, col);
        case '|': l_get(L); return make_tok(T_OR, "|", line, col);
        case ',': l_get(L); return make_tok(T_COMMA, ",", line, col);
        case '(': l_get(L); return make_tok(T_LPAREN, "(", line, col);
        case ')': l_get(L); return make_tok(T_RPAREN, ")", line, col);
        case '[': l_get(L); return make_tok(T_LBRACK, "[", line, col);
        case ']': l_get(L); return make_tok(T_RBRACK, "]", line, col);
        case '{': l_get(L); return make_tok(T_LBRACE, "{", line, col);
        case '}': l_get(L); return make_tok(T_RBRACE, "}", line, col);
        case '*': l_get(L); return make_tok(T_STAR, "*", line, col);
        default:
            fprintf(stderr,"Lex error: unexpected char '%c' at %d:%d\n", c, line, col);
            exit(1);
    }
}

/*============================= Parser / AST ================================*/

typedef struct ASTNode ASTNode;
struct ASTNode {
    char *type;          // ex: "Grammar","Rule","Choice","Sequence","Optional","Repetition","Identifier","Terminal","CharRange","Group"
    char *value;         // opcional (ex: nome do identificador, texto do terminal, número de repetição)
    ASTNode **children;  // vetor dinâmico
    int count, cap;
    int line, col;       // para melhor diagnóstico
};

static ASTNode *new_node(const char *type, const char *value, int line, int col) {
    ASTNode *n=(ASTNode*)xmalloc(sizeof(ASTNode));
    n->type=xstrdup(type);
    n->value=value ? xstrdup(value) : NULL;
    n->children=NULL; n->count=0; n->cap=0;
    n->line=line; n->col=col;
    return n;
}
static void add_child(ASTNode *parent, ASTNode *child) {
    if (!child) return;
    if (parent->count==parent->cap) {
        parent->cap = parent->cap? parent->cap*2 : 4;
        parent->children = (ASTNode**)realloc(parent->children, parent->cap*sizeof(ASTNode*));
        if (!parent->children) { fprintf(stderr,"OOM\n"); exit(1); }
    }
    parent->children[parent->count++] = child;
}
static void free_ast(ASTNode *n) {
    if (!n) return;
    for (int i=0;i<n->count;i++) free_ast(n->children[i]);
    free(n->children);
    free(n->type);
    if (n->value) free(n->value);
    free(n);
}

typedef struct {
    Lexer L;
    Token cur;
} Parser;

static void next(Parser *P) { P->cur = lex_token(&P->L); }
static void expect(Parser *P, TokenType t) {
    if (P->cur.type != t) {
        fprintf(stderr,"Parse error: expected token %d, got %d (%s) at %d:%d\n",
                t, P->cur.type, P->cur.lexeme?P->cur.lexeme:"", P->cur.line, P->cur.col);
        exit(1);
    }
}
static int accept(Parser *P, TokenType t) {
    if (P->cur.type == t) { next(P); return 1; }
    return 0;
}

/* Forward decls */
static ASTNode* parse_grammar(Parser *P);
static ASTNode* parse_rule(Parser *P);
static ASTNode* parse_expression(Parser *P);
static ASTNode* parse_term(Parser *P);
static ASTNode* parse_factor(Parser *P);
static ASTNode* parse_primary(Parser *P);

/* Grammar ::= { Rule } EOF */
static ASTNode* parse_grammar(Parser *P) {
    ASTNode *G = new_node("Grammar", NULL, P->cur.line, P->cur.col);
    while (P->cur.type != T_EOF) {
        add_child(G, parse_rule(P));
    }
    return G;
}

/* Rule ::= IDENT '=' Expression ';' */
static ASTNode* parse_rule(Parser *P) {
    if (P->cur.type != T_IDENT) {
        fprintf(stderr, "Parse error: expected IDENT at %d:%d\n", P->cur.line, P->cur.col);
        exit(1);
    }
    char *name = xstrdup(P->cur.lexeme);
    int line=P->cur.line, col=P->cur.col;
    next(P);
    expect(P, T_EQ); next(P);
    ASTNode *expr = parse_expression(P);
    expect(P, T_SEMI); next(P);

    ASTNode *R = new_node("Rule", name, line, col);
    add_child(R, expr);
    free(name);
    return R;
}

/* Expression ::= Term { '|' Term } */
static ASTNode* parse_expression(Parser *P) {
    ASTNode *first = parse_term(P);
    if (P->cur.type != T_OR) return first;

    ASTNode *choice = new_node("Choice", NULL, first->line, first->col);
    add_child(choice, first);
    while (accept(P, T_OR)) {
        add_child(choice, parse_term(P));
    }
    return choice;
}

/* Term ::= Factor { ',' Factor } */
static ASTNode* parse_term(Parser *P) {
    ASTNode *first = parse_factor(P);
    if (P->cur.type != T_COMMA) return first;

    ASTNode *seq = new_node("Sequence", NULL, first->line, first->col);
    add_child(seq, first);
    while (accept(P, T_COMMA)) {
        add_child(seq, parse_factor(P));
    }
    return seq;
}

/* Factor ::= [ INTEGER '*' ] Primary  */
static ASTNode* parse_factor(Parser *P) {
    if (P->cur.type == T_INTEGER) {
        char *rep = xstrdup(P->cur.lexeme);
        int line=P->cur.line, col=P->cur.col;
        next(P);
        expect(P, T_STAR); next(P);
        ASTNode *prim = parse_primary(P);

        ASTNode *repNode = new_node("Repetition", rep, line, col);
        add_child(repNode, prim);
        free(rep);
        return repNode;
    }
    return parse_primary(P);
}

/* Primary ::= IDENT | STRING | '(' Expression ')' | '[' Expression ']' | '{' Expression '}' | CharRange */
static ASTNode* parse_primary(Parser *P) {
    // IDENT
    if (P->cur.type == T_IDENT) {
        ASTNode *n = new_node("Identifier", P->cur.lexeme, P->cur.line, P->cur.col);
        next(P);
        return n;
    }
    // STRING (checar range STRING .. STRING)
    if (P->cur.type == T_STRING) {
        // guardar literal atual
        char *lit = xstrdup(P->cur.lexeme);
        int line=P->cur.line, col=P->cur.col;
        next(P);
        if (P->cur.type == T_DOTDOT) {
            next(P);
            if (P->cur.type != T_STRING) {
                fprintf(stderr,"Parse error: expected STRING after '..' at %d:%d\n", P->cur.line, P->cur.col);
                free(lit); exit(1);
            }
            // CharRange
            ASTNode *cr = new_node("CharRange", NULL, line, col);
            ASTNode *from = new_node("Terminal", lit, line, col);
            ASTNode *to   = new_node("Terminal", P->cur.lexeme, P->cur.line, P->cur.col);
            add_child(cr, from); add_child(cr, to);
            next(P);
            free(lit);
            return cr;
        } else {
            ASTNode *t = new_node("Terminal", lit, line, col);
            free(lit);
            return t;
        }
    }
    // '(' expr ')'
    if (P->cur.type == T_LPAREN) {
        int line=P->cur.line, col=P->cur.col;
        next(P);
        ASTNode *e = parse_expression(P);
        expect(P, T_RPAREN); next(P);
        ASTNode *g = new_node("Group", NULL, line, col);
        add_child(g, e);
        return g;
    }
    // '[' expr ']'
    if (P->cur.type == T_LBRACK) {
        int line=P->cur.line, col=P->cur.col;
        next(P);
        ASTNode *e = parse_expression(P);
        expect(P, T_RBRACK); next(P);
        ASTNode *opt = new_node("Optional", NULL, line, col);
        add_child(opt, e);
        return opt;
    }
    // '{' expr '}'
    if (P->cur.type == T_LBRACE) {
        int line=P->cur.line, col=P->cur.col;
        next(P);
        ASTNode *e = parse_expression(P);
        expect(P, T_RBRACE); next(P);
        ASTNode *rep = new_node("Repetition", NULL, line, col);
        add_child(rep, e);
        return rep;
    }

    fprintf(stderr,"Parse error: unexpected token near %d:%d\n", P->cur.line, P->cur.col);
    exit(1);
}

/*============================= JSON Printer ===============================*/

static void json_escape_str(const char *s, FILE *out) {
    fputc('"', out);
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '\\' || c == '\"') { fputc('\\', out); fputc(c, out); }
        else if (c == '\n') { fputs("\\n", out); }
        else if (c == '\r') { fputs("\\r", out); }
        else if (c == '\t') { fputs("\\t", out); }
        else if (c < 0x20) { fprintf(out, "\\u%04x", c); }
        else { fputc(c, out); }
    }
    fputc('"', out);
}

static void print_json_iso(ASTNode *n, int indent);

static void indent_n(int n) { for (int i=0;i<n;i++) fputs("  ", stdout); }

static void print_json_iso(ASTNode *n, int indent) {
    indent_n(indent);

    if (!strcmp(n->type,"Grammar")) {
        printf("{\n");
        indent_n(indent+1); printf("\"Grammar\": [\n");
        for (int i=0;i<n->count;i++) {
            print_json_iso(n->children[i], indent+2);
            if (i<n->count-1) printf(",\n");
        }
        printf("\n"); indent_n(indent+1); printf("]\n");
        indent_n(indent); printf("}");
        return;
    }

    if (!strcmp(n->type,"Rule")) {
        printf("{\"Rule\": { \"name\": ");
        json_escape_str(n->value, stdout);
        printf(", \"expr\": ");
        print_json_iso(n->children[0], 0);
        printf("} }");
        return;
    }

    if (!strcmp(n->type,"Sequence") || !strcmp(n->type,"Choice")) {
        printf("{\"%s\": [", n->type);
        for (int i=0;i<n->count;i++) {
            print_json_iso(n->children[i], 0);
            if (i<n->count-1) printf(", ");
        }
        printf("]}");
        return;
    }

    if (!strcmp(n->type,"Repetition") || !strcmp(n->type,"Optional") || !strcmp(n->type,"Group")) {
        printf("{\"%s\": ", n->type);
        if (n->count>0) print_json_iso(n->children[0], 0);
        else if(n->value) { json_escape_str(n->value, stdout); }
        else printf("null");
        printf("}");
        return;
    }

    if (!strcmp(n->type,"Identifier") || !strcmp(n->type,"Terminal")) {
        printf("{\"%s\": ", n->type);
        json_escape_str(n->value, stdout);
        printf("}");
        return;
    }

    if (!strcmp(n->type,"CharRange")) {
        printf("{\"CharRange\": [");
        for (int i=0;i<n->count;i++) {
            print_json_iso(n->children[i], 0);
            if (i<n->count-1) printf(", ");
        }
        printf("]}");
        return;
    }

    // fallback
    printf("{\"%s\": %s}", n->type, n->value?n->value:"null");
}

/*============================= Main =======================================*/

static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); exit(1); }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    if (n < 0) { perror("ftell"); exit(1); }
    fseek(f, 0, SEEK_SET);
    char *buf = (char*)xmalloc((size_t)n + 1);
    size_t r = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[r] = 0;
    if (out_len) *out_len = r;
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,"Uso: %s <arquivo.ebnf> [--json]\n", argv[0]);
        return 1;
    }
    size_t len=0;
    char *text = read_file(argv[1], &len);

    Parser P;
    P.L.src=text; P.L.len=len; P.L.i=0; P.L.line=1; P.L.col=1;
    next(&P); // carrega primeiro token

    ASTNode *ast = parse_grammar(&P);
    expect(&P, T_EOF);

    // Apenas JSON por enquanto (flag --json aceita mas é padrão)
    print_json_iso(ast, 0);
    fputc('\n', stdout);

    free_ast(ast);
    free(text);
    return 0;
}

/*===================== Exemplo robusto de ISO EBNF =========================

Salve isso como "robust_sample.ebnf" para testar:

(* Gramática de linguagem simples estilo Pascal em ISO EBNF, sem regex *)
program   = "PROGRAM", identifier, ";", block, "." ;
block     = { declaration }, "BEGIN", { statement, ";" }, "END" ;

declaration = "VAR", identifier, ":", type, ";" ;
type        = "INTEGER" | "BOOLEAN" ;

statement =
      assignment
    | if_stmt
    | while_stmt
    | block ;

assignment = identifier, ":=", expression ;

if_stmt    = "IF", expression, "THEN", statement, [ "ELSE", statement ] ;
while_stmt = "WHILE", expression, "DO", statement ;

expression = term, { ("+" | "-"), term } ;
term       = factor, { ("*" | "/"), factor } ;

factor =
      identifier
    | number
    | "TRUE"
    | "FALSE"
    | "(", expression, ")" ;

identifier = letter, { letter | digit | "_" } ;
number     = digit, { digit } ;

(* Letras e dígitos usando faixas, para testar CharRange *)
letter     = "A".."Z" | "a".."z" ;
digit      = "0".."9" ;

============================================================================
*/
