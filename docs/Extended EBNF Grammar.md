# Extended EBNF Grammar Documentation

## Overview
This document describes an extended EBNF (Extended Backus-Naur Form) grammar that supports directives, hexadecimal character ranges, and additional features beyond standard ISO EBNF.

## Grammar Structure

### Syntax Rule
```
syntax_definition = { syntax_rule | comment } ;
```
The overall syntax consists of zero or more repetitions of syntax rules, comments, directives, or whitespace.

## Rule Definitions

### Rule Structure
```
syntax_rule = metaa_identifier, "=", definition_list, ";" ;
```
Each rule consists of:
- An identifier
- An equals sign
- An definition list
- A terminating semicolon

### Definition List and Terms
```
definition_list = single_definition, { "|", single_definition } ;
single_definition = term ,{ ",", term } ;
term = factor, { "-", exception } ;
```
- Definition list are sequences of terms separated by pipes (|) representing alternatives
- Terms are sequences of factors separated by commas representing concatenation

### Factors and Quantifiers
```
exception = factor ;
factor = [ integer, "*" ], primary ;
```
Factors can include:
- Optional repetition count (integer followed by asterisk)
- A primary element

### Primary Elements
```
primary = metaa_identifier
        | terminal_string
        | optional_sequence
        | repeted_sequence
        | grouped_sequence
        | character_definition ;
```
Primary elements can be:
- Meta identifiers (references to other rules)
- Terminal strings (quoted)
- Optional sequences [...]
- Repeated sequences {...}
- Grouped sequences (...)
- Character ranges #xx .. #yy
- Hexadecimal characters #xx

### Structural Elements
```
optional_sequence = "[", definition_list, "]" ;
repeted_sequence = "{", definition_list, "}" ;
grouped_sequence = "(", definition_list, ")" ;
```
- Optional sequences: [definition list]
- Repeated sequence: {definition list} 
- Grouped sequence: (definition list)

### Directives
Directives are enclosed in percent signs and are interpreted by the compiler.

## Token Definitions

### Terminal Strings
```
terminal_string = single_quote_terminal_string | double_quote_terminal_string ;
single_quote_terminal_string = "'", { single_quote_terminal_character }, "'" ;
double_quote_terminal_string = '"', { double_quote_terminal_character }, '"' ;

single_quote_terminal_character = character - #27 | whitespace ;
double_quote_terminal_character = character - #22 | whitespace ;
```
Terminal strings can be single quoted strings or double quoted strings.

### Meta Identifiers
```
metaa_identifier = letter, { letter | decimal_digit | "_" } ;
```
Meta identifiers start with a letter, followed by letters, digits, or underscores.

### Comments
```
comment = "(*", { character | space  }, "*)" ;
```
- Block comments: (* nested comments supported *)

### Character Definitions
```
character_definition = hexadecimal_number | character_range ;
character_range = hexadecimal_number, "..", hexadecimal_number ;

decimal_digit = "0" | "1" | "2" | "3" | "4"
              | "5" | "6" | "7" | "8" | "9" ;

hexadecimal_number = "#", { hexadecimal_character } ;
hexadecimal_character = decimal_digit
                      | "A" | "B" | "C" | "D" | "E" | "F"
                      | "a" | "b" | "c" | "d" | "e" | "f" ;
```
Characters are defined using hexadecimal ranges covering:
- ASCII: #0 to #7F
- Unicode: #0 .. #FFFF

### Numeric Values
```
integer = decimal_digit, { decimal_digit } ;

decimal_digit = "0" | "1" | "2" | "3" | "4"
              | "5" | "6" | "7" | "8" | "9" ;
```
- Integers: decimal digits

### Whitespace
```
whitespace = #9 | #A | #D | #20 ;
```
Whitespace includes spaces, tabs, and newlines.

## Key Features

1. **Directives**: Special constructs enclosed in % symbols
2. **Hexadecimal Support**: Character definitions using # notation
3. **Character Ranges**: Support for ranges like #00 .. #FF
4. **Unicode Support**: Full Unicode character set support
5. **Nested Comments**: Support for nested block comments
6. **Escape Sequences**: Comprehensive escape sequence support
7. **Quantifiers**: Extended quantifier syntax including {n,m} ranges

## Example Usage
```
grammar = syntax_definition, [ lexical_definition ] ;

syntax_definition = { syntax_rule | comment } ;

syntax_rule = metaa_identifier, "=", definition_list, ";" ;

definition_list = single_definition, { "|", single_definition } ;
single_definition = term ,{ ",", term } ;
term = factor, { "-", exception } ;
exception = factor ;
factor = [ integer, "*" ], primary ;

primary = metaa_identifier
        | terminal_string
        | optional_sequence
        | repeted_sequence
        | grouped_sequence
        | character_definition ;

optional_sequence = "[", definition_list, "]" ;
repeted_sequence = "{", definition_list, "}" ;
grouped_sequence = "(", definition_list, ")" ;

character_definition = hexadecimal_number | character_range ;
character_range = hexadecimal_number, "..", hexadecimal_number ;

comment = "(*", { character | space  }, "*)" ;

lexical_definition = "%TOKENS%", { syntax_rule | comment } ;

%TOKENS%

metaa_identifier = letter, { letter | decimal_digit | "_" } ;

terminal_string = single_quote_terminal_string | double_quote_terminal_string ;
single_quote_terminal_string = "'", { single_quote_terminal_character }, "'" ;
double_quote_terminal_string = '"', { double_quote_terminal_character }, '"' ;

single_quote_terminal_character = character - #27 | whitespace ;
double_quote_terminal_character = character - #22 | whitespace ;

character = #0 .. #FFFF - ( #9 | #A | #D | #20 ) ;

integer = decimal_digit, { decimal_digit } ;

letter = "A" | "B" | "C" | "D" | "E" | "F" | "G"
       | "H" | "I" | "J" | "K" | "L" | "M" | "N"
       | "O" | "P" | "Q" | "R" | "S" | "T" | "U"
       | "V" | "W" | "X" | "Y" | "Z"
       | "a" | "b" | "c" | "d" | "e" | "f" | "g"
       | "h" | "i" | "j" | "k" | "l" | "m" | "n"
       | "o" | "p" | "q" | "r" | "s" | "t" | "u"
       | "v" | "w" | "x" | "y" | "z" ;

decimal_digit = "0" | "1" | "2" | "3" | "4"
              | "5" | "6" | "7" | "8" | "9" ;

hexadecimal_number = "#", { hexadecimal_character } ;
hexadecimal_character = decimal_digit
                      | "A" | "B" | "C" | "D" | "E" | "F"
                      | "a" | "b" | "c" | "d" | "e" | "f" ;

whitespace = #9 | #A | #D | #20 ;
```