# Extended EBNF Grammar Documentation

## Overview
This document describes an extended EBNF (Extended Backus-Naur Form) grammar that supports directives, hexadecimal character ranges, and additional features beyond standard ISO EBNF.

## Grammar Structure

### Syntax Rule
```
syntax = { (rule | comment | directive | space) } ;
```
The overall syntax consists of zero or more repetitions of rules, comments, directives, or whitespace.

## Rule Definitions

### Rule Structure
```
rule = identifier, spaces, equals, spaces, expression, spaces, semicolon ;
```
Each rule consists of:
- An identifier
- An equals sign
- An expression
- A terminating semicolon

### Expressions and Terms
```
expression = term, { spaces, pipe, spaces, term } ;
term       = factor, { spaces, comma, spaces, factor } ;
```
- Expressions are sequences of terms separated by pipes (|) representing alternatives
- Terms are sequences of factors separated by commas representing concatenation

### Factors and Quantifiers
```
factor = [ integer, asterisk | quantifier ], primary ;
quantifier = asterisk | plus | question | l_brace, spaces, integer, [ comma, spaces, [ integer ] ], spaces, r_brace ;
```
Factors can include:
- Optional repetition count (integer followed by asterisk)
- Quantifiers: *, +, ?, or {n,m} range notation
- A primary element

### Primary Elements
```
primary = identifier
        | terminal
        | optional
        | repeat
        | group
        | special_sequence
        | char_range
        | hex_char ;
```
Primary elements can be:
- Identifiers (references to other rules)
- Terminal strings (quoted)
- Optional expressions [...]
- Repeat expressions {...}
- Group expressions (...)
- Special sequences ?...?
- Character ranges #xx..#yy
- Hexadecimal characters #xx

### Structural Elements
```
optional = l_bracket, spaces, expression, spaces, r_bracket ;
repeat   = l_brace, spaces, expression, spaces, r_brace ;
group    = l_paren, spaces, expression, spaces, r_paren ;
```
- Optional: [expression]
- Repeat: {expression} 
- Group: (expression)

### Special Sequences
```
special_sequence = question, { character_set }, question ;
character_set = character_element, { spaces, minus, spaces, character_element } ;
character_element = character | question ;
```
Special sequences are enclosed in question marks and can contain character sets with ranges.

### Directives
```
directive = percent, identifier, [ l_paren, literal, r_paren ], percent ;
```
Directives are enclosed in percent signs and can include an optional parameter in parentheses.

## Token Definitions

### Literals
```
literal = string | identifier | integer | hex_char ;
```
Literals can be strings, identifiers, integers, or hexadecimal characters.

### Identifiers
```
identifier = (letter | "_"), { letter | digit | "_" } ;
```
Identifiers start with a letter or underscore, followed by letters, digits, or underscores.

### Terminals and Strings
```
terminal = '"' , { string_char }, '"' 
         | "'" , { string_char }, "'" ;

string_char = character - ('"' | "'") 
            | escape_sequence
            | unicode_escape ;

escape_sequence = "\\", ( "\\" | "'" | '"' | "n" | "t" | "r" | "b" | "f" ) ;
unicode_escape  = "\u", hex_digit, hex_digit, hex_digit, hex_digit ;
```
Strings can be enclosed in single or double quotes and support:
- Standard escape sequences: \\, \', \", \n, \t, \r, \b, \f
- Unicode escape sequences: \uXXXX

### Comments
```
comment       = block_comment | line_comment ;
block_comment = "(*", { comment_char | block_comment }, "*)" ;
line_comment  = "//", { character - (#0A | #0D) }, ( #0A | #0D ) ;

comment_char  = character - "*" | "*", character - ")" ;
```
- Block comments: (* nested comments supported *)
- Line comments: // until end of line

### Character Definitions
```
character = ascii | unicode ;
ascii     = #00 .. #7F ;
unicode   = #80 .. #FF
          | #0100 .. #FFFF
          | #010000 .. #10FFFF ;
```
Characters are defined using hexadecimal ranges covering:
- ASCII: #00 to #7F
- Unicode: #80 to #10FFFFFF (full Unicode range)

