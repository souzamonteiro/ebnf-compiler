%{
    int line_num=0;
%}

%option noyywrap

TAG         \<(.)*\>
WHITESPACE  [\ \t]
LINE        [\n]

%%

{TAG}           { printf("Tag: %s\n",yytext); };
{LINE}          { line_num++; }
{WHITESPACE}    { printf("%s",yytext); }
.               { printf("%s",yytext); }

%%

int main()
{
    while (1) {
        if (yylex() == 0) break;
    }

    printf("\nNumber of lines: %d\n", line_num);

    return 0;
}