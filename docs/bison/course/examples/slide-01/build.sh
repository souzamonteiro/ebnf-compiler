# Run bison on the grammar file. The -d flag generates the header (c_parser.tab.h) for the lexer.
bison -d c_parser.y

# Run flex on the lexer specification file.
flex c_lexer.l

# Compile all C source files together into an executable.
# Note: The order of .c files might matter. Link with -lm if using math, or -lfl for certain lex functions.
# The -lm flag might be needed for asprintf on some systems; otherwise, use sprintf and malloc.
gcc -o c_parser c_parser.tab.c lex.yy.c -lm
