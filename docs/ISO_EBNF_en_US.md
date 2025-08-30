# ISO/IEC 14977:1996(E)

## SUMMARY OF ISO/IEC 14977:1996(E) STANDARD

### **1. SCOPE (Clause 1)**
- Defines a notation (Extended BNF) for specifying syntax of linear symbol sequences
- Applications: programming language definition, OS commands, data formats
- The notation can define unparsable or ambiguous languages

### **2. DEFINITIONS (Clause 3)**
- **Sequence**: Ordered list of zero or more items
- **Subsequence**: Sequence within a sequence  
- **Non-terminal symbol**: Syntactic part of the language being defined
- **Meta-identifier**: Name of a non-terminal symbol
- **Start symbol**: Non-terminal symbol that is defined but does not occur in other rules
- **Sentence**: Sequence of symbols representing the start symbol
- **Terminal symbol**: Sequence of one or more characters forming an irreducible element

### **3. SYNTACTIC ELEMENTS (Clause 4)**

#### **4.2-4.3 Syntax and Syntax Rule**
```ebnf
syntax = syntax-rule, { syntax-rule } ;
syntax-rule = meta-identifier, defining-symbol, definitions-list, terminator-symbol ;
```

#### **4.4-4.5 Definition Lists**
```ebnf
definitions-list = single-definition, { definition-separator-symbol, single-definition } ;
single-definition = syntactic-term, { concatenate-symbol, syntactic-term } ;
```

#### **4.6-4.7 Syntactic Terms and Exceptions**
```ebnf
syntactic-term = syntactic-factor, [ except-symbol, syntactic-exception ] ;
(* syntactic-exception must be representable by syntactic-factor without meta-identifiers *)
```

#### **4.8-4.10 Syntactic Factors and Primaries**
```ebnf
syntactic-factor = [ integer, repetition-symbol ], syntactic-primary ;
syntactic-primary = optional-sequence | repeated-sequence | grouped-sequence 
                  | meta-identifier | terminal-string | special-sequence | empty-sequence ;
```

#### **4.11-4.13 Sequence Constructions**
```ebnf
optional-sequence = start-option-symbol, definitions-list, end-option-symbol ;
repeated-sequence = start-repeat-symbol, definitions-list, end-repeat-symbol ;
grouped-sequence = start-group-symbol, definitions-list, end-group-symbol ;
```

#### **4.14-4.16 Identifiers and Strings**
```ebnf
meta-identifier = letter, { meta-identifier-character } ;
meta-identifier-character = letter | decimal-digit ;
terminal-string = first-quote-symbol, { first-terminal-character }, first-quote-symbol
                | second-quote-symbol, { second-terminal-character }, second-quote-symbol ;
```

#### **4.19-4.21 Special and Empty Sequences**
```ebnf
special-sequence = special-sequence-symbol, { special-sequence-character }, special-sequence-symbol ;
empty-sequence = ; (* empty sequence *)
```

### **4. SYMBOLS AND REPRESENTATIONS (Clause 7)**

#### **Table 1 - Normal Representation**
| Symbol | Representation |
|---------|---------------|
| concatenate-symbol | , |
| defining-symbol | = |
| definition-separator-symbol | \| |
| end-comment-symbol | *) |
| end-group-symbol | ) |
| end-option-symbol | ] |
| end-repeat-symbol | } |
| except-symbol | - |
| first-quote-symbol | ' |
| repetition-symbol | * |
| second-quote-symbol | " |
| special-sequence-symbol | ? |
| start-comment-symbol | (* |
| start-group-symbol | ( |
| start-option-symbol | [ |
| start-repeat-symbol | { |
| terminator-symbol | ; |

### **5. COMMENTS AND LAYOUT (Clause 6)**
- Whitespace is ignored outside terminal strings
- Comments between `(*` and `*)` are ignored  
- Layout does not affect the defined language

## EXTENDED BNF DEFINITION OF ITSELF (Clause 8.1)

```ebnf
(* ============== ABSTRACT SYNTAX ============== *)
syntax = syntax-rule, { syntax-rule } ;
syntax-rule = meta-identifier, defining-symbol, definitions-list, terminator-symbol ;
definitions-list = single-definition, { definition-separator-symbol, single-definition } ;
single-definition = syntactic-term, { concatenate-symbol, syntactic-term } ;
syntactic-term = syntactic-factor, [ except-symbol, syntactic-exception ] ;
syntactic-factor = [ integer, repetition-symbol ], syntactic-primary ;
syntactic-primary = optional-sequence | repeated-sequence | grouped-sequence 
                  | meta-identifier | terminal-string | special-sequence | empty-sequence ;

optional-sequence = start-option-symbol, definitions-list, end-option-symbol ;
repeated-sequence = start-repeat-symbol, definitions-list, end-repeat-symbol ;
grouped-sequence = start-group-symbol, definitions-list, end-group-symbol ;

meta-identifier = letter, { meta-identifier-character } ;
integer = decimal-digit, { decimal-digit } ;
terminal-string = first-quote-symbol, { first-terminal-character }, first-quote-symbol
                | second-quote-symbol, { second-terminal-character }, second-quote-symbol ;
special-sequence = special-sequence-symbol, { special-sequence-character }, special-sequence-symbol ;
empty-sequence = ;

(* ============== LEXICAL ELEMENTS ============== *)
letter = 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M' 
       | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' 
       | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' 
       | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' ;

decimal-digit = '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
meta-identifier-character = letter | decimal-digit ;

first-terminal-character = terminal-character - first-quote-symbol ;
second-terminal-character = terminal-character - second-quote-symbol ;
special-sequence-character = terminal-character - special-sequence-symbol ;

(* ============== TERMINAL SYMBOLS ============== *)
defining-symbol = '=' ;
definition-separator-symbol = '|' ;
concatenate-symbol = ',' ;
terminator-symbol = ';' ;
except-symbol = '-' ;
repetition-symbol = '*' ;
first-quote-symbol = "'" ;
second-quote-symbol = '"' ;
special-sequence-symbol = '?' ;
start-option-symbol = '[' ;
end-option-symbol = ']' ;
start-repeat-symbol = '{' ;
end-repeat-symbol = '}' ;
start-group-symbol = '(' ;
end-group-symbol = ')' ;
start-comment-symbol = '(*' ;
end-comment-symbol = '*)' ;
```