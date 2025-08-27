# iso_ebnf_parser_clean.py
# ISO EBNF → JSON AST
# -------------------------------------------------------------
# Features
# - Rules:            identifier '=' expression ';'   (also supports '::=')
# - Expression:       alternatives with '|', sequence with ','
# - Primaries:        identifier | "string" | 'string' | '(' expr ')' | '[' expr ']' | '{' expr '}'
# - Comments:         (* ... *) with proper nesting
# - n* repetition:    INTEGER '*' Primary   (common extension)
# - CharRange:        "A" .. "Z" / '0' .. '9'
#
# Output
# - A single JSON object:
#   { "Grammar": [ { "Rule": { "name": <id>, "expr": <node> } }, ... ] }
#
# Node shapes
# - Identifier:   { "Identifier": "name" }
# - Terminal:     { "Terminal": "text" }          # quotes removed
# - Sequence:     { "Sequence": [ <node>, ... ] }
# - Choice:       { "Choice": [ <node>, ... ] }
# - Optional:     { "Optional": <node> }
# - Repetition:   { "Repetition": <node> }
# - Repeat n*:    { "Repeat": { "count": n, "node": <primary> } }
# - Group:        { "Group": <node> }
# - CharRange:    { "CharRange": { "from": "A", "to": "Z" } }
#
# Limitations
# - No semantic validation (undefined identifiers, left recursion, etc.).
# - String literals are taken verbatim (no escape sequences beyond basic quotes).
# - ISO “special sequence” ? ... ? is not implemented.
# - Whitespace is skipped everywhere; commas are sequence separators (ISO 14977 style).
#
# Usage
#   python3 iso_ebnf_parser_clean.py <file.ebnf>
#
# Example
#   program   = "PROGRAM", identifier, ";", block, "." ;
#   letter    = "A" .. "Z" | "a" .. "z" ;
#   digit     = "0" .. "9" ;
#
# -------------------------------------------------------------

import sys
import json

# =========================
# Token and Lexer
# =========================

class Token:
    def __init__(self, typ, val, pos):
        self.type = typ   # "IDENT", "INTEGER", "STRING", "SYMBOL", "EOF"
        self.value = val
        self.pos = pos
    def __repr__(self):
        return f"Token({self.type}, {self.value!r}, {self.pos})"

class Lexer:
    """
    Manual scanner with:
    - nested (* ... *) comments
    - longest-match for symbols: '::=', ':=', '..'
    - strings in single or double quotes (no escapes inside, but '' or "" end strictly)
    """
    def __init__(self, text):
        self.text = text
        self.n = len(text)
        self.i = 0

    def _peek(self, k=0):
        j = self.i + k
        return self.text[j] if j < self.n else None

    def _advance(self, k=1):
        self.i += k

    def _is_ident_start(self, c):
        return c.isalpha() or c == '_'

    def _is_ident_cont(self, c):
        return c.isalnum() or c == '_'

    def _skip_ws_and_comments(self):
        while True:
            advanced = False
            # whitespace
            while self._peek() is not None and self._peek().isspace():
                self._advance()
                advanced = True
            # nested comments (* ... *)
            if self._peek() == '(' and self._peek(1) == '*':
                advanced = True
                self._advance(2)
                depth = 1
                while depth > 0:
                    if self._peek() is None:
                        raise SyntaxError("Unclosed comment")
                    if self._peek() == '(' and self._peek(1) == '*':
                        self._advance(2); depth += 1
                    elif self._peek() == '*' and self._peek(1) == ')':
                        self._advance(2); depth -= 1
                    else:
                        self._advance(1)
            if not advanced:
                break

    def _read_string(self):
        quote = self._peek()
        pos0 = self.i
        self._advance(1)  # consume opening quote
        start = self.i
        while True:
            c = self._peek()
            if c is None:
                raise SyntaxError(f"Unterminated string starting at {pos0}")
            if c == quote:
                s = self.text[start:self.i]  # content without quotes
                self._advance(1)             # consume closing quote
                return Token("STRING", s, pos0)
            # allow newlines in strings (as the C version did)
            self._advance(1)

    def _read_ident_or_integer(self):
        pos0 = self.i
        c0 = self._peek()
        if c0 is None:
            return None
        if c0.isdigit():
            # INTEGER
            while self._peek() is not None and self._peek().isdigit():
                self._advance(1)
            return Token("INTEGER", self.text[pos0:self.i], pos0)
        if self._is_ident_start(c0):
            self._advance(1)
            while self._peek() is not None and self._is_ident_cont(self._peek()):
                self._advance(1)
            return Token("IDENT", self.text[pos0:self.i], pos0)
        return None

    def _read_symbol(self):
        # Longest match first
        pos0 = self.i
        two = (self._peek(), self._peek(1), self._peek(2))
        tri = "".join(ch for ch in two if ch is not None)
        # Try 3-char first
        if self._peek() == ':' and self._peek(1) == ':' and self._peek(2) == '=':
            self._advance(3); return Token("SYMBOL", "::=", pos0)
        # Try 2-char
        if self._peek() == ':' and self._peek(1) == '=':
            self._advance(2); return Token("SYMBOL", ":=", pos0)
        if self._peek() == '.' and self._peek(1) == '.':
            self._advance(2); return Token("SYMBOL", "..", pos0)
        # Single char symbols
        s1 = self._peek()
        if s1 in "=|,{}[]();.*":
            self._advance(1)
            return Token("SYMBOL", s1, pos0)
        return None

    def tokens(self):
        toks = []
        while True:
            self._skip_ws_and_comments()
            if self.i >= self.n:
                break
            pos0 = self.i
            c = self._peek()

            if c in ("'", '"'):
                toks.append(self._read_string()); continue

            t = self._read_ident_or_integer()
            if t is not None:
                toks.append(t); continue

            s = self._read_symbol()
            if s is not None:
                toks.append(s); continue

            # If we reach here, it's an unexpected character
            bad = self._peek()
            raise SyntaxError(f"Unexpected character {bad!r} at {pos0}")

        toks.append(Token("EOF", "", self.i))
        return toks


