#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import re
import json

# ---------------------------
# Lexer
# ---------------------------

class Token:
    def __init__(self, type_, value, line, col):
        self.type = type_
        self.value = value
        self.line = line
        self.col = col

    def __repr__(self):
        return f"Token({self.type}, {self.value}, {self.line}:{self.col})"

class Lexer:
    """Simple regex-based lexer for ISO EBNF with %RULES%/%TOKENS% blocks"""
    token_specification = [
        ('PERCENT', r'%'),
        ('EQUALS', r'='),
        ('SEMICOLON', r';'),
        ('PIPE', r'\|'),
        ('COMMA', r','),
        ('ASTERISK', r'\*'),
        ('PLUS', r'\+'),
        ('QUESTION', r'\?'),
        ('MINUS', r'-'),
        ('LBRACKET', r'\['),
        ('RBRACKET', r'\]'),
        ('LBRACE', r'\{'),
        ('RBRACE', r'\}'),
        ('LPAREN', r'\('),
        ('RPAREN', r'\)'),
        ('DOTDOT', r'\.\.'),
        ('HASH', r'#'),
        ('STRING', r'"([^"\\]|\\.)*"|\'([^\'\\]|\\.)*\''),
        ('IDENTIFIER', r'[A-Za-z_][A-Za-z0-9_]*'),
        ('INTEGER', r'\d+'),
        ('SPACE', r'[ \t\n\r]+'),
        ('COMMENT1', r'\(\*.*?\*\)'),
        ('COMMENT2', r'//[^\n]*'),
    ]
    def __init__(self, text):
        self.text = text
        self.tokens = []
        self.pos = 0
        self.line = 1
        self.col = 1
        self.regex = re.compile('|'.join('(?P<%s>%s)' % pair for pair in self.token_specification), re.DOTALL)
        self.tokenize()

    def tokenize(self):
        for mo in self.regex.finditer(self.text):
            kind = mo.lastgroup
            value = mo.group()
            if kind == 'SPACE' or kind.startswith('COMMENT'):
                self.update_pos(value)
                continue
            tok = Token(kind, value, self.line, self.col)
            self.tokens.append(tok)
            self.update_pos(value)

    def update_pos(self, text):
        lines = text.split('\n')
        if len(lines) > 1:
            self.line += len(lines) - 1
            self.col = len(lines[-1]) + 1
        else:
            self.col += len(text)

    def peek(self):
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return None

    def next(self):
        tok = self.peek()
        if tok:
            self.pos += 1
        return tok

# ---------------------------
# Parser
# ---------------------------

