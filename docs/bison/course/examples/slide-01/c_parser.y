%{
/*** C Code Section - Included at the top of the generated file ***/

#include <stdio.h>
#include <stdlib.h>

// Function to report errors
void yyerror(const char *s) {
    fprintf(stderr, "Parser Error: %s\n", s);
}

// Declare the lexer function
extern int yylex(void);

// Helper function to print XML (simplified for the example)
void print_xml_open(const char* tag) { printf("<%s>", tag); }
void print_xml_close(const char* tag) { printf("</%s>", tag); }
void print_xml_tag(const char* tag, const char* value) { printf("<%s>%s</%s>", tag, value, tag); }

%}

/*** Bison Declarations Section ***/

// Define the types for the semantic values (union fields)
%union {
    int num;
    char* str;
    char* xml; // Adicionado para valores XML
}

// Declare Tokens and their value type (from the %union)
%token <str> IDENTIFIER
%token <num> NUMBER
%token INT RETURN IF WHILE
%token PLUS MINUS ASSIGN EQ LT
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON
%token ERROR

// Declare the types of nonterminals
%type <xml> program function compound_statement statements statement expression
%type <str> type

// Define the start symbol of the grammar
%start program

%%

/*** Grammar Rules and Actions Section ***/

program
    : /* empty */ { printf("<program>\n"); } // Can be an empty program
    | program function { print_xml_close("program"); } // End the program after all functions
    ;

function
    : type IDENTIFIER LPAREN RPAREN compound_statement {
        printf("<function>");
        print_xml_tag("name", $2);
        printf("%s", $5); // Output the XML from compound_statement
        print_xml_close("function");
        free($2); // Free the allocated memory for the identifier
    }
    ;

type
    : INT { $$ = "int"; printf("<type>int</type>"); }
    ;

compound_statement
    : LBRACE statements RBRACE { $$ = $2; } // $$ is the value of this rule, passed up
    ;

statements
    : /* empty */ { 
        char* empty = malloc(1);
        empty[0] = '\0';
        $$ = empty; 
    } // A block can be empty
    | statements statement { 
        // Concatenar os statements
        char* result;
        asprintf(&result, "%s%s", $1, $2);
        free($1);
        free($2);
        $$ = result;
    }
    ;

statement
    : expression SEMICOLON {
        char* result;
        asprintf(&result, "<statement>%s</statement>", $1);
        free($1);
        $$ = result;
    }
    | RETURN expression SEMICOLON {
        char* result;
        asprintf(&result, "<return>%s</return>", $2);
        free($2);
        $$ = result;
    }
    ;

expression
    : IDENTIFIER ASSIGN expression {
        // Dynamically allocate a string for the output (simplified)
        char* result;
        asprintf(&result, "<assignment><lvalue>%s</lvalue><rvalue>%s</rvalue></assignment>", $1, $3);
        $$ = result;
        free($1); // Free the identifier string from the lexer
        free($3); // Free the expression string from the recursive rule
    }
    | IDENTIFIER {
        char* result;
        asprintf(&result, "<variable>%s</variable>", $1);
        $$ = result;
        free($1);
    }
    | NUMBER {
        char* result;
        asprintf(&result, "<constant>%d</constant>", $1);
        $$ = result;
    }
    ;

%%

/*** Additional C Code - The main function ***/

int main(void) {
    // Start the parsing process. yyparse() will call yylex() until done.
    printf("Starting Parser...\n");
    yyparse();
    printf("Parsing Finished.\n");
    return 0;
}