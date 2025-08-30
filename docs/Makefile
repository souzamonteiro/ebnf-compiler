CC = gcc
CFLAGS = -g -Wall
FLEX = flex
BISON = bison

all: parser

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

parser: lex.yy.c parser.tab.c ast.o
	$(CC) $(CFLAGS) -o $@ parser.tab.c lex.yy.c ast.o

clean:
	rm -f *.o parser.tab.c parser.tab.h lex.yy.c parser

test: parser
	./parser test_input.ebnf