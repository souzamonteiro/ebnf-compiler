bison -d -v ansi.c.grammar.y
flex ansi.c.grammar.l
gcc -o ansi.c.bin ansi.c.grammar.tab.c lex.yy.c -lfl
