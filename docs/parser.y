%{
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

extern int yylineno;
extern FILE *yyin;
void yyerror(const char *s);
int yylex(void);

ASTNode* root;

%}

%union {
    char* str;
    ASTNode* node;
}

%token <str> IDENTIFIER STRING INTEGER HEX_CHAR CHARACTER SPECIAL_SEQUENCE
%token EQUALS PIPE SEMICOLON STAR PLUS QUESTION
%token LBRACKET RBRACKET LBRACE RBRACE LPAREN RPAREN
%token RANGE PERCENT COMMA HASH

%type <node> syntax rule_list rule expression term factor primary
%type <node> repetition optional repeated group special_sequence
%type <node> char_range

%%

syntax
    : rule_list { root = $1; }
    ;

rule_list
    : rule { $$ = $1; }
    | rule_list rule { 
        $$ = create_node(NODE_EXPRESSION, "rule_list");
        add_child($$, $1);
        add_child($$, $2);
      }
    ;

rule
    : IDENTIFIER EQUALS expression SEMICOLON {
        $$ = create_node(NODE_RULE, $1);
        add_child($$, $3);
        free($1);
      }
    ;

expression
    : term { $$ = $1; }
    | expression PIPE term {
        $$ = create_node(NODE_EXPRESSION, "|");
        add_child($$, $1);
        add_child($$, $3);
      }
    ;

term
    : factor { $$ = $1; }
    | term factor {
        $$ = create_node(NODE_TERM, "concatenation");
        add_child($$, $1);
        add_child($$, $2);
      }
    ;

factor
    : primary repetition {
        $$ = create_node(NODE_FACTOR, NULL);
        add_child($$, $1);
        add_child($$, $2);
      }
    | primary {
        $$ = create_node(NODE_FACTOR, NULL);
        add_child($$, $1);
      }
    ;

repetition
    : STAR { $$ = create_node(NODE_REPETITION, "*"); }
    | PLUS { $$ = create_node(NODE_REPETITION, "+"); }
    | QUESTION { $$ = create_node(NODE_REPETITION, "?"); }
    | LBRACE INTEGER COMMA INTEGER RBRACE {
        $$ = create_node(NODE_REPETITION, "range");
        add_child($$, create_node(NODE_INTEGER, $2));
        add_child($$, create_node(NODE_INTEGER, $4));
        free($2); free($4);
      }
    | LBRACE INTEGER COMMA RBRACE {
        $$ = create_node(NODE_REPETITION, "range_min");
        add_child($$, create_node(NODE_INTEGER, $2));
        free($2);
      }
    | LBRACE INTEGER RBRACE {
        $$ = create_node(NODE_REPETITION, "exact");
        add_child($$, create_node(NODE_INTEGER, $2));
        free($2);
      }
    ;

primary
    : IDENTIFIER { $$ = create_node(NODE_IDENTIFIER, $1); free($1); }
    | STRING { $$ = create_node(NODE_TERMINAL, $1); free($1); }
    | HEX_CHAR { $$ = create_node(NODE_HEX_CHAR, $1); free($1); }
    | optional { $$ = $1; }
    | repeated { $$ = $1; }
    | group { $$ = $1; }
    | special_sequence { $$ = $1; }
    | char_range { $$ = $1; }
    ;

optional
    : LBRACKET expression RBRACKET {
        $$ = create_node(NODE_OPTIONAL, NULL);
        add_child($$, $2);
      }
    ;

repeated
    : LBRACE expression RBRACE {
        $$ = create_node(NODE_REPEATED, NULL);
        add_child($$, $2);
      }
    ;

group
    : LPAREN expression RPAREN {
        $$ = create_node(NODE_GROUP, NULL);
        add_child($$, $2);
      }
    ;

special_sequence
    : QUESTION CHARACTER QUESTION {
        $$ = create_node(NODE_SPECIAL_SEQUENCE, $2);
        free($2);
      }
    | QUESTION CHARACTER CHARACTER QUESTION {
        char* combined = malloc(strlen($2) + strlen($3) + 1);
        strcpy(combined, $2);
        strcat(combined, $3);
        $$ = create_node(NODE_SPECIAL_SEQUENCE, combined);
        free($2); free($3);
      }
    ;

char_range
    : HEX_CHAR RANGE HEX_CHAR {
        $$ = create_node(NODE_CHAR_RANGE, NULL);
        add_child($$, create_node(NODE_HEX_CHAR, $1));
        add_child($$, create_node(NODE_HEX_CHAR, $3));
        free($1); free($3);
      }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Error opening file");
            return 1;
        }
        yyin = file;
    }
    
    yyparse();
    
    if (root) {
        printf("Abstract Syntax Tree:\n");
        print_ast(root, 0);
        free_ast(root);
    }
    
    return 0;
}