class Parser:
    """Recursive-descent parser for ISO EBNF"""
    def __init__(self, lexer):
        self.lexer = lexer
        self.pos = 0
        self.tokens = lexer.tokens

    def current(self):
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return None

    def advance(self):
        self.pos += 1

    def expect(self, type_):
        tok = self.current()
        if not tok or tok.type != type_:
            raise SyntaxError(f"Expected {type_} at pos {self.pos}, got {tok}")
        self.advance()
        return tok

    def parse_syntax(self):
        """Parse the entire syntax file with %RULES%/%TOKENS% blocks"""
        nodes = []
        mode = None
        while self.current():
            tok = self.current()
            if tok.type == 'PERCENT':
                directive = self.parse_directive()
                if directive == 'RULES':
                    mode = 'RULES'
                elif directive == 'TOKENS':
                    mode = 'TOKENS'
                else:
                    mode = None
            elif mode == 'RULES' and tok.type == 'IDENTIFIER':
                nodes.append(self.parse_rule())
            elif mode == 'TOKENS' and tok.type == 'IDENTIFIER':
                nodes.append(self.parse_token())
            else:
                self.advance()  # skip any unhandled token
        return nodes

    def parse_directive(self):
        self.expect('PERCENT')
        ident = self.expect('IDENTIFIER').value
        self.expect('PERCENT')
        return ident

    # ---------------------------
    # Rules Parsing
    # ---------------------------

    def parse_rule(self):
        rule_name = self.expect('IDENTIFIER').value
        self.expect('EQUALS')
        expr = self.parse_expression()
        self.expect('SEMICOLON')
        return {'rule': rule_name, 'expr': expr}

    def parse_expression(self):
        terms = [self.parse_term()]
        while self.current() and self.current().type == 'PIPE':
            self.expect('PIPE')
            terms.append(self.parse_term())
        if len(terms) == 1:
            return terms[0]
        return {'choice': terms}

    def parse_term(self):
        factors = [self.parse_factor()]
        while self.current() and self.current().type == 'COMMA':
            self.expect('COMMA')
            factors.append(self.parse_factor())
        if len(factors) == 1:
            return factors[0]
        return {'sequence': factors}

    def parse_factor(self):
        # First parse the primary element
        prim = self.parse_primary()

        # Then check if a quantifier follows
        quant = None
        tok = self.current()
        if tok and tok.type in ('ASTERISK','PLUS','QUESTION','LBRACE'):
            quant = self.parse_quantifier()

        if quant:
            return {'quantified': {'prim': prim, 'quant': quant}}
        return prim


    def parse_quantifier(self):
        tok = self.current()
        if tok.type == 'ASTERISK':
            self.advance()
            return {'min':0,'max':None}
        elif tok.type == 'PLUS':
            self.advance()
            return {'min':1,'max':None}
        elif tok.type == 'QUESTION':
            self.advance()
            return {'min':0,'max':1}
        elif tok.type == 'LBRACE':
            self.expect('LBRACE')
            min_val = int(self.expect('INTEGER').value)
            max_val = min_val
            if self.current() and self.current().type == 'COMMA':
                self.expect('COMMA')
                if self.current() and self.current().type == 'INTEGER':
                    max_val = int(self.expect('INTEGER').value)
                else:
                    max_val = None
            self.expect('RBRACE')
            return {'min': min_val,'max': max_val}
        else:
            return None

    def parse_primary(self):
        tok = self.current()
        if tok.type == 'IDENTIFIER':
            return {'identifier': self.expect('IDENTIFIER').value}
        elif tok.type == 'STRING':
            return {'terminal': self.expect('STRING').value}
        elif tok.type == 'LBRACKET':
            return self.parse_optional()
        elif tok.type == 'LBRACE':
            return self.parse_repeat()
        elif tok.type == 'LPAREN':
            return self.parse_group()
        else:
            raise SyntaxError(f"Unexpected primary {tok}")

    def parse_optional(self):
        self.expect('LBRACKET')
        expr = self.parse_expression()
        self.expect('RBRACKET')
        return {'optional': expr}

    def parse_repeat(self):
        self.expect('LBRACE')
        expr = self.parse_expression()
        self.expect('RBRACE')
        return {'repeat': expr}

    def parse_group(self):
        self.expect('LPAREN')
        expr = self.parse_expression()
        self.expect('RPAREN')
        return {'group': expr}

    # ---------------------------
    # Tokens Parsing
    # ---------------------------

    def parse_token(self):
        tok_name = self.expect('IDENTIFIER').value
        self.expect('EQUALS')
        tok_value = self.parse_token_value()
        self.expect('SEMICOLON')
        return {'token': tok_name, 'value': tok_value}

    def parse_token_value(self):
        tok = self.current()
        if tok.type in ('STRING','IDENTIFIER','INTEGER'):
            return self.expect(tok.type).value
        elif tok.type == 'HASH':
            self.expect('HASH')
            digits = self.expect('INTEGER').value
            return f'#{digits}'
        else:
            raise SyntaxError(f"Unexpected token value {tok}")

# ---------------------------
# Main
# ---------------------------

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 iso_ebnf_parser_v8.py ISO_EBNF-file")
        sys.exit(1)
    filename = sys.argv[1]
    with open(filename,'r',encoding='utf-8') as f:
        text = f.read()
    lexer = Lexer(text)
    parser = Parser(lexer)
    grammar_json = parser.parse_syntax()
    print(json.dumps(grammar_json, indent=2))

if __name__ == "__main__":
    main()
