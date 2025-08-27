#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_HEX_CHAR,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_PIPE,
    TOKEN_COMMA,
    TOKEN_ASTERISK,
    TOKEN_PLUS,
    TOKEN_QUESTION,
    TOKEN_MINUS,
    TOKEN_L_BRACKET,
    TOKEN_R_BRACKET,
    TOKEN_L_BRACE,
    TOKEN_R_BRACE,
    TOKEN_L_PAREN,
    TOKEN_R_PAREN,
    TOKEN_DOT_DOT,
    TOKEN_HASH,
    TOKEN_PERCENT,
    TOKEN_COMMENT,
    TOKEN_DIRECTIVE,
    TOKEN_SPECIAL_SEQUENCE,
    TOKEN_CHAR_RANGE,
    TOKEN_WHITESPACE,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

typedef struct {
    const char* input;
    int position;
    int line;
    int column;
    Token current_token;
} Lexer;

Lexer* lexer_create(const char* input) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_token.type = TOKEN_EOF;
    lexer->current_token.value = NULL;
    return lexer;
}

void lexer_destroy(Lexer* lexer) {
    if (lexer->current_token.value) {
        free(lexer->current_token.value);
    }
    free(lexer);
}

char lexer_peek(Lexer* lexer) {
    return lexer->input[lexer->position];
}

char lexer_peek_next(Lexer* lexer, int offset) {
    return lexer->input[lexer->position + offset];
}