### Numeric Values
```
integer = digit, { digit } ;
hex_char = hash, { hex_digit } ;
char_range = hex_char, spaces, dot_dot, spaces, hex_char ;
hex_digit  = "0" .. "9" | "A" .. "F" | "a" .. "f" ;
```
- Integers: decimal digits
- Hexadecimal characters: # followed by hex digits
- Character ranges: #xx..#yy
- Hex digits: 0-9, A-F, a-f

### Whitespace
```
spaces = space, { space } ;
space  = #20 | #09 | #0A | #0D ;
```
Whitespace includes spaces, tabs, and newlines.

## Terminal Symbols
```
equals     = "=" ;
semicolon  = ";" ;
pipe       = "|" ;
comma      = "," ;
asterisk   = "*" ;
plus       = "+" ;
question   = "?" ;
minus      = "-" ;
l_bracket  = "[" ;
r_bracket  = "]" ;
l_brace    = "{" ;
r_brace    = "}" ;
l_paren    = "(" ;
r_paren    = ")" ;
dot_dot    = ".." ;
hash       = "#" ;
percent    = "%" ;
```

## Key Features

1. **Directives**: Special constructs enclosed in % symbols
2. **Hexadecimal Support**: Character definitions using # notation
3. **Character Ranges**: Support for ranges like #00..#FF
4. **Unicode Support**: Full Unicode character set support
5. **Nested Comments**: Support for nested block comments
6. **Escape Sequences**: Comprehensive escape sequence support
7. **Quantifiers**: Extended quantifier syntax including {n,m} ranges

## Example Usage
```
%RULES%

syntax = { (rule | comment | directive | space) } ;

rule = identifier, spaces, equals, spaces, expression, spaces, semicolon ;

expression = term, { spaces, pipe, spaces, term } ;
term       = factor, { spaces, comma, spaces, factor } ;

factor = [ integer, asterisk | quantifier ], primary ;

quantifier = asterisk 
           | plus 
           | question 
           | l_brace, spaces, integer, [ comma, spaces, [ integer ] ], spaces, r_brace ;

primary = identifier
        | terminal
        | optional
        | repeat
        | group
        | special_sequence
        | char_range
        | hex_char ;

optional = l_bracket, spaces, expression, spaces, r_bracket ;
repeat   = l_brace, spaces, expression, spaces, r_brace ;
group    = l_paren, spaces, expression, spaces, r_paren ;

special_sequence = question, { character_set }, question ;

character_set = character_element, { spaces, minus, spaces, character_element } ;
character_element = character | question ;

directive = percent, identifier, [ l_paren, literal, r_paren ], percent ;

%TOKENS%

whitespace = space ;

equals     = "=" ;
semicolon  = ";" ;
pipe       = "|" ;
comma      = "," ;
asterisk   = "*" ;
plus       = "+" ;
question   = "?" ;
minus      = "-" ;
l_bracket  = "[" ;
r_bracket  = "]" ;
l_brace    = "{" ;
r_brace    = "}" ;
l_paren    = "(" ;
r_paren    = ")" ;
dot_dot    = ".." ;
hash       = "#" ;
percent    = "%" ;

literal = string | identifier | integer | hex_char ;

identifier = (letter | "_"), { letter | digit | "_" } ;

terminal = '"' , { string_char }, '"' 
         | "'" , { string_char }, "'" ;

string_char = character - ('"' | "'") 
            | escape_sequence
            | unicode_escape ;

escape_sequence = "\\", ( "\\" | "'" | '"' | "n" | "t" | "r" | "b" | "f" ) ;
unicode_escape  = "\u", hex_digit, hex_digit, hex_digit, hex_digit ;

comment       = block_comment | line_comment ;
block_comment = "(*", { comment_char | block_comment }, "*)" ;
line_comment  = "//", { character - (#0A | #0D) }, ( #0A | #0D ) ;

comment_char  = character - "*" | "*", character - ")" ;

character = ascii | unicode ;
ascii     = #00 .. #7F ;
unicode   = #80 .. #FF
          | #0100 .. #FFFF
          | #010000 .. #10FFFF ;

integer = digit, { digit } ;
letter  = "A" .. "Z" | "a" .. "z" ;
digit   = "0" .. "9" ;
hex_char = hash, { hex_digit } ;
char_range = hex_char, spaces, dot_dot, spaces, hex_char ;
hex_digit  = "0" .. "9" | "A" .. "F" | "a" .. "f" ;

spaces = space, { space } ;
space  = #20 | #09 | #0A | #0D ;
```