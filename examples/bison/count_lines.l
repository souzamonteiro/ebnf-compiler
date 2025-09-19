%{
/*
    Count lines and characters in input.
    Run with: ./count_lines < inputfile
*/
    int line_num=0,char_num=0;
%}

%option noyywrap

%%

\n  ++line_num;
.   ++char_num;

%%

int main()
{
    while (1) {
        if (yylex() == 0) break;
    }
    
    printf("\nNumber of lines: %d\n", line_num);
    printf( "Number of chars:  %d\n", char_num);

    return 0;
}