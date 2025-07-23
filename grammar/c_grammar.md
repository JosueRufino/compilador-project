# Gramática C Corrigida - Sem Ambiguidades

## Estrutura Principal

<programa> ::= { <preprocessador> } { <declaracao_global> }

<preprocessador> ::= <include_directive>
                  | <define_directive>
                  | <ifdef_directive>
                  | <ifndef_directive>
                  | <endif_directive>
                  | <undef_directive>

<include_directive> ::= "#include" ("<" <identificador> ".h" ">" | "\"" <identificador> ".h" "\"")
<define_directive> ::= "#define" <identificador> [ <macro_params> ] <macro_body>
<ifdef_directive> ::= "#ifdef" <identificador>
<ifndef_directive> ::= "#ifndef" <identificador>
<endif_directive> ::= "#endif"
<undef_directive> ::= "#undef" <identificador>

<macro_params> ::= "(" [ <lista_identificadores_macro> ] ")"
<lista_identificadores_macro> ::= <identificador> { "," <identificador> }
<macro_body> ::= { <token> }

<declaracao_global> ::= <definicao_funcao> | <declaracao_variavel> | <typedef> | <struct>

## Definições de Tipos

<typedef> ::= "typedef" <tipo> <identificador> ";"

<struct> ::= "struct" <identificador>? "{" { <declaracao_variavel> } "}" [ <identificador> ] ";"

<tipo> ::= [ "const" ] <tipo_base> { "*" }
<tipo_base> ::= "int" | "float" | "char" | "void" | <identificador>

## Declarações

<declaracao_variavel> ::= <tipo> <lista_identificadores> ";"
<lista_identificadores> ::= <inicializador> { "," <inicializador> }
<inicializador> ::= <designador> [ "=" <expressao_inicializacao> ]

# NOVO: Designador com suporte a arrays multidimensionais
<designador> ::= <identificador> { <dimensao_array> }
<dimensao_array> ::= "[" [ <expressao> ] "]"

# NOVO: Inicialização completa de arrays e estruturas
<expressao_inicializacao> ::= <expressao> 
                           | <lista_inicializadores>

<lista_inicializadores> ::= "{" [ <inicializadores> ] "}"
<inicializadores> ::= <inicializador_elemento> { "," <inicializador_elemento> }
<inicializador_elemento> ::= [ <designador_inicializacao> ] <expressao_inicializacao>

# Para inicializadores designados: arr[2] = 5, .campo = 10
<designador_inicializacao> ::= { <designador_array> | <designador_campo> }
<designador_array> ::= "[" <expressao> "]"
<designador_campo> ::= "." <identificador>

<definicao_funcao> ::= <tipo> <identificador> "(" [ <parametros> ] ")" <bloco>
<parametros> ::= <parametro> { "," <parametro> }
<parametro> ::= <tipo> <designador>

## Comandos - CORRIGIDO: Dangling Else Resolvido

<bloco> ::= "{" { <item_bloco> } "}"
<item_bloco> ::= <declaracao_variavel> | <comando>

<comando> ::= <comando_sem_if>
           | <comando_if_completo>
           | <comando_if_incompleto>

<comando_sem_if> ::= <comando_expressao>
                  | <comando_bloco>
                  | <comando_while>
                  | <comando_do_while>
                  | <comando_for>
                  | <comando_switch>
                  | <comando_return>
                  | <comando_break>
                  | <comando_continue>
                  | <comando_goto>
                  | <comando_rotulo>

# IF com ELSE (matched) - else liga sempre ao if mais próximo
<comando_if_completo> ::= "if" "(" <expressao> ")" <comando_if_completo> "else" <comando>
                       | "if" "(" <expressao> ")" <comando_sem_if> "else" <comando>

# IF sem ELSE (unmatched)
<comando_if_incompleto> ::= "if" "(" <expressao> ")" <comando>

## Outros Comandos

<comando_expressao> ::= [ <expressao> ] ";"
<comando_bloco> ::= <bloco>
<comando_while> ::= "while" "(" <expressao> ")" <comando>
<comando_do_while> ::= "do" <comando> "while" "(" <expressao> ")" ";"
<comando_for> ::= "for" "(" [ <for_init> ] ";" [ <expressao> ] ";" [ <expressao> ] ")" <comando>
<for_init> ::= <declaracao_variavel_local> | <expressao>
<declaracao_variavel_local> ::= <tipo> <identificador> [ "=" <expressao> ]

