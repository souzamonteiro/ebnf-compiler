# ISO/IEC 14977:1996(E)

## RESUMO DA NORMA ISO/IEC 14977:1996(E)

### **1. ESCOPO (Cláusula 1)**
- Define uma notação (Extended BNF) para especificar sintaxe de sequências lineares de símbolos
- Aplicações: definição de linguagens de programação, comandos de SO, formatos de dados
- A notação pode definir linguagens não_analisáveis ou ambíguas

### **2. DEFINIÇÕES (Cláusula 3)**
- **Sequência**: Lista ordenada de zero ou mais itens
- **Subsequência**: Sequência dentro de uma sequência  
- **Símbolo não_terminal**: Parte sintática da linguagem sendo definida
- **Meta_identificador**: Nome de um símbolo não_terminal
- **Símbolo inicial**: Símbolo não_terminal definido mas que não ocorre em outras regras
- **Sentença**: Sequência de símbolos que representa o símbolo inicial
- **Símbolo terminal**: Sequência de um ou mais caracteres formando elemento irredutível

### **3. ELEMENTOS SINÁTICOS (Cláusula 4)**

#### **4.2_4.3 Sintaxe e Regra Sintática**
```ebnf
syntax = syntax_rule, { syntax_rule } ;
syntax_rule = meta_identifier, defining_symbol, definitions_list, terminator_symbol ;
```

#### **4.4_4.5 Listas de Definições**
```ebnf
definitions_list = single_definition, { definition_separator_symbol, single_definition } ;
single_definition = syntactic_term, { concatenate_symbol, syntactic_term } ;
```

#### **4.6_4.7 Termos e Exceções Sintáticas**
```ebnf
syntactic_term = syntactic_factor, [ except_symbol, syntactic_exception ] ;
(* syntactic_exception deve poder ser representado por syntactic_factor sem meta_identifiers *)
```

#### **4.8_4.10 Fatores e Primárias Sintáticas**
```ebnf
syntactic_factor = [ integer, repetition_symbol ], syntactic_primary ;
syntactic_primary = optional_sequence | repeated_sequence | grouped_sequence 
                  | meta_identifier | terminal_string | special_sequence | empty_sequence ;
```

#### **4.11_4.13 Construções de Sequência**
```ebnf
optional_sequence = start_option_symbol, definitions_list, end_option_symbol ;
repeated_sequence = start_repeat_symbol, definitions_list, end_repeat_symbol ;
grouped_sequence = start_group_symbol, definitions_list, end_group_symbol ;
```

#### **4.14_4.16 Identificadores e Strings**
```ebnf
meta_identifier = letter, { meta_identifier_character } ;
meta_identifier_character = letter | decimal_digit ;
terminal_string = first_quote_symbol, { first_terminal_character }, first_quote_symbol
                | second_quote_symbol, { second_terminal_character }, second_quote_symbol ;
```

#### **4.19_4.21 Sequências Especiais e Vazias**
```ebnf
special_sequence = special_sequence_symbol, { special_sequence_character }, special_sequence_symbol ;
empty_sequence = ; (* sequência vazia *)
```

### **4. SÍMBOLOS E REPRESENTAÇÕEs (Cláusula 7)**

#### **Tabela 1 _ Representação Normal**
| Símbolo | Representação |
|---------|---------------|
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

### **5. COMENTÁRIOS E LAYOUT (Cláusula 6)**
- Espaços em branco são ignorados fora de strings terminais
- Comentários entre `(*` e `*)` são ignorados
- Layout não afeta a linguagem definida

## EBNF DA PRÓPRIA EXTENDED BNF (Cláusula 8.1)

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
