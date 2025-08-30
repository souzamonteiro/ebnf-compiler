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

%token <str> IDENTIFIER STRING INTEGER HEX_CHAR CHARACTER
%token EQUALS PIPE SEMICOLON STAR MINUS RANGE HASH PERCENT QUESTION
%token LBRACKET RBRACKET LBRACE RBRACE LPAREN RPAREN COMMA

%type <node> syntax syntax_rules rule directive definitions_list single_definition
%type <node> syntactic_term syntactic_factor syntactic_primary
%type <node> optional_sequence repeated_sequence grouped_sequence
%type <node> meta_identifier integer terminal_string special_sequence empty_sequence
%type <node> char_range hex_char

%%

syntax
    : syntax_rules { root = $1; }
    ;

syntax_rules
    : /* empty */ { $$ = create_node(NODE_SYNTAX, NULL); }
    | syntax_rules rule { add_child($1, $2); $$ = $1; }
    | syntax_rules directive { add_child($1, $2); $$ = $1; }
    ;

rule
    : meta_identifier EQUALS definitions_list SEMICOLON {
        $$ = create_node(NODE_RULE, $1);
        add_child($$, $3);
        free($1);
      }
    ;

directive
    : PERCENT IDENTIFIER PERCENT {
        $$ = create_node(NODE_DIRECTIVE, $2);
        free($2);
      }
    | PERCENT IDENTIFIER grouped_sequence PERCENT {
        $$ = create_node(NODE_DIRECTIVE, $2);
        add_child($$, $3);
        free($2);
      }
    ;

definitions_list
    : single_definition { $$ = $1; }
    | definitions_list PIPE single_definition {
        $$ = create_node(NODE_ALTERNATIVE, "|");
        add_child($$, $1);
        add_child($$, $3);
      }
    ;

single_definition
    : syntactic_term { $$ = $1; }
    | single_definition COMMA syntactic_term {
        $$ = create_node(NODE_CONCATENATION, ",");
        add_child($$, $1);
        add_child($$, $3);
      }
    ;

syntactic_term
    : syntactic_factor { $$ = $1; }
    | syntactic_factor MINUS syntactic_factor {
        $$ = create_node(NODE_SYNTACTIC_TERM, "-");
        add_child($$, $1);
        add_child($$, $3);
      }
    ;

syntactic_factor
    : syntactic_primary { $$ = $1; }
    | integer STAR syntactic_primary {
        $$ = create_node(NODE_SYNTACTIC_FACTOR, "*");
        add_child($$, create_node(NODE_INTEGER, $1));
        add_child($$, $3);
        free($1);
      }
    ;

syntactic_primary
    : optional_sequence { $$ = $1; }
    | repeated_sequence { $$ = $1; }
    | grouped_sequence { $$ = $1; }
    | meta_identifier { $$ = $1; }
    | terminal_string { $$ = $1; }
    | special_sequence { $$ = $1; }
    | empty_sequence { $$ = $1; }
    | char_range { $$ = $1; }
    ;

optional_sequence
    : LBRACKET definitions_list RBRACKET {
        $$ = create_node(NODE_OPTIONAL_SEQUENCE, NULL);
        add_child($$, $2);
      }
    ;

repeated_sequence
    : LBRACE definitions_list RBRACE {
        $$ = create_node(NODE_REPEATED_SEQUENCE, NULL);
        add_child($$, $2);
      }
    ;

grouped_sequence
    : LPAREN definitions_list RPAREN {
        $$ = create_node(NODE_GROUPED_SEQUENCE, NULL);
        add_child($$, $2);
      }
    ;

meta_identifier
    : IDENTIFIER { $$ = create_node(NODE_META_IDENTIFIER, $1); free($1); }
    ;

integer
    : INTEGER { $$ = create_node(NODE_INTEGER, $1); free($1); }
    ;

terminal_string
    : STRING { $$ = create_node(NODE_TERMINAL_STRING, $1); free($1); }
    ;

special_sequence
    : QUESTION CHARACTER QUESTION {
        $$ = create_node(NODE_SPECIAL_SEQUENCE, $2);
        free($2);
      }
    ;

empty_sequence
    : { $$ = create_node(NODE_EMPTY_SEQUENCE, ""); }
    ;

char_range
    : HEX_CHAR RANGE HEX_CHAR {
        $$ = create_node(NODE_CHAR_RANGE, NULL);
        add_child($$, create_node(NODE_HEX_CHAR, $1));
        add_child($$, create_node(NODE_HEX_CHAR, $3));
        free($1); free($3);
      }
    ;

hex_char
    : HEX_CHAR { $$ = create_node(NODE_HEX_CHAR, $1); free($1); }
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