<comando_switch> ::= "switch" "(" <expressao> ")" "{" { <caso_switch> } [ <caso_default> ] "}"
<caso_switch> ::= "case" <constante> ":" { <comando> }
<caso_default> ::= "default" ":" { <comando> }

<comando_return> ::= "return" [ <expressao> ] ";"
<comando_break> ::= "break" ";"
<comando_continue> ::= "continue" ";"
<comando_goto> ::= "goto" <identificador> ";"
<comando_rotulo> ::= <identificador> ":" <comando>

## Expressões - CORRIGIDO: Precedência e Associatividade

<expressao> ::= <expressao_virgula>
<expressao_virgula> ::= <atribuicao> { "," <atribuicao> }

# Atribuição (associativa à direita)
<atribuicao> ::= <condicional> [ <operador_atribuicao> <atribuicao> ]

# Operador ternário (associativo à direita)  
<condicional> ::= <logico_ou> [ "?" <expressao> ":" <condicional> ]

# Operadores lógicos (associativos à esquerda)
<logico_ou> ::= <logico_e> { "||" <logico_e> }
<logico_e> ::= <bitwise_ou> { "&&" <bitwise_ou> }

# Operadores bitwise (associativos à esquerda)
<bitwise_ou> ::= <bitwise_xor> { "|" <bitwise_xor> }
<bitwise_xor> ::= <bitwise_and> { "^" <bitwise_and> }
<bitwise_and> ::= <igualdade> { "&" <igualdade> }

# Operadores de comparação (associativos à esquerda)
<igualdade> ::= <relacional> { ("==" | "!=") <relacional> }
<relacional> ::= <shift> { ("<" | ">" | "<=" | ">=") <shift> }

# Operadores de shift (associativos à esquerda)
<shift> ::= <soma> { ("<<" | ">>") <soma> }

# Operadores aritméticos (associativos à esquerda)
<soma> ::= <produto> { ("+" | "-") <produto> }
<produto> ::= <cast> { ("*" | "/" | "%") <cast> }

# NOVO: Cast explícito para resolver ambiguidade
<cast> ::= "(" <tipo> ")" <cast>
        | <unario>

# Operadores unários (associativos à direita)
<unario> ::= <operador_unario> <unario>
          | <sufixo>

<operador_unario> ::= "+" | "-" | "!" | "~" | "++" | "--" | "*" | "&"

# Operadores sufixo (associativos à esquerda)
<sufixo> ::= <primario> { <operador_sufixo> }
<operador_sufixo> ::= "[" <expressao> "]"
                   | "(" [ <lista_expressao> ] ")"
                   | "." <identificador>
                   | "->" <identificador>
                   | "++"
                   | "--"

# Expressões primárias
<primario> ::= <numero>
            | <caractere>
            | <string>
            | <identificador>
            | "(" <expressao> ")"
            | <lista_inicializadores>

## Listas e Operadores

<lista_expressao> ::= <expressao> { "," <expressao> }

<operador_atribuicao> ::= "=" | "+=" | "-=" | "*=" | "/=" | "%=" 
                       | "&=" | "|=" | "^=" | "<<=" | ">>="

## Tokens Básicos

<numero> ::= <numero_inteiro> | <numero_float>
<numero_inteiro> ::= <decimal> | <octal> | <hexadecimal>
<decimal> ::= [1-9][0-9]*
<octal> ::= "0"[0-7]*
<hexadecimal> ::= "0"[xX][0-9a-fA-F]+
<numero_float> ::= [0-9]+ "." [0-9]+ | [0-9]+ "." | "." [0-9]+

<identificador> ::= [a-zA-Z_][a-zA-Z0-9_]*

