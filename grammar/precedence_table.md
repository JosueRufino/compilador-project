# Tabela de Precedência de Operadores

Esta tabela define a precedência e associatividade dos operadores na gramática do compilador C educacional.

## Tabela de Precedência (do maior para o menor)

| Precedência | Operador | Descrição | Associatividade | Exemplo |
|-------------|----------|-----------|-----------------|---------|
| 1 (mais alta) | `()` | Chamada de função | Esquerda → Direita | `func(a, b)` |
| 1 | `++` `--` | Pós-incremento/decremento | Esquerda → Direita | `a++`, `b--` |
| 2 | `++` `--` | Pré-incremento/decremento | Direita → Esquerda | `++a`, `--b` |
| 2 | `+` `-` | Unário positivo/negativo | Direita → Esquerda | `+a`, `-b` |
| 2 | `!` | Negação lógica | Direita → Esquerda | `!flag` |
| 3 | `*` `/` `%` | Multiplicação, divisão, módulo | Esquerda → Direita | `a * b / c` |
| 4 | `+` `-` | Adição, subtração | Esquerda → Direita | `a + b - c` |
| 5 | `<` `<=` `>` `>=` | Relacionais | Esquerda → Direita | `a < b <= c` |
| 6 | `==` `!=` | Igualdade, desigualdade | Esquerda → Direita | `a == b != c` |
| 7 | `&&` | E lógico | Esquerda → Direita | `a && b && c` |
| 8 | `\|\|` | OU lógico | Esquerda → Direita | `a \|\| b \|\| c` |
| 9 | `?:` | Operador ternário | Direita → Esquerda | `a ? b : c` |
| 10 | `=` | Atribuição | Direita → Esquerda | `a = b = c` |
| 11 (mais baixa) | `,` | Vírgula | Esquerda → Direita | `a, b, c` |

## Regras de Associatividade

### Associatividade à Esquerda
Operadores com mesma precedência são avaliados da esquerda para a direita:
\`\`\`c
a + b + c    // equivale a ((a + b) + c)
a * b / c    // equivale a ((a * b) / c)
a < b <= c   // equivale a ((a < b) <= c)
\`\`\`

### Associatividade à Direita
Operadores com mesma precedência são avaliados da direita para a esquerda:
\`\`\`c
a = b = c    // equivale a (a = (b = c))
++--a        // equivale a (++(--a))
a ? b : c ? d : e  // equivale a (a ? b : (c ? d : e))
\`\`\`

## Exemplos de Precedência

### Exemplo 1: Expressão Aritmética
\`\`\`c
int result = 2 + 3 * 4;  // result = 2 + (3 * 4) = 14
\`\`\`

### Exemplo 2: Expressão Lógica
\`\`\`c
int a = 5, b = 10, c = 15;
if (a < b && b < c || a == 0) {  // ((a < b) && (b < c)) || (a == 0)
    // código
}
\`\`\`

### Exemplo 3: Atribuição e Comparação
\`\`\`c
int x, y = 5;
if ((x = y + 1) > 5) {  // x = (y + 1), depois (x > 5)
    // código
}
\`\`\`

### Exemplo 4: Operadores Unários
\`\`\`c
int a = 5;
int b = -++a;  // b = (-(++a)) = -6, a = 6
\`\`\`

### Exemplo 5: Chamada de Função
\`\`\`c
int result = func(a + b, c * d);  // argumentos: (a + b) e (c * d)
\`\`\`

## Implementação no Parser

A precedência é implementada através de funções recursivas no parser:

\`\`\`c
// Precedência mais baixa (vírgula)
parse_expression()
    ↓
// Atribuição
parse_assignment_expression()
    ↓
// OU lógico
parse_logical_or_expression()
    ↓
// E lógico
parse_logical_and_expression()
    ↓
// Igualdade
parse_equality_expression()
    ↓
// Relacionais
parse_relational_expression()
    ↓
// Aditivos
parse_additive_expression()
    ↓
// Multiplicativos
parse_multiplicative_expression()
    ↓
// Unários
parse_unary_expression()
    ↓
// Pós-fixos
parse_postfix_expression()
    ↓
// Primários (precedência mais alta)
parse_primary_expression()
\`\`\`

## Notas Importantes

1. **Parênteses**: Sempre têm a precedência mais alta e podem alterar a ordem de avaliação
2. **Operadores Unários**: Têm precedência mais alta que os binários correspondentes
3. **Atribuição**: Tem associatividade à direita, permitindo `a = b = c`
4. **Vírgula**: Tem a precedência mais baixa, usado para sequenciar expressões

## Casos Especiais

### Operador Ternário
\`\`\`c
a > b ? c + d : e * f  // equivale a (a > b) ? (c + d) : (e * f)
\`\`\`

### Incremento/Decremento
\`\`\`c
++a * b    // equivale a (++a) * b
a++ * b    // equivale a (a++) * b
\`\`\`

### Atribuição Composta (não implementada ainda)
\`\`\`c
a += b * c  // equivale a a = a + (b * c)
\`\`\`

Esta tabela de precedência garante que as expressões sejam analisadas corretamente pelo parser, seguindo as convenções da linguagem C.
