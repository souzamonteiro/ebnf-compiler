#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iso_ebnf_lexer_v6.c"

/* -----------------------------
   AST definitions
------------------------------*/
typedef enum {
    NODE_RULE,
    NODE_SEQUENCE,
    NODE_CHOICE,
    NODE_PRIMARY,
    NODE_LITERAL,
    NODE_QUANTIFIER,
    NODE_CHARSET,
    NODE_CHAR_RANGE,
    NODE_SPECIAL_SEQ
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char text[128];             // nome da regra ou literal
    struct ASTNode **children;  // sub-nós
    int child_count;
} ASTNode;

/* Criar novo nó */
ASTNode* create_node(NodeType type, const char *text) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    if(text) strncpy(node->text,text,127);
    else node->text[0]='\0';
    node->children=NULL;
    node->child_count=0;
    return node;
}

/* Adicionar filho */
void node_add_child(ASTNode *parent, ASTNode *child){
    parent->children = realloc(parent->children,sizeof(ASTNode*)*(parent->child_count+1));
    parent->children[parent->child_count++] = child;
}

/* -----------------------------
   Lexer globals
------------------------------*/
Lexer lexer;
Token current_token;

void next() { current_token = next_token(&lexer); }
int check(TokenType t) { return current_token.type == t; }
void expect(TokenType t) {
    if(check(t)) next();
    else{
        fprintf(stderr,"Parse error at %d:%d: expected token %d, got %d ('%s')\n",
            current_token.line,current_token.col,t,current_token.type,current_token.text);
        exit(1);
    }
}

/* -----------------------------
   Parser forward declarations
------------------------------*/
ASTNode* parse_syntax();
ASTNode* parse_rule();
ASTNode* parse_expression();
ASTNode* parse_term();
ASTNode* parse_factor();
ASTNode* parse_primary();
ASTNode* parse_optional();
ASTNode* parse_repeat();
ASTNode* parse_group();
ASTNode* parse_special_sequence();
ASTNode* parse_char_range();
ASTNode* parse_character_set();
ASTNode* parse_directive();

/* -----------------------------
   Parser implementation
------------------------------*/

/* syntax = { rule | directive | comment } */
ASTNode* parse_syntax() {
    ASTNode* root = create_node(NODE_SEQUENCE,"syntax");
    while(!check(TOK_EOF)){
        if(check(TOK_IDENTIFIER))
            node_add_child(root, parse_rule());
        else if(check(TOK_PERCENT))
            node_add_child(root, parse_directive());
        else next(); // ignora comentários
    }
    return root;
}

/* rule = identifier '=' expression ';' */
ASTNode* parse_rule() {
    char name[128];
    strcpy(name,current_token.text);
    next();
    expect(TOK_EQUALS);
    ASTNode* expr = parse_expression();
    expect(TOK_SEMICOLON);
    ASTNode* rule_node = create_node(NODE_RULE,name);
    node_add_child(rule_node,expr);
    return rule_node;
}

/* expression = term { '|' term } */
ASTNode* parse_expression() {
    ASTNode* term_node = parse_term();
    if(check(TOK_PIPE)){
        ASTNode* choice = create_node(NODE_CHOICE,"");
        node_add_child(choice,term_node);
        while(check(TOK_PIPE)){
            next();
            node_add_child(choice, parse_term());
        }
        return choice;
    }
    return term_node;
}

/* term = factor { ',' factor } */
ASTNode* parse_term() {
    ASTNode* factor_node = parse_factor();
    if(check(TOK_COMMA)){
        ASTNode* seq = create_node(NODE_SEQUENCE,"");
        node_add_child(seq,factor_node);
        while(check(TOK_COMMA)){
            next();
            node_add_child(seq,parse_factor());
        }
        return seq;
    }
    return factor_node;
}

/* factor = primary [ quantifier ] */
ASTNode* parse_factor() {
    ASTNode* primary = parse_primary();
    if(check(TOK_ASTERISK)||check(TOK_PLUS)||check(TOK_QUESTION)||check(TOK_LBRACE)){
        ASTNode* qnode = create_node(NODE_QUANTIFIER,current_token.text);
        next(); // consome quantificador simples ou abre brace
        if(primary) node_add_child(qnode,primary);
        if(check(TOK_LBRACE)){
            // parse {n} or {n,m}
            next();
            if(check(TOK_INTEGER)) next();
            if(check(TOK_COMMA)){
                next();
                if(check(TOK_INTEGER)) next();
            }
            expect(TOK_RBRACE);
        }
        return qnode;
    }
    return primary;
}

/* primary = identifier | terminal | optional | repeat | group | special_sequence | char_range | character_set */
ASTNode* parse_primary() {
    if(check(TOK_IDENTIFIER)||check(TOK_STRING)||check(TOK_HEX_CHAR)){
        ASTNode* node = create_node(NODE_LITERAL,current_token.text);
        next();
        return node;
    } else if(check(TOK_LBRACKET)) return parse_optional();
    else if(check(TOK_LBRACE)) return parse_repeat();
    else if(check(TOK_LPAREN)) return parse_group();
    else if(check(TOK_SPECIAL_SEQ)) return parse_special_sequence();
    else if(check(TOK_HASH)) return parse_char_range();
    else{
        fprintf(stderr,"Parse error at %d:%d unexpected token '%s'\n",
            current_token.line,current_token.col,current_token.text);
        exit(1);
    }
    return NULL;
}