char lexer_advance(Lexer* lexer) {
    char c = lexer_peek(lexer);
    if (c != '\0') {
        lexer->position++;
        lexer->column++;
        if (c == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
    }
    return c;
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (isspace(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
}

bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_hex_digit(char c) {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

char* lexer_read_identifier(Lexer* lexer) {
    int start = lexer->position;
    
    if (is_letter(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
        lexer_advance(lexer);
        while (is_letter(lexer_peek(lexer)) || is_digit(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
            lexer_advance(lexer);
        }
    }
    
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_integer(Lexer* lexer) {
    int start = lexer->position;
    
    while (is_digit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
    
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_string(Lexer* lexer, char quote) {
    int start = lexer->position;
    lexer_advance(lexer); // Skip opening quote
    
    while (lexer_peek(lexer) != quote && lexer_peek(lexer) != '\0') {
        if (lexer_peek(lexer) == '\\') {
            lexer_advance(lexer); // Skip backslash
            if (lexer_peek(lexer) != '\0') {
                lexer_advance(lexer); // Skip escaped character
            }
        } else {
            lexer_advance(lexer);
        }
    }
    
    if (lexer_peek(lexer) == quote) {
        lexer_advance(lexer); // Skip closing quote
    }
    
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_hex_char(Lexer* lexer) {
    int start = lexer->position;
    
    if (lexer_peek(lexer) == '#') {
        lexer_advance(lexer);
        while (is_hex_digit(lexer_peek(lexer))) {
            lexer_advance(lexer);
        }
    }
    
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_comment_block(Lexer* lexer) {
    int start = lexer->position;
    
    // Block comment: (* ... *)
    lexer_advance(lexer); // Skip '('
    lexer_advance(lexer); // Skip '*'
    
    int depth = 1;
    while (depth > 0 && lexer_peek(lexer) != '\0') {
        if (lexer_peek(lexer) == '(' && lexer_peek_next(lexer, 1) == '*') {
            depth++;
            lexer_advance(lexer);
            lexer_advance(lexer);
        } else if (lexer_peek(lexer) == '*' && lexer_peek_next(lexer, 1) == ')') {
            depth--;
            lexer_advance(lexer);
            lexer_advance(lexer);
        } else {
            lexer_advance(lexer);
        }
    }
    
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

char* lexer_read_comment_line(Lexer* lexer) {
    int start = lexer->position;
    
    // Line comment: // ... \n
    lexer_advance(lexer); // Skip '/'
    lexer_advance(lexer); // Skip '/'
    
    while (lexer_peek(lexer) != '\n' && lexer_peek(lexer) != '\r' && lexer_peek(lexer) != '\0') {
        lexer_advance(lexer);
    }
    
    // Don't include the newline in the comment value
    int length = lexer->position - start;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start, length);
    value[length] = '\0';
    return value;
}

Token lexer_next_token(Lexer* lexer) {
    if (lexer->current_token.value) {
        free(lexer->current_token.value);
    }
    
    lexer_skip_whitespace(lexer);
    
    int line = lexer->line;
    int column = lexer->column;
    char current = lexer_peek(lexer);
    
    if (current == '\0') {
        lexer->current_token.type = TOKEN_EOF;
        lexer->current_token.value = strdup("");
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    // Check for multi-character tokens first
    if (current == '(' && lexer_peek_next(lexer, 1) == '*') {
        char* value = lexer_read_comment_block(lexer);
        lexer->current_token.type = TOKEN_COMMENT;
        lexer->current_token.value = value;
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    if (current == '/' && lexer_peek_next(lexer, 1) == '/') {
        char* value = lexer_read_comment_line(lexer);
        lexer->current_token.type = TOKEN_COMMENT;
        lexer->current_token.value = value;
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    if (current == '.' && lexer_peek_next(lexer, 1) == '.') {
        lexer_advance(lexer);
        lexer_advance(lexer);
        lexer->current_token.type = TOKEN_DOT_DOT;
        lexer->current_token.value = strdup("..");
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    // Check for hex char (# followed by hex digits)
    if (current == '#' && is_hex_digit(lexer_peek_next(lexer, 1))) {
        char* value = lexer_read_hex_char(lexer);
        lexer->current_token.type = TOKEN_HEX_CHAR;
        lexer->current_token.value = value;
        lexer->current_token.line = line;
        lexer->current_token.column = column;
        return lexer->current_token;
    }
    
    // Single character tokens
    switch (current) {
        case '=':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_EQUALS;
            lexer->current_token.value = strdup("=");
            break;
        case ';':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_SEMICOLON;
            lexer->current_token.value = strdup(";");
            break;
        case '|':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_PIPE;
            lexer->current_token.value = strdup("|");
            break;
        case ',':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_COMMA;
            lexer->current_token.value = strdup(",");
            break;
        case '*':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_ASTERISK;
            lexer->current_token.value = strdup("*");
            break;
        case '+':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_PLUS;
            lexer->current_token.value = strdup("+");
            break;
        case '?':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_QUESTION;
            lexer->current_token.value = strdup("?");
            break;
        case '-':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_MINUS;
            lexer->current_token.value = strdup("-");
            break;
        case '[':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_L_BRACKET;
            lexer->current_token.value = strdup("[");
            break;
        case ']':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_R_BRACKET;
            lexer->current_token.value = strdup("]");
            break;
        case '{':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_L_BRACE;
            lexer->current_token.value = strdup("{");
            break;
        case '}':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_R_BRACE;
            lexer->current_token.value = strdup("}");
            break;
        case '(':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_L_PAREN;
            lexer->current_token.value = strdup("(");
            break;
        case ')':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_R_PAREN;
            lexer->current_token.value = strdup(")");
            break;
        case '#':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_HASH;
            lexer->current_token.value = strdup("#");
            break;
        case '%':
            lexer_advance(lexer);
            lexer->current_token.type = TOKEN_PERCENT;
            lexer->current_token.value = strdup("%");
            break;
        case '"':
        case '\'':
            lexer->current_token.type = TOKEN_STRING;
            lexer->current_token.value = lexer_read_string(lexer, current);
            break;
        default:
            if (is_digit(current)) {
                lexer->current_token.type = TOKEN_INTEGER;
                lexer->current_token.value = lexer_read_integer(lexer);
            } else if (is_letter(current) || current == '_') {
                lexer->current_token.type = TOKEN_IDENTIFIER;
                lexer->current_token.value = lexer_read_identifier(lexer);
            } else {
                // Unknown character
                char* value = malloc(2);
                value[0] = current;
                value[1] = '\0';
                lexer_advance(lexer);
                lexer->current_token.type = TOKEN_ERROR;
                lexer->current_token.value = value;
            }
            break;
    }
    
    lexer->current_token.line = line;
    lexer->current_token.column = column;
    return lexer->current_token;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_HEX_CHAR: return "HEX_CHAR";
        case TOKEN_EQUALS: return "EQUALS";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_PIPE: return "PIPE";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_ASTERISK: return "ASTERISK";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_QUESTION: return "QUESTION";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_L_BRACKET: return "L_BRACKET";
        case TOKEN_R_BRACKET: return "R_BRACKET";
        case TOKEN_L_BRACE: return "L_BRACE";
        case TOKEN_R_BRACE: return "R_BRACE";
        case TOKEN_L_PAREN: return "L_PAREN";
        case TOKEN_R_PAREN: return "R_PAREN";
        case TOKEN_DOT_DOT: return "DOT_DOT";
        case TOKEN_HASH: return "HASH";
        case TOKEN_PERCENT: return "PERCENT";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_DIRECTIVE: return "DIRECTIVE";
        case TOKEN_SPECIAL_SEQUENCE: return "SPECIAL_SEQUENCE";
        case TOKEN_CHAR_RANGE: return "CHAR_RANGE";
        case TOKEN_WHITESPACE: return "WHITESPACE";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void print_token(Token token) {
    printf("Token(%s, '%s', line=%d, column=%d)\n", 
           token_type_to_string(token.type), 
           token.value, 
           token.line, 
           token.column);
}

// Example usage
#ifdef TEST_LEXER
int main() {
    const char* input = "identifier = expression ;\n"
                       "(* This is a block comment *)\n"
                       "// This is a line comment\n"
                       "terminal = \"string\" | 'char' ;\n"
                       "hex_char = #FF ;\n"
                       "char_range = #00.. #FF ;";
    
    Lexer* lexer = lexer_create(input);
    
    Token token;
    do {
        token = lexer_next_token(lexer);
        print_token(token);
    } while (token.type != TOKEN_EOF);
    
    lexer_destroy(lexer);
    return 0;
}
#endif