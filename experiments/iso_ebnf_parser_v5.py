#!/usr/bin/env python3
import re
import sys
import json

# ---------------------------
# Token definition
# ---------------------------

TOKEN_SPECS = [
    ("BLOCK_COMMENT", r"\(\*.*?\*\)"),
    ("LINE_COMMENT", r"//[^\n\r]*"),
    ("EQUALS", r"="),
    ("SEMICOLON", r";"),
    ("PIPE", r"\|"),
    ("COMMA", r","),
    ("MINUS", r"-"),
    ("ASTERISK", r"\*"),
    ("PLUS", r"\+"),
    ("QUESTION", r"\?"),
    ("LBRACKET", r"\["),
    ("RBRACKET", r"\]"),
    ("LBRACE", r"\{"),
    ("RBRACE", r"\}"),
    ("LPAREN", r"\("),
    ("RPAREN", r"\)"),
    ("DOTDOT", r"\.\."),
    ("PERCENT", r"%"),
    ("HASH", r"#"),
    ("HEX_CHAR", r"#[0-9A-Fa-f]{2}"),
    ("HEX_INT", r"\$[0-9A-Fa-f]+"),
    ("UNICODE_ESCAPE", r"\\u[0-9A-Fa-f]{4}"),
    ("ESCAPE_SEQUENCE", r"\\[\\'\"ntrbf]"),
    ("STRING", r"\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*'"),
    ("IDENTIFIER", r"[A-Za-z_][A-Za-z0-9_]*"),
    ("INTEGER", r"[0-9]+"),
    ("SPACE", r"[ \t\r\n]+"),
    ("BACKSLASH", r"\\"),
    ("MISMATCH", r".")
]

TOK_REGEX = "|".join("(?P<%s>%s)" % pair for pair in TOKEN_SPECS)
token_re = re.compile(TOK_REGEX, re.DOTALL)

class Token:
    def __init__(self, typ, val, line, col):
        self.type = typ
        self.value = val
        self.line = line
        self.col = col
    def __repr__(self):
        return f"Token({self.type}, {self.value}, {self.line}:{self.col})"

def tokenize(code):
    line = 1
    col = 1
    tokens = []
    for m in token_re.finditer(code):
        typ = m.lastgroup
        val = m.group()
        if typ == "SPACE" or typ == "BLOCK_COMMENT" or typ == "LINE_COMMENT":
            # skip whitespace and comments
            line += val.count("\n")
            continue
        elif typ == "MISMATCH":
            raise SyntaxError(f"Unexpected character {val!r} at line {line} col {col}")
        tokens.append(Token(typ, val, line, col))
        line += val.count("\n")
    return tokens

# ---------------------------
# Recursive descent parser
# ---------------------------

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def expect(self, typ):
        tok = self.peek()
        if tok is None or tok.type != typ:
            raise SyntaxError(f"Expected {typ} at pos {self.pos}, got {tok}")
        self.pos += 1
        return tok

    def accept(self, typ):
        if self.peek() and self.peek().type == typ:
            return self.expect(typ)
        return None

    # -------------------------
    # Grammar rules
    # -------------------------

    def parse_syntax(self):
        rules = []
        while self.peek():
            if self.peek().type == "IDENTIFIER":
                rules.append(self.parse_rule())
            elif self.peek().type == "PERCENT":
                rules.append(self.parse_directive())
            else:
                self.pos += 1  # skip unexpected
        return {"rules": rules}

    def parse_rule(self):
        ident = self.expect("IDENTIFIER").value
        self.expect("EQUALS")
        expr = self.parse_expression()
        self.expect("SEMICOLON")
        return {"type": "rule", "name": ident, "expr": expr}

    def parse_expression(self):
        terms = [self.parse_term()]
        while self.accept("PIPE"):
            terms.append(self.parse_term())
        if len(terms) == 1:
            return terms[0]
        return {"type": "choice", "options": terms}

    def parse_term(self):
        factors = [self.parse_factor()]
        while self.accept("COMMA"):
            factors.append(self.parse_factor())
        if len(factors) == 1:
            return factors[0]
        return {"type": "sequence", "factors": factors}

    def parse_factor(self):
        quant = None
        if self.accept("ASTERISK"):
            quant = {"min": 0, "max": None}
        elif self.accept("PLUS"):
            quant = {"min": 1, "max": None}
        elif self.accept("QUESTION"):
            quant = {"min": 0, "max": 1}
        prim = self.parse_primary()
        if quant:
            return {"type": "quantified", "quant": quant, "expr": prim}
        return prim

    def parse_primary(self):
        tok = self.peek()
        if tok.type == "IDENTIFIER":
            return {"type": "nonterminal", "name": self.expect("IDENTIFIER").value}
        elif tok.type == "STRING":
            return {"type": "terminal", "value": self.expect("STRING").value}
        elif tok.type == "LPAREN":
            self.expect("LPAREN")
            expr = self.parse_expression()
            self.expect("RPAREN")
            return {"type": "group", "expr": expr}
        elif tok.type == "LBRACKET":
            self.expect("LBRACKET")
            expr = self.parse_expression()
            self.expect("RBRACKET")
            return {"type": "optional", "expr": expr}
        elif tok.type == "LBRACE":
            self.expect("LBRACE")
            expr = self.parse_expression()
            self.expect("RBRACE")
            return {"type": "repeat", "expr": expr}
        elif tok.type == "HEX_CHAR":
            return {"type": "hex_char", "value": self.expect("HEX_CHAR").value}
        elif tok.type == "INTEGER":
            return {"type": "integer", "value": self.expect("INTEGER").value}
        raise SyntaxError(f"Unexpected token {tok}")

    def parse_directive(self):
        self.expect("PERCENT")
        name = self.expect("IDENTIFIER").value
        self.expect("PERCENT")
        return {"type": "directive", "name": name}

# ---------------------------
# Main driver
# ---------------------------

def main():
    if len(sys.argv) != 2:
        print("Usage: ebnf2json.py input.ebnf")
        sys.exit(1)
    with open(sys.argv[1], "r", encoding="utf-8") as f:
        code = f.read()
    tokens = tokenize(code)
    parser = Parser(tokens)
    grammar = parser.parse_syntax()
    print(json.dumps(grammar, indent=2))

if __name__ == "__main__":
    main()
