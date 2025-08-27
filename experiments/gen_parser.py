import json
import sys

class ParserGenerator:
    def __init__(self, grammar):
        # Expecting grammar format:
        # {
        #   "Grammar": [
        #       {"Rule": {"name": "identifier", "expr": {...}}},
        #       {"Rule": {"name": "number", "expr": {...}}}
        #   ]
        # }
        self.rules = {
            rule_obj["Rule"]["name"]: rule_obj["Rule"]["expr"]
            for rule_obj in grammar["Grammar"]
        }

    def generate(self):
        parts = []
        parts.append("import json\n")
        parts.append("class ASTNode:\n")
        parts.append("    def __init__(self, type, value=None, children=None):\n")
        parts.append("        self.type = type\n")
        parts.append("        self.value = value\n")
        parts.append("        self.children = children or []\n")
        parts.append("    def to_dict(self):\n")
        parts.append("        return {\n")
        parts.append("            'type': self.type,\n")
        parts.append("            'value': self.value,\n")
        parts.append("            'children': [c.to_dict() for c in self.children]\n")
        parts.append("        }\n\n")

        parts.append("class Parser:\n")
        parts.append("    def __init__(self, tokens):\n")
        parts.append("        self.tokens = tokens\n")
        parts.append("        self.pos = 0\n\n")
        parts.append("    def _peek(self):\n")
        parts.append("        return self.tokens[self.pos] if self.pos < len(self.tokens) else None\n\n")
        parts.append("    def _match(self, expected_type, expected_value=None):\n")
        parts.append("        tok = self._peek()\n")
        parts.append("        if tok and tok['type'] == expected_type and (expected_value is None or tok['value'] == expected_value):\n")
        parts.append("            self.pos += 1\n")
        parts.append("            return ASTNode('Terminal', value=tok['value'])\n")
        parts.append("        raise SyntaxError(f\"Expected {expected_type} {expected_value}, got {tok}\")\n\n")
        parts.append("    def _save(self): return self.pos\n")
        parts.append("    def _restore(self, pos): self.pos = pos\n\n")

        # Generate rule methods
        for name, expr in self.rules.items():
            parts.append(self._gen_rule_method(name, expr))

        # Add main entry
        start_rule = list(self.rules.keys())[0]
        parts.append(f"    def parse(self):\n")
        parts.append(f"        return self._rule_{start_rule}()\n\n")

        # Main execution wrapper
        parts.append("if __name__ == '__main__':\n")
        parts.append("    import sys, json\n")
        parts.append("    with open(sys.argv[1]) as f:\n")
        parts.append("        tokens = json.load(f)\n")
        parts.append("    parser = Parser(tokens)\n")
        parts.append("    ast = parser.parse()\n")
        parts.append("    print(json.dumps(ast.to_dict(), indent=2))\n")

        return "".join(parts)

    def _gen_rule_method(self, name, expr):
        code = []
        code.append(f"    def _rule_{name}(self):\n")
        code.append(f"        p0 = self._save()\n")
        code.append(f"        try:\n")
        body = self._gen_expr(expr, indent="            ")
        code.extend(body)
        code.append(f"        except SyntaxError:\n")
        code.append(f"            self._restore(p0)\n")
        code.append(f"            raise SyntaxError(f\"Failed to parse rule '{name}' at pos {{self.pos}}\")\n")
        code.append("\n")
        return "".join(code)

    def _gen_expr(self, expr, indent="        "):
        etype = expr["type"]
        if etype == "Terminal":
            return [f"{indent}node = self._match('TERMINAL', {repr(expr['value'])})\n",
                    f"{indent}return ASTNode('Terminal', value=node.value)\n"]
        elif etype == "Identifier":
            return [f"{indent}child = self._rule_{expr['value']}()\n",
                    f"{indent}return ASTNode('NonTerminal', value={repr(expr['value'])}, children=[child])\n"]
        elif etype == "Sequence":
            code = [f"{indent}children = []\n"]
            for sub in expr["value"]:
                code.append(f"{indent}children.append(self._gen_wrap_{sub['type']}(lambda: self._subexpr_{id(sub)}()))\n")
            code.append(f"{indent}return ASTNode('Sequence', children=children)\n\n")
            # generate helper subexprs
            self._inject_subexpr(sub)
            return code
        elif etype == "Choice":
            code = [f"{indent}p0 = self._save()\n"]
            first = True
            for sub in expr["value"]:
                if not first:
                    code.append(f"{indent}self._restore(p0)\n")
                code.append(f"{indent}try:\n")
                code.append(f"{indent}    return self._subexpr_{id(sub)}()\n")
                code.append(f"{indent}except SyntaxError:\n")
                code.append(f"{indent}    pass\n")
                self._inject_subexpr(sub)
                first = False
            code.append(f"{indent}raise SyntaxError('No choice matched')\n")
            return code
        else:
            return [f"{indent}# TODO implement {etype}\n",
                    f"{indent}return ASTNode('{etype}')\n"]

    def _inject_subexpr(self, sub):
        """Generate a helper method for a subexpression if not already created"""
        # This could track generated ids to avoid duplicates
        pass


if __name__ == "__main__":
    grammar_file = sys.argv[1]
    with open(grammar_file) as f:
        grammar = json.load(f)
    gen = ParserGenerator(grammar)
    code = gen.generate()
    with open("parser_generated.py", "w") as f:
        f.write(code)