/* optional = '[' expression ']' */
ASTNode* parse_optional(){
    expect(TOK_LBRACKET);
    ASTNode* node = create_node(NODE_SEQUENCE,"optional");
    node_add_child(node,parse_expression());
    expect(TOK_RBRACKET);
    return node;
}

/* repeat = '{' expression '}' */
ASTNode* parse_repeat(){
    expect(TOK_LBRACE);
    ASTNode* node = create_node(NODE_SEQUENCE,"repeat");
    node_add_child(node,parse_expression());
    expect(TOK_RBRACE);
    return node;
}

/* group = '(' expression ')' */
ASTNode* parse_group(){
    expect(TOK_LPAREN);
    ASTNode* node = create_node(NODE_SEQUENCE,"group");
    node_add_child(node,parse_expression());
    expect(TOK_RPAREN);
    return node;
}

/* special_sequence = '? ... ?' */
ASTNode* parse_special_sequence(){
    ASTNode* node = create_node(NODE_SPECIAL_SEQ,current_token.text);
    next();
    return node;
}

/* char_range = '#' hex_char '..' '#' hex_char */
ASTNode* parse_char_range(){
    expect(TOK_HASH);
    expect(TOK_HEX_CHAR);
    expect(TOK_DOTDOT);
    expect(TOK_HASH);
    expect(TOK_HEX_CHAR);
    return create_node(NODE_CHAR_RANGE,"#..#");
}

/* character_set = '[' character_element { '-' character_element } ']' */
ASTNode* parse_character_set(){
    expect(TOK_LBRACKET);
    ASTNode* node = create_node(NODE_CHARSET,"charset");
    while(!check(TOK_RBRACKET)){
        if(check(TOK_IDENTIFIER)||check(TOK_STRING)||check(TOK_HEX_CHAR)||check(TOK_SPECIAL_SEQ)){
            node_add_child(node,create_node(NODE_LITERAL,current_token.text));
            next();
        }
        if(check(TOK_MINUS)){
            next(); // consome -
        }
    }
    expect(TOK_RBRACKET);
    return node;
}

/* directive = '%' identifier [ '(' literal ')' ] '%' */
ASTNode* parse_directive(){
    expect(TOK_PERCENT);
    char name[128];
    strcpy(name,current_token.text);
    expect(TOK_IDENTIFIER);
    ASTNode* node = create_node(NODE_RULE,name);
    if(check(TOK_LPAREN)){
        next();
        if(!check(TOK_RPAREN)) node_add_child(node,parse_primary());
        expect(TOK_RPAREN);
    }
    expect(TOK_PERCENT);
    return node;
}

/* -----------------------------
   Print JSON recursively
------------------------------*/
void print_node_json(ASTNode* node,int indent){
    for(int i=0;i<indent;i++) printf(" ");
    switch(node->type){
        case NODE_RULE: printf("{\"type\":\"rule\",\"name\":\"%s\",",node->text); break;
        case NODE_SEQUENCE: printf("{\"type\":\"sequence\","); break;
        case NODE_CHOICE: printf("{\"type\":\"choice\","); break;
        case NODE_LITERAL: printf("{\"type\":\"literal\",\"value\":\"%s\"}",node->text); return;
        case NODE_QUANTIFIER: printf("{\"type\":\"quantifier\",\"symbol\":\"%s\",",node->text); break;
        case NODE_CHARSET: printf("{\"type\":\"charset\","); break;
        case NODE_CHAR_RANGE: printf("{\"type\":\"char_range\"}"); return;
        case NODE_SPECIAL_SEQ: printf("{\"type\":\"special_seq\",\"value\":\"%s\"}",node->text); return;
    }
    if(node->child_count>0){
        printf("\"children\":[\n");
        for(int i=0;i<node->child_count;i++){
            print_node_json(node->children[i],indent+2);
            if(i+1<node->child_count) printf(",\n");
        }
        printf("]}");
    } else printf("}");
}

/* -----------------------------
   Main
------------------------------*/
int main(int argc,char **argv){
    if(argc<2){ printf("Usage: %s file.ebnf\n",argv[0]); return 1; }
    FILE *f=fopen(argv[1],"r");
    if(!f){ perror("fopen"); return 1; }
    fseek(f,0,SEEK_END);
    long len=ftell(f); fseek(f,0,SEEK_SET);
    char *input=malloc(len+1);
    fread(input,1,len,f); input[len]='\0'; fclose(f);

    lexer.input=input; lexer.pos=0; lexer.line=1; lexer.col=1;
    next();

    ASTNode* ast = parse_syntax();

    printf("{\n\"ast\": ");
    print_node_json(ast,2);
    printf("\n}\n");

    free(input);
    return 0;
}