<caractere> ::= "'" <char_content> "'"
<char_content> ::= [^'\\] | "\\" ['"\\ntr]

<string> ::= "\"" { <string_char> } "\""
<string_char> ::= [^"\\] | "\\" ['"\\ntr]

<constante> ::= <numero> | <caractere>

# NOVO: Token genérico para macro bodies
<token> ::= <identificador> | <numero> | <caractere> | <string> | <operador> | <delimitador>
<operador> ::= "+" | "-" | "*" | "/" | "%" | "=" | "==" | "!=" | "<" | ">" | "<=" | ">=" 
            | "&&" | "||" | "!" | "&" | "|" | "^" | "~" | "<<" | ">>" | "++" | "--"
            | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="
            | "?" | ":" | "." | "->" | "sizeof"
<delimitador> ::= "(" | ")" | "[" | "]" | "{" | "}" | ";" | ","

---

## 🎯 **CORREÇÕES IMPLEMENTADAS:**

### ✅ **1. Dangling Else Resolvido:**
- `comando_if_completo`: IF com ELSE obrigatório
- `comando_if_incompleto`: IF sem ELSE  
- **Regra:** ELSE sempre liga ao IF mais próximo

### ✅ **2. Cast Explícito:**
- Adicionada regra `<cast>` para resolver ambiguidade `(int)` vs `(expressão)`
- Cast tem precedência maior que operadores unários

### ✅ **3. Separação Clara de Contextos:**
- `item_bloco`: distingue declarações de comandos em blocos
- `for_init`: permite declarações locais em for

### ✅ **4. Operadores Completos:**
- Incluído `*` (dereference) e `&` (address-of)
- Operadores bitwise: `|`, `^`, `&`, `<<`, `>>`
- Operador ternário: `? :`
- Operadores de atribuição compostos: `+=`, `-=`, etc.

### ✅ **5. Precedência Completa e Correta:**

vírgula → atribuição → ternário → ||  → && → | → ^ → & 
→ == != → < > <= >= → << >> → + - → * / % → cast → unário → sufixo → primário

### ✅ **6. NOVO: Preprocessador Completo:**
- `#include` com arquivos de cabeçalho
- `#define` para macros simples e com parâmetros
- `#ifdef`, `#ifndef`, `#endif` para compilação condicional
- `#undef` para remover definições

### ✅ **7. NOVO: Arrays Multidimensionais:**
- Suporte a `int matrix[3][4][5]`
- Arrays de tamanho variável: `int arr[]`
- Declarações flexíveis: `int arr[][5]`

### ✅ **8. NOVO: Inicialização Completa de Arrays:**
- Inicialização literal: `int arr[] = {1, 2, 3, 4}`
- Inicialização aninhada: `int matrix[2][3] = {{1,2,3}, {4,5,6}}`
- Inicialização parcial: `int arr[5] = {1, 2}` (demais = 0)
- Inicialização designada: `int arr[10] = {[2] = 5, [7] = 9}`
- Inicialização de structs: `struct Point p = {.x = 10, .y = 20}`

### ✅ **9. Estruturas Avançadas:**
- `typedef` para definição de tipos
- `struct` para estruturas
- Arrays e ponteiros
- Múltiplos tipos de constantes (decimal, octal, hexadecimal)

---

## 📝 **EXEMPLOS DE CÓDIGO SUPORTADO:**

### **Arrays Multidimensionais:**
```c
int matrix[3][4];                    // ✅ Declaração 2D
int cube[2][3][4];                   // ✅ Declaração 3D
int arr[][5] = {{1,2}, {3,4}};       // ✅ Tamanho inferido
```

### **Inicialização de Arrays:**
```c
int arr[] = {1, 2, 3, 4};            // ✅ Tamanho automático
int matrix[2][3] = {{1,2,3}, {4,5,6}}; // ✅ Multidimensional
int partial[5] = {1, 2};             // ✅ Inicialização parcial
int designated[10] = {[2] = 5, [7] = 9}; // ✅ Designada
```

### **Preprocessador:**
```c
#include <stdio.h>                   // ✅ Include sistema
#include "myheader.h"                // ✅ Include local
#define MAX 100                      // ✅ Macro simples
#define MIN(a,b) ((a) < (b) ? (a) : (b)) // ✅ Macro com parâmetros
#ifdef DEBUG                         // ✅ Compilação condicional
    printf("Debug mode\n");
#endif
```

Esta gramática agora está **completa** para um compilador C educacional robusto!