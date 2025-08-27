bison -d -v c90.y
flex c90.l
gcc -o c90 c90.tab.c lex.yy.c -lfl
