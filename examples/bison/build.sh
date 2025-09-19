#!/bin/sh

flex -o count_lines.c count_lines.l
gcc -o count_lines count_lines.c

flex -o simple_html_lexer.c simple_html_lexer.l
gcc -o simple_html_lexer simple_html_lexer.c

flex -o simple_html_lexer_2.c simple_html_lexer_2.l
gcc -o simple_html_lexer_2 simple_html_lexer_2.c

flex -o simple_html_lexer_3.c simple_html_lexer_3.l
gcc -o simple_html_lexer_3 simple_html_lexer_3.c