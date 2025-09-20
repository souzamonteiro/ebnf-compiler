---
marp: true
theme: default
paginate: true
header: '**Introduction to Bison and Lex**'
footer: 'Building a C to XML Parser'
---

# **Introduction to Bison and Lex**
### Building a C to XML Parser

**Instructor:** Roberto Luiz Souza Monteiro
**Course Duration:** 60 Minutes.

---

## **What You Will Learn Today**

By the end of this 60-minute session, you will:
- Understand the roles of **Lex** (Flex) and **Yacc** (Bison).
- Know the basic structure of `.l` (Flex) and `.y` (Bison) files.
- Be able to build a simple **lexer** and **parser**.
- Have a foundational codebase to parse a subset of C and output **XML**.

---

## **The Big Picture: What Are We Building?**

**Goal:** A program that reads C code and outputs its structure as XML.

![alt text](fig-01.svg)

---

## **Meet The Tools: Lex (or Flex)**

- **Lex:** A *lexical analyzer generator*.
- Its job is to break input text into a stream of **tokens**.
- You define tokens using **Regular Expressions**.
- Example Tokens for C: `int`, `while`, `(`, `)`, `{`, `}`, `identifier`, `number`.

**Input:**
```c
int main() {
```

**Output (Token Stream):**
`INT IDENTIFIER(main) LPAREN RPAREN LBRACE`

---

## **Meet The Tools: Bison**

- **Bison:** A *parser generator*.
- Its job is to find structure in the stream of tokens from Flex.
- You define structure using a **Context-Free Grammar**.
- It checks if the token stream follows the grammar rules of the language (e.g., C).

**Grammar Rule:**
```
function : INT IDENTIFIER '(' ')' compound_statement ;
```

---

## **How They Work Together**

1. **Bison** calls `yylex()` to get the next token.
2. **Lex** runs, matches input against its rules, and returns a token to Bison.
3. **Bison** uses these tokens to build the parse tree according to its grammar rules.
4. We insert **C code actions** in Bison to generate XML output when rules are matched.

---

## **Course Agenda**

1.  **Introduction & Overview** *(Now!)*
2.  **Hands-On with Lex:** Building the Tokenizer.
3.  **Hands-On with Bison:** Defining the Grammar.
4.  **Linking Lex and Bison:** Compiling the Parser.
5.  **Applied Project:** Creating our C → XML parser.
6.  **Live Demo & Testing**
7.  **Q&A and Next Steps**

---

## **What You Need**

- Basic knowledge of the **C** programming language.
- Familiarity with the **command line** (Linux, macOS, or WSL).
- **Tools installed:** `flex` and `bison`.
    - Ubuntu/Debian: `sudo apt install flex bison`
    - macOS: `brew install flex bison`

Let's begin!
```