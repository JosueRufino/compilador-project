# Regras da Gramática C Educacional

Este documento detalha as regras gramaticais implementadas no compilador C educacional, explicando cada construção sintática suportada.

## Estrutura Geral

### Programa
Um programa C consiste em uma sequência de declarações:
\`\`\`
program → declaration*
\`\`\`

**Exemplo:**
\`\`\`c
int x = 5;
int main() { return 0; }
float calcular(int a) { return a * 2.0; }
\`\`\`

## Declarações

### Declaração de Variável
\`\`\`
variable_declaration → type_specifier identifier [= expression] ;
\`\`\`

**Exemplos:**
\`\`\`c
int x;              // Declaração simples
float pi = 3.14;    // Declaração com inicialização
char letra = 'A';   // Declaração de caractere
\`\`\`

**Regras:**
- Tipo obrigatório (`int`, `float`, `char`, `void`)
- Identificador obrigatório
- Inicialização opcional
- Ponto e vírgula obrigatório

### Declaração de Função
\`\`\`
function_declaration → type_specifier identifier ( [parameter_list] ) compound_statement
\`\`\`

**Exemplos:**
\`\`\`c
int main() {
    return 0;
}

float calcular(int a, float b) {
    return a + b;
}

void imprimir() {
    printf("Hello World");
}
\`\`\`

**Regras:**
- Tipo de retorno obrigatório
- Nome da função obrigatório
- Parênteses obrigatórios
- Lista de parâmetros opcional
- Corpo da função obrigatório

### Lista de Parâmetros
\`\`\`
parameter_list → parameter_declaration (, parameter_declaration)*
parameter_declaration → type_specifier identifier
\`\`\`

**Exemplos:**
\`\`\`c
void func1();                    // Sem parâmetros
void func2(int x);              // Um parâmetro
void func3(int x, float y);     // Múltiplos parâmetros
\`\`\`

## Comandos (Statements)

### Comando Composto
\`\`\`
compound_statement → { (declaration | statement)* }
\`\`\`

**Exemplo:**
\`\`\`c
{
    int x = 5;
    int y = 10;
    printf("Soma: %d", x + y);
}
\`\`\`

**Regras:**
- Chaves obrigatórias
- Pode conter declarações e comandos
- Cria novo escopo

### Comando de Expressão
\`\`\`
expression_statement → [expression] ;
\`\`\`

**Exemplos:**
\`\`\`c
x = 5;              // Atribuição
func();             // Chamada de função
x++;                // Incremento
;                   // Comando vazio
\`\`\`

### Comando If
\`\`\`
if_statement → if ( expression ) statement [else statement]
\`\`\`

**Exemplos:**
\`\`\`c
if (x > 0) {
    printf("Positivo");
}

if (x > 0) {
    printf("Positivo");
} else {
    printf("Não positivo");
}

if (x > 0)
    printf("Positivo");
else if (x < 0)
    printf("Negativo");
else
    printf("Zero");
\`\`\`

**Regras:**
- Palavra-chave `if` obrigatória
- Parênteses obrigatórios na condição
- Comando then obrigatório
- Comando else opcional
- Condição deve ser uma expressão válida

### Comando While
\`\`\`
while_statement → while ( expression ) statement
\`\`\`

**Exemplos:**
\`\`\`c
while (i < 10) {
    printf("%d ", i);
    i++;
}

while (continuar) {
    continuar = processar();
}
\`\`\`

**Regras:**
- Palavra-chave `while` obrigatória
- Parênteses obrigatórios na condição
- Corpo do loop obrigatório
- Condição avaliada antes de cada iteração

### Comando Return
\`\`\`
return_statement → return [expression] ;
\`\`\`

**Exemplos:**
\`\`\`c
return;           // Retorno sem valor (void)
return 0;         // Retorno com valor
return x + y;     // Retorno com expressão
\`\`\`

**Regras:**
- Palavra-chave `return` obrigatória
- Expressão opcional
- Ponto e vírgula obrigatório
- Tipo deve ser compatível com função

## Expressões

### Hierarquia de Precedência
As expressões seguem uma hierarquia de precedência (da menor para a maior):

1. **Expressão de Vírgula** (não implementada)
2. **Expressão de Atribuição**
3. **OU Lógico** (`||`)
4. **E Lógico** (`&&`)
5. **Igualdade** (`==`, `!=`)
6. **Relacional** (`<`, `>`, `<=`, `>=`)
7. **Aditiva** (`+`, `-`)
8. **Multiplicativa** (`*`, `/`, `%`)
9. **Unária** (`-`, `!`, `++`, `--`)
10. **Pós-fixa** (chamada de função, `++`, `--`)
11. **Primária** (identificadores, constantes, parênteses)

### Expressão de Atribuição
\`\`\`
assignment_expression → logical_or_expression [= assignment_expression]
\`\`\`

**Exemplos:**
\`\`\`c
x = 5;              // Atribuição simples
x = y = z = 0;      // Atribuições múltiplas (associatividade à direita)
x = y + z;          // Atribuição com expressão
\`\`\`

### Expressões Lógicas
\`\`\`
logical_or_expression → logical_and_expression (|| logical_and_expression)*
logical_and_expression → equality_expression (&& equality_expression)*
\`\`\`

**Exemplos:**
\`\`\`c
a && b              // E lógico
a || b              // OU lógico
a && b || c         // Equivale a (a && b) || c
\`\`\`

### Expressões de Igualdade
\`\`\`
equality_expression → relational_expression ((== | !=) relational_expression)*
\`\`\`

**Exemplos:**
\`\`\`c
a == b              // Igualdade
a != b              // Desigualdade
a == b == c         // Equivale a (a == b) == c
\`\`\`

### Expressões Relacionais
\`\`\`
relational_expression → additive_expression ((< | > | <= | >=) additive_expression)*
\`\`\`

**Exemplos:**
\`\`\`c
a < b               // Menor que
a <= b              // Menor ou igual
a > b               // Maior que
a >= b              // Maior ou igual
\`\`\`

### Expressões Aritméticas
\`\`\`
additive_expression → multiplicative_expression ((+ | -) multiplicative_expression)*
multiplicative_expression → unary_expression ((* | / | %) unary_expression)*
\`\`\`

**Exemplos:**
\`\`\`c
a + b               // Adição
a - b               // Subtração
a * b               // Multiplicação
a / b               // Divisão
a % b               // Módulo
a + b * c           // Equivale a a + (b * c)
\`\`\`

### Expressões Unárias
\`\`\`
unary_expression → [- | ! | ++ | --] postfix_expression
\`\`\`

**Exemplos:**
\`\`\`c
-x                  // Negativo
!flag               // Negação lógica
++x                 // Pré-incremento
--x                 // Pré-decremento
\`\`\`

### Expressões Pós-fixas
\`\`\`
postfix_expression → primary_expression (( [argument_list] ) | ++ | --)*
\`\`\`

**Exemplos:**
\`\`\`c
func()              // Chamada sem argumentos
func(a, b)          // Chamada com argumentos
x++                 // Pós-incremento
x--                 // Pós-decremento
\`\`\`

### Expressões Primárias
\`\`\`
primary_expression → identifier | constant | string_literal | ( expression )
\`\`\`

**Exemplos:**
\`\`\`c
x                   // Identificador
42                  // Constante inteira
3.14                // Constante float
'A'                 // Constante caractere
"Hello"             // String literal
(x + y)             // Expressão entre parênteses
\`\`\`

## Tipos de Dados

### Especificadores de Tipo
\`\`\`
type_specifier → int | float | char | void
\`\`\`

**Descrição:**
- `int`: Números inteiros
- `float`: Números de ponto flutuante
- `char`: Caracteres
- `void`: Tipo vazio (apenas para funções)

### Constantes
\`\`\`
constant → integer_constant | floating_constant | character_constant
\`\`\`

**Exemplos:**
\`\`\`c
42                  // Inteiro decimal
0755                // Inteiro octal
0xFF                // Inteiro hexadecimal
3.14                // Float
.5                  // Float
2e10                // Float com notação científica
'A'                 // Caractere
'\n'                // Caractere de escape
\`\`\`

### Literais de String
\`\`\`
string_literal → " s_char* "
\`\`\`

**Exemplos:**
\`\`\`c
"Hello World"       // String simples
"Line 1\nLine 2"    // String com escape
""                  // String vazia
\`\`\`

## Identificadores
\`\`\`
identifier → letter (letter | digit | _)*
\`\`\`

**Regras:**
- Deve começar com letra ou underscore
- Pode conter letras, dígitos e underscores
- Case-sensitive
- Não pode ser palavra-chave

**Exemplos válidos:**
\`\`\`c
x, y, z
contador
_temp
variavel1
minhaFuncao
\`\`\`

**Exemplos inválidos:**
\`\`\`c
1variavel           // Começa com dígito
int                 // Palavra-chave
minha-variavel      // Contém hífen
\`\`\`

## Comentários
\`\`\`
comment → // texto_até_fim_da_linha
        | /* texto_até_*/ */
\`\`\`

**Exemplos:**
\`\`\`c
// Comentário de linha
int x = 5; // Comentário no final da linha

/*
 * Comentário de bloco
 * múltiplas linhas
 */

/* Comentário inline */ int y = 10;
\`\`\`

## Palavras-chave Reservadas

As seguintes palavras são reservadas e não podem ser usadas como identificadores:

- `int`, `float`, `char`, `void`
- `if`, `else`
- `while`, `for`
- `return`
- `break`, `continue` (planejadas)

## Limitações Atuais

Esta gramática é uma versão simplificada do C e não inclui:

- Arrays e ponteiros
- Estruturas e uniões
- Enumerações
- Typedef
- Preprocessador completo
- Operadores bitwise
- Operadores de atribuição composta
- Comando for
- Comando switch
- Múltiplos arquivos
- Bibliotecas externas

## Extensões Futuras

Planejadas para versões futuras:

- Suporte a arrays: `int arr[10]`
- Ponteiros básicos: `int *ptr`
- Estruturas: `struct Point { int x, y; }`
- Comando for: `for (init; cond; incr)`
- Preprocessador: `#include`, `#define`