# =========================
# Parser
# =========================

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def _peek(self):
        return self.tokens[self.pos]

    def _consume(self, expected_type=None, expected_value=None):
        tok = self._peek()
        if expected_type and tok.type != expected_type:
            raise SyntaxError(f"Expected {expected_type}, got {tok.type} at {tok.pos}")
        if expected_value and tok.value != expected_value:
            raise SyntaxError(f"Expected {expected_value}, got {tok.value} at {tok.pos}")
        self.pos += 1
        return tok

    # Grammar ::= { Rule } EOF
    def parse_grammar(self):
        rules = []
        while self._peek().type != "EOF":
            rules.append(self.parse_rule())
        return {"Grammar": rules}

    # Rule ::= IDENT ( '=' | '::=' ) Expression ';'
    def parse_rule(self):
        name = self._consume("IDENT").value
        if self._peek().type == "SYMBOL" and self._peek().value in ("=", "::="):
            self._consume("SYMBOL")  # accept '=' or '::='
        else:
            raise SyntaxError(f"Expected '=' or '::=' after rule name '{name}' at {self._peek().pos}")
        expr = self.parse_expression()
        self._consume("SYMBOL", ";")
        return {"Rule": {"name": name, "expr": expr}}

    # Expression ::= Term { '|' Term }
    def parse_expression(self):
        terms = [self.parse_term()]
        while self._peek().type == "SYMBOL" and self._peek().value == "|":
            self._consume("SYMBOL", "|")
            terms.append(self.parse_term())
        if len(terms) == 1:
            return terms[0]
        return {"Choice": terms}

    # Term ::= Factor { ',' Factor }
    def parse_term(self):
        factors = [self.parse_factor()]
        while self._peek().type == "SYMBOL" and self._peek().value == ",":
            self._consume("SYMBOL", ",")
            factors.append(self.parse_factor())
        if len(factors) == 1:
            return factors[0]
        return {"Sequence": factors}

    # Factor ::= [ INTEGER '*' ] Primary
    def parse_factor(self):
        repeat_count = None
        if self._peek().type == "INTEGER":
            # Lookahead for '*'
            num_tok = self._consume("INTEGER")
            if self._peek().type == "SYMBOL" and self._peek().value == "*":
                self._consume("SYMBOL", "*")
                repeat_count = int(num_tok.value)
            else:
                # Not a repetition, roll back (we consumed INTEGER that belongs to Primary? ISO doesn't use
                # bare integers as primaries, so safest is to treat this as an error rather than rollback silently).
                # But to be user-friendly, we "unconsume" by stepping back one token if not repetition.
                self.pos -= 1

        primary = self.parse_primary()
        if repeat_count is not None:
            return {"Repeat": {"count": repeat_count, "node": primary}}
        return primary

    # Primary ::= IDENT
    #           | STRING
    #           | '(' Expression ')'
    #           | '[' Expression ']'
    #           | '{' Expression '}'
    #           | STRING '..' STRING         (CharRange)
    def parse_primary(self):
        tok = self._peek()

        # Identifier
        if tok.type == "IDENT":
            return {"Identifier": self._consume().value}

        # String or CharRange
        if tok.type == "STRING":
            s1 = self._consume("STRING").value
            # Check for range
            if self._peek().type == "SYMBOL" and self._peek().value == "..":
                self._consume("SYMBOL", "..")
                s2 = self._consume("STRING").value
                return {"CharRange": {"from": s1, "to": s2}}
            return {"Terminal": s1}

        # Parenthesized / Optional / Repetition
        if tok.type == "SYMBOL":
            if tok.value == "(":
                self._consume("SYMBOL", "(")
                expr = self.parse_expression()
                self._consume("SYMBOL", ")")
                return {"Group": expr}
            if tok.value == "[":
                self._consume("SYMBOL", "[")
                expr = self.parse_expression()
                self._consume("SYMBOL", "]")
                return {"Optional": expr}
            if tok.value == "{":
                self._consume("SYMBOL", "{")
                expr = self.parse_expression()
                self._consume("SYMBOL", "}")
                return {"Repetition": expr}

        raise SyntaxError(f"Unexpected token {tok} at {tok.pos}")


# =========================
# Main
# =========================

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 iso_ebnf_parser_clean.py <file.ebnf>")
        sys.exit(1)

    with open(sys.argv[1], "r", encoding="utf-8") as f:
        text = f.read()

    lexer = Lexer(text)
    tokens = lexer.tokens()
    parser = Parser(tokens)
    ast = parser.parse_grammar()

    print(json.dumps(ast, indent=2, ensure_ascii=False))

if __name__ == "__main__":
    main()
