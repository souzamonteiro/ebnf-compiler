# ISO/IEC 14977:1996(E)

## SUMMARY OF ISO/IEC 14977:1996(E) STANDARD

### **1. SCOPE (Clause 1)**
- Defines a notation (Extended BNF) for specifying syntax of linear symbol sequences
- Applications: programming language definition, OS commands, data formats
- The notation can define unparsable or ambiguous languages

### **2. DEFINITIONS (Clause 3)**
- **Sequence**: Ordered list of zero or more items
- **Subsequence**: Sequence within a sequence  
- **Non_terminal symbol**: Syntactic part of the language being defined
- **Meta_identifier**: Name of a non_terminal symbol
- **Start symbol**: Non_terminal symbol that is defined but does not occur in other rules
- **Sentence**: Sequence of symbols representing the start symbol
- **Terminal symbol**: Sequence of one or more characters forming an irreducible element

### **3. SYNTACTIC ELEMENTS (Clause 4)**

#### **4.2_4.3 Syntax and Syntax Rule**
```ebnf
syntax = syntax_rule, { syntax_rule } ;
syntax_rule = meta_identifier, defining_symbol, definitions_list, terminator_symbol ;
```

#### **4.4_4.5 Definition Lists**
```ebnf
definitions_list = single_definition, { definition_separator_symbol, single_definition } ;
single_definition = syntactic_term, { concatenate_symbol, syntactic_term } ;
```

#### **4.6_4.7 Syntactic Terms and Exceptions**
```ebnf
syntactic_term = syntactic_factor, [ except_symbol, syntactic_exception ] ;
(* syntactic_exception must be representable by syntactic_factor without meta_identifiers *)
```

#### **4.8_4.10 Syntactic Factors and Primaries**
```ebnf
syntactic_factor = [ integer, repetition_symbol ], syntactic_primary ;
syntactic_primary = optional_sequence | repeated_sequence | grouped_sequence 
                  | meta_identifier | terminal_string | special_sequence | empty_sequence ;
```

#### **4.11_4.13 Sequence Constructions**
```ebnf
optional_sequence = start_option_symbol, definitions_list, end_option_symbol ;
repeated_sequence = start_repeat_symbol, definitions_list, end_repeat_symbol ;
grouped_sequence = start_group_symbol, definitions_list, end_group_symbol ;
```

#### **4.14_4.16 Identifiers and Strings**
```ebnf
meta_identifier = letter, { meta_identifier_character } ;
meta_identifier_character = letter | decimal_digit ;
terminal_string = first_quote_symbol, { first_terminal_character }, first_quote_symbol
                | second_quote_symbol, { second_terminal_character }, second_quote_symbol ;
```

#### **4.19_4.21 Special and Empty Sequences**
```ebnf
special_sequence = special_sequence_symbol, { special_sequence_character }, special_sequence_symbol ;
empty_sequence = ; (* empty sequence *)
```

### **4. SYMBOLS AND REPRESENTATIONS (Clause 7)**

#### **Table 1 _ Normal Representation**
| Symbol | Representation |
|--------|----------------|
| concatenate_symbol | , |
| defining_symbol | = |
| definition_separator_symbol | \| |
| end_comment_symbol | *) |
| end_group_symbol | ) |
| end_option_symbol | ] |
| end_repeat_symbol | } |
| except_symbol | - |
| first_quote_symbol | ' |
| repetition_symbol | * |
| second_quote_symbol | " |
| special_sequence_symbol | ? |
| start_comment_symbol | (* |
| start_group_symbol | ( |
| start_option_symbol | [ |
| start_repeat_symbol | { |
| terminator_symbol | ; |

### **5. COMMENTS AND LAYOUT (Clause 6)**
- Whitespace is ignored outside terminal strings
- Comments between `(*` and `*)` are ignored  
- Layout does not affect the defined language

## EXTENDED BNF DEFINITION OF ITSELF (Clause 8.1)

```ebnf
(* ============== ABSTRACT SYNTAX ============== *)
syntax = syntax_rule, { syntax_rule } ;
syntax_rule = meta_identifier, defining_symbol, definitions_list, terminator_symbol ;
definitions_list = single_definition, { definition_separator_symbol, single_definition } ;
single_definition = syntactic_term, { concatenate_symbol, syntactic_term } ;
syntactic_term = syntactic_factor, [ except_symbol, syntactic_exception ] ;
syntactic_factor = [ integer, repetition_symbol ], syntactic_primary ;
syntactic_primary = optional_sequence | repeated_sequence | grouped_sequence 
                  | meta_identifier | terminal_string | special_sequence | empty_sequence ;

optional_sequence = start_option_symbol, definitions_list, end_option_symbol ;
repeated_sequence = start_repeat_symbol, definitions_list, end_repeat_symbol ;
grouped_sequence = start_group_symbol, definitions_list, end_group_symbol ;

meta_identifier = letter, { meta_identifier_character } ;
integer = decimal_digit, { decimal_digit } ;
terminal_string = first_quote_symbol, { first_terminal_character }, first_quote_symbol
                | second_quote_symbol, { second_terminal_character }, second_quote_symbol ;
special_sequence = special_sequence_symbol, { special_sequence_character }, special_sequence_symbol ;
empty_sequence = ;

(* ============== LEXICAL ELEMENTS ============== *)
letter = 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M' 
       | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' 
       | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' 
       | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' ;

decimal_digit = '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
meta_identifier_character = letter | decimal_digit ;

first_terminal_character = terminal_character - first_quote_symbol ;
second_terminal_character = terminal_character - second_quote_symbol ;
special_sequence_character = terminal_character - special_sequence_symbol ;

(* ============== TERMINAL SYMBOLS ============== *)
defining_symbol = '=' ;
definition_separator_symbol = '|' ;
concatenate_symbol = ',' ;
terminator_symbol = ';' ;
except_symbol = '-' ;
repetition_symbol = '*' ;
first_quote_symbol = "'" ;
second_quote_symbol = '"' ;
special_sequence_symbol = '?' ;
start_option_symbol = '[' ;
end_option_symbol = ']' ;
start_repeat_symbol = '{' ;
end_repeat_symbol = '}' ;
start_group_symbol = '(' ;
end_group_symbol = ')' ;
start_comment_symbol = '(*' ;
end_comment_symbol = '*)' ;
```