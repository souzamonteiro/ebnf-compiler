%{
#include "ast.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>

void yyerror(const char *s);
extern FILE *yyin;

#define YY_NO_UNPUT 1
#define YY_NO_INPUT 1

%}

%option noyywrap
%option yylineno
%option noinput
%option nounput

%%

[ \t\n\r]+    { /* ignore whitespace */ }

"//".*        { /* ignore line comments */ }
"(*"([^*]|"*"[^)])*"*)" { /* ignore block comments */ }

"="           { return EQUALS; }
"|"           { return PIPE; }
";"           { return SEMICOLON; }
"*"           { return STAR; }
"+"           { return PLUS; }
"?"           { return QUESTION; }
"["           { return LBRACKET; }
"]"           { return RBRACKET; }
"{"           { return LBRACE; }
"}"           { return RBRACE; }
"("           { return LPAREN; }
")"           { return RPAREN; }
".."          { return RANGE; }
"%"           { return PERCENT; }
","           { return COMMA; }
"#"           { return HASH; }

\"([^"\\]|\\.)*\"   |
\'([^'\\]|\\.)*\'   { 
    yylval.str = strdup(yytext); 
    return STRING; 
}

"#"[0-9A-Fa-f]{4} {
    yylval.str = strdup(yytext);
    return HEX_CHAR;
}

[0-9]+        { 
    yylval.str = strdup(yytext); 
    return INTEGER; 
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);
    return IDENTIFIER;
}

.             { 
    yylval.str = strdup(yytext); 
    return CHARACTER; 
}

%%