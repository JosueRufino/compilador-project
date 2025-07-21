# Compilador C Educacional

Este é um compilador educacional para a linguagem C, implementado em C, que demonstra as principais fases de compilação:

## Estrutura do Projeto

\`\`\`
src/
├── lexer.h/c      - Analisador Léxico
├── ast.h/c        - Árvore Sintática Abstrata
├── parser.h/c     - Analisador Sintático
├── semantic.h/c   - Analisador Semântico
└── main.c         - Programa principal

examples/          - Exemplos de código C
Makefile          - Script de compilação
\`\`\`

## Fases do Compilador

### 1. Análise Léxica (Lexer)
- Converte o código fonte em tokens
- Reconhece palavras-chave, identificadores, números, operadores
- Remove comentários e espaços em branco
- Reporta erros léxicos

### 2. Análise Sintática (Parser)
- Constrói a Árvore Sintática Abstrata (AST)
- Implementa a gramática da linguagem C (simplificada)
- Usa análise descendente recursiva
- Reporta erros sintáticos

### 3. Análise Semântica
- Verifica tipos de dados
- Gerencia tabela de símbolos
- Verifica declarações e usos de variáveis/funções
- Reporta erros semânticos

## Gramática Suportada

\`\`\`
program → declaration*

declaration → function_declaration | variable_declaration

function_declaration → type IDENTIFIER '(' parameter_list? ')' compound_statement

variable_declaration → type IDENTIFIER ('=' expression)? ';'

statement → compound_statement | if_statement | while_statement | 
           return_statement | expression_statement

expression → assignment_expression

assignment_expression → logical_or_expression ('=' assignment_expression)?

logical_or_expression → logical_and_expression ('||' logical_and_expression)*

logical_and_expression → equality_expression ('&&' equality_expression)*

equality_expression → relational_expression (('==' | '!=') relational_expression)*

relational_expression → additive_expression (('<' | '>' | '<=' | '>=') additive_expression)*

additive_expression → multiplicative_expression (('+' | '-') multiplicative_expression)*

multiplicative_expression → unary_expression (('*' | '/' | '%') unary_expression)*

unary_expression → ('-' | '!') unary_expression | postfix_expression

postfix_expression → primary_expression ('(' argument_list? ')')*

primary_expression → IDENTIFIER | NUMBER | STRING | '(' expression ')'
\`\`\`

## Compilação e Uso

### Compilar o projeto:
\`\`\`bash
make
\`\`\`

### Executar o compilador:
\`\`\`bash
./bin/compiler examples/exemplo1.c
\`\`\`

### Limpar arquivos compilados:
\`\`\`bash
make clean
\`\`\`

### Executar teste básico:
\`\`\`bash
make test
\`\`\`

## Funcionalidades Implementadas

### Analisador Léxico
- [x] Reconhecimento de tokens básicos
- [x] Palavras-chave (int, float, char, void, if, else, while, for, return)
- [x] Operadores aritméticos e lógicos
- [x] Delimitadores e pontuação
- [x] Comentários de linha (//) e bloco (/* */)
- [x] Strings e números
- [x] Tratamento de erros léxicos

### Analisador Sintático
- [x] Análise descendente recursiva
- [x] Construção da AST
- [x] Declarações de funções e variáveis
- [x] Estruturas de controle (if, while)
- [x] Expressões com precedência correta
- [x] Chamadas de função
- [x] Tratamento de erros sintáticos

### Analisador Semântico
- [x] Tabela de símbolos com escopos
- [x] Verificação de tipos
- [x] Verificação de declarações
- [x] Verificação de compatibilidade de tipos
- [x] Funções built-in (printf, scanf)
- [x] Tratamento de erros semânticos

## Limitações

Este é um compilador educacional com algumas limitações:

- Não gera código objeto ou executável
- Suporte limitado a tipos de dados
- Não implementa preprocessador completo
- Arrays e ponteiros não totalmente suportados
- Estruturas e unions não implementadas

## Exemplos de Uso

### Exemplo 1 - Função Fibonacci
\`\`\`c
#include <stdio.h>

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    int num = 10;
    int result = fibonacci(num);
    printf("Fibonacci de %d = %d\n", num, result);
    return 0;
}
\`\`\`

### Exemplo 2 - Estruturas de Controle
\`\`\`c
int main() {
    int x = 5;
    int y = 10;
    int soma = x + y;
    
    if (soma > 10) {
        printf("Soma é maior que 10\n");
    } else {
        printf("Soma é menor ou igual a 10\n");
    }
    
    int i = 0;
    while (i < 5) {
        printf("Contador: %d\n", i);
        i = i + 1;
    }
    
    return soma;
}
\`\`\`

## Saída do Compilador

O compilador produz três tipos de saída:

1. **Análise Léxica**: Lista todos os tokens encontrados
2. **Análise Sintática**: Mostra a árvore sintática abstrata
3. **Análise Semântica**: Reporta verificações de tipo e escopo

## Contribuição

Este projeto é educacional e pode ser estendido com:

- Geração de código intermediário
- Otimizações
- Suporte a mais tipos de dados
- Melhor tratamento de erros
- Interface gráfica

## Licença

Este projeto é de domínio público e pode ser usado para fins educacionais.
