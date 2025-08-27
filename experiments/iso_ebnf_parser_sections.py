#!/usr/bin/env python3
import sys
import re

# ---------------------------
# Token definition
# ---------------------------

class Token:
    def __init__(self, type_, value, line, col):
        self.type = type_
        self.value = value
        self.line = line
        self.col = col

    def __repr__(self):
        return f"Token({self.type}, {self.value}, {self.line}:{self.col})"

# ---------------------------
# Lexer
# ---------------------------

class Lexer:
    def __init__(self, text):
        self.text = text
        self.i = 0
        self.line = 1
        self.col = 1

    def peek(self, n=0):
        if self.i + n < len(self.text):
            return self.text[self.i + n]
        return ""

    def advance(self, n=1):
        for _ in range(n):
            if self.i < len(self.text):
                ch = self.text[self.i]
                self.i += 1
                if ch == "\n":
                    self.line += 1
                    self.col = 1
                else:
                    self.col += 1

    def skip_ws_and_comments(self):
        while True:
            ch = self.peek()
            if ch in " \t\r\n":
                self.advance()
                continue
            # comment (* ... *)
            if ch == "(" and self.peek(1) == "*":
                self.advance(2)
                while not (self.peek() == "*" and self.peek(1) == ")"):
                    if self.peek() == "":
                        raise SyntaxError("Unterminated comment")
                    self.advance()
                self.advance(2)  # skip closing *)
                continue
            break

    def next_token(self):
        self.skip_ws_and_comments()
        start_line, start_col = self.line, self.col
        ch = self.peek()

        if ch == "":
            return None

        # punctuation
        if ch in "=;|,(){}[]*?-\\":
            self.advance()
            return ("SYMBOL", ch, start_line, start_col)

        # hex character #00 .. #FF
        if ch == "#":
            start_line, start_col = self.line, self.col
            hexstr = "#"
            self.advance()  # skip '#'
            
            # Verifica se temos dois dígitos hexadecimais
            if not re.match(r"[0-9A-Fa-f]", self.peek()) or not re.match(r"[0-9A-Fa-f]", self.peek(1)):
                raise SyntaxError(f"Invalid hex char at {start_line}:{start_col}")
            
            # Coleta os dois dígitos
            hexstr += self.peek()
            self.advance()
            hexstr += self.peek()
            self.advance()
            
            return Token("HEXCHAR", hexstr, start_line, start_col)

        # ".." range
        if ch == "." and self.peek(1) == ".":
            self.advance(2)
            return Token("RANGE", "..", start_line, start_col)

        # integer
        if ch.isdigit():
            num = ""
            while self.peek().isdigit():
                num += self.peek()
                self.advance()
            return Token("INTEGER", num, start_line, start_col)

        # identifier
        if ch.isalpha() or ch == "_":
            ident = ""
            while re.match(r"[A-Za-z0-9_]", self.peek()):
                ident += self.peek()
                self.advance()
            return Token("IDENT", ident, start_line, start_col)

        # string literal (single or double quotes)
        if ch in "\"'":
            quote = ch
            self.advance()
            s = ""
            while True:
                c = self.peek()
                if c == "":
                    raise SyntaxError(f"Unterminated string at {start_line}:{start_col}")
                if c == "\\":
                    self.advance()
                    esc = self.peek()
                    if esc in ['"', "'", "\\", "n", "t", "r"]:
                        mapping = {"n": "\n", "t": "\t", "r": "\r"}
                        s += mapping.get(esc, esc)
                        self.advance()
                    else:
                        raise SyntaxError(f"Invalid escape \\{esc}")
                elif c == quote:
                    self.advance()
                    break
                else:
                    s += c
                    self.advance()
            return Token("STRING", s, start_line, start_col)

        raise SyntaxError(f"Unexpected character {ch!r} at {start_line}:{start_col}")

    def tokens(self):
        result = []
        while True:
            t = self.next_token()
            if not t:
                break
            result.append(t)
        return result

# ---------------------------
# Parser (simplified recursive descent)
# ---------------------------

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self):
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return None

    def expect(self, type_):
        t = self.peek()
        if not t or t.type != type_:
            raise SyntaxError(f"Expected {type_}, got {t}")
        self.pos += 1
        return t

    def accept(self, type_):
        if self.peek() and self.peek().type == type_:
            return self.expect(type_)
        return None

    # grammar = { rule | comment } ;
    def parse_grammar(self):
        rules = []
        while self.peek():
            rules.append(self.parse_rule())
        return rules

    def parse_rule(self):
        ident = self.expect("IDENT")
        self.expect("=")
        expr = self.parse_expression()
        self.expect(";")
        return ("rule", ident.value, expr)

    def parse_expression(self):
        term = self.parse_term()
        alts = [term]
        while self.accept("|"):
            alts.append(self.parse_term())
        if len(alts) == 1:
            return alts[0]
        return ("alt", alts)

    def parse_term(self):
        fac = self.parse_factor()
        seq = [fac]
        while True:
            if self.peek() and self.peek().type in ("IDENT", "STRING", "(", "[", "{", "?", "INTEGER"):
                seq.append(self.parse_factor())
            else:
                break
        if len(seq) == 1:
            return seq[0]
        return ("seq", seq)

    def parse_factor(self):
        mult = None
        if self.peek() and self.peek().type == "INTEGER":
            mult = self.expect("INTEGER").value
            self.expect("*")
        prim = self.parse_primary()
        if mult:
            return ("repeat", int(mult), prim)
        return prim

    def parse_primary(self):
        t = self.peek()
        if not t:
            raise SyntaxError("Unexpected EOF in primary")

        if t.type == "IDENT":
            return ("ident", self.expect("IDENT").value)
        if t.type == "STRING":
            return ("string", self.expect("STRING").value)
        if t.type == "(":
            self.expect("(")
            e = self.parse_expression()
            self.expect(")")
            return ("group", e)
        if t.type == "[":
            self.expect("[")
            e = self.parse_expression()
            self.expect("]")
            return ("optional", e)
        if t.type == "{":
            self.expect("{")
            e = self.parse_expression()
            self.expect("}")
            return ("repeat", e)
        if t.type == "?":
            self.expect("?")
            seq = []
            while self.peek() and self.peek().type != "?":
                seq.append(self.expect(self.peek().type).value)
            self.expect("?")
            return ("special", seq)

        raise SyntaxError(f"Unexpected token {t}")

# ---------------------------
# Main
# ---------------------------

def main():
    if len(sys.argv) != 2:
        print("Usage: iso_ebnf_parser_sections.py <file>")
        sys.exit(1)

    with open(sys.argv[1], "r", encoding="utf-8") as f:
        text = f.read()

    lexer = Lexer(text)
    tokens = lexer.tokens()

    parser = Parser(tokens)
    grammar = parser.parse_grammar()

    for rule in grammar:
        print(rule)

if __name__ == "__main__":
    main()
