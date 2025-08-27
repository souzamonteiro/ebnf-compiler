import json

class ASTNode:
    def __init__(self, type_, value=None, children=None):
        self.type = type_
        self.value = value
        self.children = children or []

    def to_dict(self):
        return {
            "type": self.type,
            "value": self.value,
            "children": [c.to_dict() for c in self.children]
        }

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def _peek(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def _match(self, expected):
        if self._peek() == expected:
            self.pos += 1
            return ASTNode("Terminal", value=expected)
        return None

    def _save(self): return self.pos
    def _restore(self, pos): self.pos = pos


    def _rule_syntax(self):
        p0 = self._save()
        children = []
        while True:
            for option in range(2):
                if option == 0:
                node = self._rule_rule()
                if node: return node
                    if node: return node
                if option == 1:
                node = self._rule_comment()
                if node: return node
                    if node: return node
            if not node: break
            children.append(node)
        return ASTNode('Repetition', children=children)
        self._restore(p0)
        return None

    def _rule_rule(self):
        p0 = self._save()
        p1 = self._save()
        children = []
        node = self._rule_identifier()
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        node = self._match('=')
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        node = self._rule_expression()
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        node = self._match(';')
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        return ASTNode('Rule', value='rule', children=children)
        self._restore(p0)
        return None

    def _rule_expression(self):
        p0 = self._save()
        p1 = self._save()
        children = []
        node = self._rule_term()
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        children = []
        while True:
            p1 = self._save()
            children = []
            node = self._match('|')
            if node:  node
            if not node: self._restore(p1);  None
            children.append(node)
            node = self._rule_term()
            if node:  node
            if not node: self._restore(p1);  None
            children.append(node)
             ASTNode('Rule', value='expression', children=children)
            if not node: break
            children.append(node)
         ASTNode('Repetition', children=children)
        if not node: self._restore(p1); return None
        children.append(node)
        return ASTNode('Rule', value='expression', children=children)
        self._restore(p0)
        return None

    def _rule_term(self):
        p0 = self._save()
        p1 = self._save()
        children = []
        node = self._rule_factor()
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        children = []
        while True:
            p1 = self._save()
            children = []
            node = self._match(',')
            if node:  node
            if not node: self._restore(p1);  None
            children.append(node)
            node = self._rule_factor()
            if node:  node
            if not node: self._restore(p1);  None
            children.append(node)
             ASTNode('Rule', value='term', children=children)
            if not node: break
            children.append(node)
         ASTNode('Repetition', children=children)
        if not node: self._restore(p1); return None
        children.append(node)
        return ASTNode('Rule', value='term', children=children)
        self._restore(p0)
        return None

    def _rule_factor(self):
        p0 = self._save()
        p1 = self._save()
        children = []
        node = None
        p1 = self._save()
        children = []
        node = self._rule_integer()
        if node:  node
        if not node: self._restore(p1);  None
        children.append(node)
        node = self._match('*')
        if node:  node
        if not node: self._restore(p1);  None
        children.append(node)
         ASTNode('Rule', value='factor', children=children)
         ASTNode('Optional', children=[node] if node else [])
        if not node: self._restore(p1); return None
        children.append(node)
        node = self._rule_primary()
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        return ASTNode('Rule', value='factor', children=children)
        self._restore(p0)
        return None

    def _rule_primary(self):
        p0 = self._save()
        for option in range(5):
            if option == 0:
            node = self._rule_identifier()
            if node: return node
                if node: return node
            if option == 1:
            node = self._rule_terminal()
            if node: return node
                if node: return node
            if option == 2:
            p1 = self._save()
            children = []
            node = self._match('(')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._rule_expression()
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._match(')')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            return ASTNode('Rule', value='primary', children=children)
                if node: return node
            if option == 3:
            p1 = self._save()
            children = []
            node = self._match('[')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._rule_expression()
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._match(']')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            return ASTNode('Rule', value='primary', children=children)
                if node: return node
            if option == 4:
            p1 = self._save()
            children = []
            node = self._match('{')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._rule_expression()
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._match('}')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            return ASTNode('Rule', value='primary', children=children)
                if node: return node
        self._restore(p0)
        return None

    def _rule_identifier(self):
        p0 = self._save()
        p1 = self._save()
        children = []
        node = self._rule_letter()
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        children = []
        while True:
            for option in range(2):
                if option == 0:
                node = self._rule_letter()
                if node:  node
                    if node:  node
                if option == 1:
                node = self._rule_digit()
                if node:  node
                    if node:  node
            if not node: break
            children.append(node)
         ASTNode('Repetition', children=children)
        if not node: self._restore(p1); return None
        children.append(node)
        return ASTNode('Rule', value='identifier', children=children)
        self._restore(p0)
        return None

    def _rule_terminal(self):
        p0 = self._save()
        for option in range(2):
            if option == 0:
            p1 = self._save()
            children = []
            node = self._match('"')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._rule_character()
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            children = []
            while True:
                node = self._rule_character()
                if node:  node
                if not node: break
                children.append(node)
             ASTNode('Repetition', children=children)
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._match('"')
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            return ASTNode('Rule', value='terminal', children=children)
                if node: return node
            if option == 1:
            p1 = self._save()
            children = []
            node = self._match("'")
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._rule_character()
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            children = []
            while True:
                node = self._rule_character()
                if node:  node
                if not node: break
                children.append(node)
             ASTNode('Repetition', children=children)
            if not node: self._restore(p1); return None
            children.append(node)
            node = self._match("'")
            if node:  node
            if not node: self._restore(p1); return None
            children.append(node)
            return ASTNode('Rule', value='terminal', children=children)
                if node: return node
        self._restore(p0)
        return None

    def _rule_character(self):
        p0 = self._save()
        for option in range(2):
            if option == 0:
            node = self._rule_letter()
            if node: return node
                if node: return node
            if option == 1:
            node = self._rule_digit()
            if node: return node
                if node: return node
        self._restore(p0)
        return None

    def _rule_integer(self):
        p0 = self._save()
        p1 = self._save()
        children = []
        node = self._rule_digit()
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        children = []
        while True:
            node = self._rule_digit()
            if node:  node
            if not node: break
            children.append(node)
         ASTNode('Repetition', children=children)
        if not node: self._restore(p1); return None
        children.append(node)
        return ASTNode('Rule', value='integer', children=children)
        self._restore(p0)
        return None

    def _rule_letter(self):
        p0 = self._save()
        for option in range(52):
            if option == 0:
            node = self._match('A')
            if node: return node
                if node: return node
            if option == 1:
            node = self._match('B')
            if node: return node
                if node: return node
            if option == 2:
            node = self._match('C')
            if node: return node
                if node: return node
            if option == 3:
            node = self._match('D')
            if node: return node
                if node: return node
            if option == 4:
            node = self._match('E')
            if node: return node
                if node: return node
            if option == 5:
            node = self._match('F')
            if node: return node
                if node: return node
            if option == 6:
            node = self._match('G')
            if node: return node
                if node: return node
            if option == 7:
            node = self._match('H')
            if node: return node
                if node: return node
            if option == 8:
            node = self._match('I')
            if node: return node
                if node: return node
            if option == 9:
            node = self._match('J')
            if node: return node
                if node: return node
            if option == 10:
            node = self._match('K')
            if node: return node
                if node: return node
            if option == 11:
            node = self._match('L')
            if node: return node
                if node: return node
            if option == 12:
            node = self._match('M')
            if node: return node
                if node: return node
            if option == 13:
            node = self._match('N')
            if node: return node
                if node: return node
            if option == 14:
            node = self._match('O')
            if node: return node
                if node: return node
            if option == 15:
            node = self._match('P')
            if node: return node
                if node: return node
            if option == 16:
            node = self._match('Q')
            if node: return node
                if node: return node
            if option == 17:
            node = self._match('R')
            if node: return node
                if node: return node
            if option == 18:
            node = self._match('S')
            if node: return node
                if node: return node
            if option == 19:
            node = self._match('T')
            if node: return node
                if node: return node
            if option == 20:
            node = self._match('U')
            if node: return node
                if node: return node
            if option == 21:
            node = self._match('V')
            if node: return node
                if node: return node
            if option == 22:
            node = self._match('W')
            if node: return node
                if node: return node
            if option == 23:
            node = self._match('X')
            if node: return node
                if node: return node
            if option == 24:
            node = self._match('Y')
            if node: return node
                if node: return node
            if option == 25:
            node = self._match('Z')
            if node: return node
                if node: return node
            if option == 26:
            node = self._match('a')
            if node: return node
                if node: return node
            if option == 27:
            node = self._match('b')
            if node: return node
                if node: return node
            if option == 28:
            node = self._match('c')
            if node: return node
                if node: return node
            if option == 29:
            node = self._match('d')
            if node: return node
                if node: return node
            if option == 30:
            node = self._match('e')
            if node: return node
                if node: return node
            if option == 31:
            node = self._match('f')
            if node: return node
                if node: return node
            if option == 32:
            node = self._match('g')
            if node: return node
                if node: return node
            if option == 33:
            node = self._match('h')
            if node: return node
                if node: return node
            if option == 34:
            node = self._match('i')
            if node: return node
                if node: return node
            if option == 35:
            node = self._match('j')
            if node: return node
                if node: return node
            if option == 36:
            node = self._match('k')
            if node: return node
                if node: return node
            if option == 37:
            node = self._match('l')
            if node: return node
                if node: return node
            if option == 38:
            node = self._match('m')
            if node: return node
                if node: return node
            if option == 39:
            node = self._match('n')
            if node: return node
                if node: return node
            if option == 40:
            node = self._match('o')
            if node: return node
                if node: return node
            if option == 41:
            node = self._match('p')
            if node: return node
                if node: return node
            if option == 42:
            node = self._match('q')
            if node: return node
                if node: return node
            if option == 43:
            node = self._match('r')
            if node: return node
                if node: return node
            if option == 44:
            node = self._match('s')
            if node: return node
                if node: return node
            if option == 45:
            node = self._match('t')
            if node: return node
                if node: return node
            if option == 46:
            node = self._match('u')
            if node: return node
                if node: return node
            if option == 47:
            node = self._match('v')
            if node: return node
                if node: return node
            if option == 48:
            node = self._match('w')
            if node: return node
                if node: return node
            if option == 49:
            node = self._match('x')
            if node: return node
                if node: return node
            if option == 50:
            node = self._match('y')
            if node: return node
                if node: return node
            if option == 51:
            node = self._match('z')
            if node: return node
                if node: return node
        self._restore(p0)
        return None

    def _rule_digit(self):
        p0 = self._save()
        for option in range(10):
            if option == 0:
            node = self._match('0')
            if node: return node
                if node: return node
            if option == 1:
            node = self._match('1')
            if node: return node
                if node: return node
            if option == 2:
            node = self._match('2')
            if node: return node
                if node: return node
            if option == 3:
            node = self._match('3')
            if node: return node
                if node: return node
            if option == 4:
            node = self._match('4')
            if node: return node
                if node: return node
            if option == 5:
            node = self._match('5')
            if node: return node
                if node: return node
            if option == 6:
            node = self._match('6')
            if node: return node
                if node: return node
            if option == 7:
            node = self._match('7')
            if node: return node
                if node: return node
            if option == 8:
            node = self._match('8')
            if node: return node
                if node: return node
            if option == 9:
            node = self._match('9')
            if node: return node
                if node: return node
        self._restore(p0)
        return None

    def _rule_comment(self):
        p0 = self._save()
        p1 = self._save()
        children = []
        node = self._match('(*')
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        children = []
        while True:
            for option in range(2):
                if option == 0:
                node = self._rule_character()
                if node:  node
                    if node:  node
                if option == 1:
                node = self._rule_space()
                if node:  node
                    if node:  node
            if not node: break
            children.append(node)
         ASTNode('Repetition', children=children)
        if not node: self._restore(p1); return None
        children.append(node)
        node = self._match('*)')
        if node:  node
        if not node: self._restore(p1); return None
        children.append(node)
        return ASTNode('Rule', value='comment', children=children)
        self._restore(p0)
        return None

    def _rule_space(self):
        p0 = self._save()
        for option in range(4):
            if option == 0:
            node = self._match(' ')
            if node: return node
                if node: return node
            if option == 1:
            node = self._match('\\t')
            if node: return node
                if node: return node
            if option == 2:
            node = self._match('\\n')
            if node: return node
                if node: return node
            if option == 3:
            node = self._match('\\r')
            if node: return node
                if node: return node
        self._restore(p0)
        return None

def parse(tokens):
    parser = Parser(tokens)
    node = parser._rule_syntax()
    if parser.pos != len(tokens):
        raise SyntaxError(f"Extra tokens at {parser.pos}: {parser.tokens[parser.pos:]}")
    return node
