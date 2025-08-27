#!/usr/bin/env python3
import re
from collections import namedtuple

# ---------------------------
# Token class
# ---------------------------
Token = namedtuple("Token", ["type", "value", "line", "col"])

# ---------------------------
# Token definitions
# ---------------------------
TOKEN_SPECS = [
    ("BLOCK_COMMENT", r"\(\*(?:.|\n)*?\*\)"),
    ("LINE_COMMENT",  r"//[^\n\r]*"),
    ("EQUALS",        r"="),
    ("SEMICOLON",     r";"),
    ("PIPE",          r"\|"),
    ("COMMA",         r","),
    ("MINUS",         r"-"),
    ("ASTERISK",      r"\*"),
    ("PLUS",          r"\+"),
    ("QUESTION",      r"\?"),
    ("LBRACKET",      r"\["),
    ("RBRACKET",      r"\]"),
    ("LBRACE",        r"\{"),
    ("RBRACE",        r"\}"),
    ("LPAREN",        r"\("),
    ("RPAREN",        r"\)"),
    ("DOTDOT",        r"\.\."),
    ("PERCENT",       r"%"),
    ("HEX_CHAR",      r"#[0-9A-Fa-f]{2}"),
    ("HEX_INT",       r"\$[0-9A-Fa-f]+"),
    ("UNICODE_ESCAPE", r"\\u[0-9A-Fa-f]{4}"),
    ("ESCAPE_SEQUENCE", r"\\[\\'\"ntrbf]"),
    ("BACKSLASH", r"\\"),
    ("STRING",        r'"([^"\\]|\\.)*"|\'([^\'\\]|\\.)*\''),
    ("IDENTIFIER",    r"[A-Za-z_][A-Za-z0-9_]*"),
    ("INTEGER",       r"[0-9]+"),
    ("SPACE",         r"[ \t\r\n]+"),
    ("MISMATCH",      r"."),
]

TOKEN_REGEX = "|".join(f"(?P<{name}>{regex})" for name, regex in TOKEN_SPECS)
TOKEN_PATTERN = re.compile(TOKEN_REGEX, re.MULTILINE)

# ---------------------------
# Lexer function
# ---------------------------
def tokenize(code):
    line = 1
    col  = 1
    tokens = []
    pos = 0
    while pos < len(code):
        m = TOKEN_PATTERN.match(code, pos)
        if not m:
            raise SyntaxError(f"Unexpected character {code[pos]!r} at line {line} col {col}")
        typ = m.lastgroup
        val = m.group(typ)
        if typ == "SPACE":
            line += val.count("\n")
            if "\n" in val:
                col = 1 + len(val.split("\n")[-1])
            else:
                col += len(val)
        elif typ in ("BLOCK_COMMENT", "LINE_COMMENT"):
            line += val.count("\n")
            if "\n" in val:
                col = 1 + len(val.split("\n")[-1])
            else:
                col += len(val)
        elif typ == "MISMATCH":
            raise SyntaxError(f"Unexpected character {val!r} at line {line} col {col}")
        else:
            tokens.append(Token(typ, val, line, col))
            col += len(val)
        pos = m.end()
    return tokens

# ---------------------------
# Test main
# ---------------------------
if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python3 iso_ebnf_lexer.py <file.ebnf>")
        sys.exit(1)
    with open(sys.argv[1], "r", encoding="utf-8") as f:
        code = f.read()
    tokens = tokenize(code)
    for t in tokens:
        print(t)
