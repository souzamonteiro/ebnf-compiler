#!/bin/sh

bison -d grammar.y
flex grammar.l
gcc grammar.tab.c lex.yy.c -o parser -lfl
