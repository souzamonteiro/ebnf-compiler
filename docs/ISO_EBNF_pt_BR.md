# ISO/IEC 14977:1996(E)

## RESUMO DA NORMA ISO/IEC 14977:1996(E)

### **1. ESCOPO (Cláusula 1)**
- Define uma notação (Extended BNF) para especificar sintaxe de sequências lineares de símbolos
- Aplicações: definição de linguagens de programação, comandos de SO, formatos de dados
- A notação pode definir linguagens não-analisáveis ou ambíguas

### **2. DEFINIÇÕES (Cláusula 3)**
- **Sequência**: Lista ordenada de zero ou mais itens
- **Subsequência**: Sequência dentro de uma sequência  
- **Símbolo não-terminal**: Parte sintática da linguagem sendo definida
- **Meta-identificador**: Nome de um símbolo não-terminal
- **Símbolo inicial**: Símbolo não-terminal definido mas que não ocorre em outras regras
- **Sentença**: Sequência de símbolos que representa o símbolo inicial
- **Símbolo terminal**: Sequência de um ou mais caracteres formando elemento irredutível

### **3. ELEMENTOS SINÁTICOS (Cláusula 4)**

#### **4.2-4.3 Sintaxe e Regra Sintática**
```ebnf
syntax = syntax-rule, { syntax-rule } ;
syntax-rule = meta-identifier, defining-symbol, definitions-list, terminator-symbol ;
```

#### **4.4-4.5 Listas de Definições**
```ebnf
definitions-list = single-definition, { definition-separator-symbol, single-definition } ;
single-definition = syntactic-term, { concatenate-symbol, syntactic-term } ;
```

#### **4.6-4.7 Termos e Exceções Sintáticas**
```ebnf
syntactic-term = syntactic-factor, [ except-symbol, syntactic-exception ] ;
(* syntactic-exception deve poder ser representado por syntactic-factor sem meta-identifiers *)
```

#### **4.8-4.10 Fatores e Primárias Sintáticas**
```ebnf
syntactic-factor = [ integer, repetition-symbol ], syntactic-primary ;
syntactic-primary = optional-sequence | repeated-sequence | grouped-sequence 
                  | meta-identifier | terminal-string | special-sequence | empty-sequence ;
```

#### **4.11-4.13 Construções de Sequência**
```ebnf
optional-sequence = start-option-symbol, definitions-list, end-option-symbol ;
repeated-sequence = start-repeat-symbol, definitions-list, end-repeat-symbol ;
grouped-sequence = start-group-symbol, definitions-list, end-group-symbol ;
```

#### **4.14-4.16 Identificadores e Strings**
```ebnf
meta-identifier = letter, { meta-identifier-character } ;
meta-identifier-character = letter | decimal-digit ;
terminal-string = first-quote-symbol, { first-terminal-character }, first-quote-symbol
                | second-quote-symbol, { second-terminal-character }, second-quote-symbol ;
```

#### **4.19-4.21 Sequências Especiais e Vazias**
```ebnf
special-sequence = special-sequence-symbol, { special-sequence-character }, special-sequence-symbol ;
empty-sequence = ; (* sequência vazia *)
```

### **4. SÍMBOLOS E REPRESENTAÇÕEs (Cláusula 7)**

#### **Tabela 1 - Representação Normal**
| Símbolo | Representação |
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

### **5. COMENTÁRIOS E LAYOUT (Cláusula 6)**
- Espaços em branco são ignorados fora de strings terminais
- Comentários entre `(*` e `*)` são ignorados
- Layout não afeta a linguagem definida

## EBNF DA PRÓPRIA EXTENDED BNF (Cláusula 8.1)

```ebnf
(* ============== SINTAXE ABSTRATA ============== *)
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

(* ============== ELEMENTOS LEXICAIS ============== *)
letter = 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M' 
       | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' 
       | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' 
       | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' ;

decimal-digit = '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
meta-identifier-character = letter | decimal-digit ;

first-terminal-character = terminal-character - first-quote-symbol ;
second-terminal-character = terminal-character - second-quote-symbol ;
special-sequence-character = terminal-character - special-sequence-symbol ;

(* ============== SÍMBOLOS TERMINAIS ============== *)
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
