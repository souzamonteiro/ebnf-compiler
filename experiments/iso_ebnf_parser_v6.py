#!/usr/bin/env python3
import sys
import json
from iso_ebnf_lexer_v6 import tokenize, Token

# -------------------------------
# Parser class for ISO EBNF
# -------------------------------
class EBNFParser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def current(self):
        # Return the current token or None if at end
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return None

    def advance(self):
        # Move to the next token
        self.pos += 1

    def expect(self, token_type):
        # Consume a token if it matches token_type, else raise SyntaxError
        tok = self.current()
        if tok and tok.type == token_type:
            self.advance()
            return tok
        raise SyntaxError(f"Expected {token_type} at pos {self.pos}, got {tok}")

    def parse_syntax(self):
        # Parse the top-level syntax: a list of rules, comments, directives, or spaces
        nodes = []
        while self.current():
            tok = self.current()
            if tok.type == 'IDENTIFIER':
                nodes.append(self.parse_rule())
            elif tok.type == 'PERCENT':
                nodes.append(self.parse_directive())
            elif tok.type in ('LBRACE', 'LPAREN', 'LBRACKET', 'STRING', 'MINUS', 'HEX_CHAR', 'ESCAPE_SEQUENCE'):
                # these can appear inside expressions but skip at top-level
                self.advance()
            else:
                # Skip spaces or comments
                self.advance()
        return nodes

    # -------------------------------
    # Rule: identifier '=' expression ';'
    # -------------------------------
    def parse_rule(self):
        rule_name = self.expect('IDENTIFIER').value
        self.expect('EQUALS')
        expr = self.parse_expression()
        self.expect('SEMICOLON')
        return {"type": "rule", "name": rule_name, "expression": expr}

    # -------------------------------
    # Expression: term { '|' term }
    # -------------------------------
    def parse_expression(self):
        terms = [self.parse_term()]
        while self.current() and self.current().type == 'PIPE':
            self.advance()
            terms.append(self.parse_term())
        if len(terms) == 1:
            return terms[0]
        return {"type": "choice", "options": terms}

    # -------------------------------
    # Term: factor { ',' factor }
    # -------------------------------
    def parse_term(self):
        factors = [self.parse_factor()]
        while self.current() and self.current().type == 'COMMA':
            self.advance()
            factors.append(self.parse_factor())
        if len(factors) == 1:
            return factors[0]
        return {"type": "sequence", "items": factors}

    # -------------------------------
    # Factor: optional quantifier + primary
    # -------------------------------
    def parse_factor(self):
        # First parse the primary element
        prim = self.parse_primary()

        # Check if a quantifier follows: *, +, ?, {n}, {n,m}
        tok = self.current()
        quant = None
        if tok and tok.type in ('ASTERISK', 'PLUS', 'QUESTION', 'LBRACE'):
            quant = self.parse_quantifier()

        # Return factor node
        if quant:
            return {"type": "factor", "quantifier": quant, "primary": prim}
        return prim


    # -------------------------------
    # Quantifiers: '*', '+', '?', '{n}', '{n,m}'
    # -------------------------------
    def parse_quantifier(self):
        tok = self.current()
        if tok.type in ('ASTERISK', 'PLUS', 'QUESTION'):
            self.advance()
            return tok.value
        elif tok.type == 'LBRACE':
            self.advance()
            min_val = int(self.expect('INTEGER').value)
            max_val = None
            if self.current() and self.current().type == 'COMMA':
                self.advance()
                if self.current().type == 'INTEGER':
                    max_val = int(self.expect('INTEGER').value)
            self.expect('RBRACE')
            return {"min": min_val, "max": max_val}
        return None

    # -------------------------------
    # Primary: identifier, terminal, optional, repeat, group, special_sequence, char_range, hex_char
    # -------------------------------
    def parse_primary(self):
        tok = self.current()
        if tok.type == 'IDENTIFIER':
            self.advance()
            return {"type": "identifier", "value": tok.value}
        elif tok.type == 'STRING':
            self.advance()
            return {"type": "terminal", "value": tok.value}
        elif tok.type == 'LBRACKET':
            return self.parse_optional()
        elif tok.type == 'LBRACE':
            return self.parse_repeat()
        elif tok.type == 'LPAREN':
            return self.parse_group()
        elif tok.type == 'QUESTION':
            return self.parse_special_sequence()
        elif tok.type == 'HEX_CHAR':
            self.advance()
            return {"type": "hex_char", "value": tok.value}
        else:
            raise SyntaxError(f"Unexpected token in primary: {tok}")

    def parse_optional(self):
        self.expect('LBRACKET')
        expr = self.parse_expression()
        self.expect('RBRACKET')
        return {"type": "optional", "expression": expr}

    def parse_repeat(self):
        self.expect('LBRACE')
        expr = self.parse_expression()
        self.expect('RBRACE')
        return {"type": "repeat", "expression": expr}

    def parse_group(self):
        self.expect('LPAREN')
        expr = self.parse_expression()
        self.expect('RPAREN')
        return {"type": "group", "expression": expr}

    def parse_special_sequence(self):
        self.expect('QUESTION')
        chars = []
        while self.current() and self.current().type != 'QUESTION':
            chars.append(self.current().value)
            self.advance()
        self.expect('QUESTION')
        return {"type": "special_sequence", "chars": chars}

    # -------------------------------
    # Directive: %IDENTIFIER[(literal)]%
    # -------------------------------
    def parse_directive(self):
        self.expect('PERCENT')
        name = self.expect('IDENTIFIER').value
        content = None
        if self.current() and self.current().type == 'LBRACKET':
            self.advance()
            content = self.parse_expression()
            self.expect('RBRACKET')
        self.expect('PERCENT')
        return {"type": "directive", "name": name, "content": content}


# -------------------------------
# Main entry
# -------------------------------
def main():
    if len(sys.argv) != 2:
        print("Usage: python3 iso_ebnf_parser_v7.py <grammar.ebnf>")
        sys.exit(1)

    filename = sys.argv[1]
    with open(filename, "r", encoding="utf-8") as f:
        code = f.read()

    tokens = tokenize(code)  # get tokens from lexer
    parser = EBNFParser(tokens)
    grammar_json = parser.parse_syntax()

    print(json.dumps(grammar_json, indent=2))


if __name__ == "__main__":
    main()
