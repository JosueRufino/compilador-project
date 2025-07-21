# Changelog do Compilador C

## Versão 2.0.0 - Atualização Completa

### ✨ Novas Funcionalidades

#### Sistema de Gerenciamento de Erros
- **ErrorHandler**: Sistema centralizado para gerenciar erros e avisos
- **Relatórios detalhados**: Erros com linha, coluna e contexto
- **Múltiplos tipos**: Léxicos, sintáticos, semânticos e avisos
- **Limite de erros**: Para evitar spam de mensagens

#### Tabela de Símbolos Avançada
- **Escopos hierárquicos**: Suporte completo a blocos aninhados
- **Tipos de símbolos**: Variáveis, funções, estruturas, enums, typedefs
- **Informações detalhadas**: Parâmetros de função, membros de struct
- **Verificação de redeclaração**: Detecção de símbolos duplicados

#### Gerador de Código
- **Múltiplos formatos**: C (transpilação), Assembly x86-64, Bytecode
- **Otimizações básicas**: Estrutura preparada para otimizações
- **Comentários automáticos**: Código gerado com documentação
- **Labels e temporários**: Sistema de geração automática

#### Tokens Expandidos
- **Operadores adicionais**: `++`, `--`, `->`, `.`, `?:`, bitwise
- **Palavras-chave**: `struct`, `union`, `enum`, `typedef`, `const`, `static`, `extern`
- **Melhor reconhecimento**: Suporte a mais construções C

### 🔧 Melhorias

#### Interface de Linha de Comando
- **Opções avançadas**: `-S` (assembly), `-c` (C), `-b` (bytecode)
- **Flags de debug**: `--tokens`, `--ast`, `--symbols`
- **Modo verboso**: `-v` para saída detalhada
- **Otimização**: `-O` para código otimizado

#### Análise Semântica Robusta
- **Verificação de tipos**: Compatibilidade e conversões implícitas
- **Escopo de variáveis**: Verificação correta de visibilidade
- **Funções built-in**: `printf`, `scanf` pré-definidas
- **Avisos semânticos**: Detecção de problemas não-críticos

#### Sistema de Build Melhorado
- **Makefile avançado**: Múltiplos targets e opções
- **Testes automatizados**: `make test`, `make test-assembly`, `make test-bytecode`
- **Instalação**: `make install` para instalar globalmente
- **Análise estática**: `make lint` para verificação de código

### 📁 Nova Estrutura de Arquivos

\`\`\`
src/
├── lexer.h/c           - Analisador léxico expandido
├── ast.h/c             - AST com novos tipos de nós
├── parser.h/c          - Parser com melhor tratamento de erros
├── semantic.h/c        - Análise semântica robusta
├── symbol_table.h/c    - Sistema de símbolos avançado
├── error_handler.h/c   - Gerenciamento centralizado de erros
├── code_generator.h/c  - Geração de código multi-formato
└── main.c              - Interface CLI completa

grammar/
├── c_grammar.bnf       - Gramática formal em BNF
├── c_grammar.ebnf      - Gramática em EBNF
├── precedence_table.md - Tabela de precedência
├── grammar_rules.md    - Regras detalhadas
└── syntax_diagrams.md  - Diagramas sintáticos

examples/
├── exemplo1.c          - Função Fibonacci
├── exemplo2.c          - Estruturas de controle
└── ...                 - Mais exemplos

tests/
└── (testes automatizados)
\`\`\`

### 🎯 Funcionalidades Implementadas

#### ✅ Análise Léxica
- [x] Tokens básicos e avançados
- [x] Comentários de linha e bloco
- [x] Strings com escape sequences
- [x] Números inteiros e float
- [x] Operadores unários e binários
- [x] Palavras-chave completas

#### ✅ Análise Sintática
- [x] Gramática C simplificada mas robusta
- [x] Precedência correta de operadores
- [x] Estruturas de controle (if, while, for)
- [x] Declarações de função com parâmetros
- [x] Expressões complexas aninhadas
- [x] Recuperação de erros

#### ✅ Análise Semântica
- [x] Tabela de símbolos com escopos
- [x] Verificação de tipos com conversões
- [x] Validação de declarações e usos
- [x] Verificação de compatibilidade
- [x] Detecção de variáveis não inicializadas
- [x] Verificação de retorno de funções

#### ✅ Geração de Código
- [x] Transpilação para C
- [x] Geração de Assembly x86-64
- [x] Bytecode personalizado
- [x] Preservação de semântica
- [x] Comentários no código gerado

### 🚀 Como Usar

#### Compilação Básica
\`\`\`bash
make                    # Compilar o projeto
./bin/ccompiler file.c  # Compilar arquivo C
\`\`\`

#### Opções Avançadas
\`\`\`bash
# Gerar assembly
./bin/ccompiler -S file.c -o file.s

# Modo verboso com debug
./bin/ccompiler -v --tokens --ast --symbols file.c

# Transpilação para C
./bin/ccompiler -c file.c -o output.c

# Gerar bytecode
./bin/ccompiler -b file.c -o output.bc
\`\`\`

#### Testes
\`\`\`bash
make test           # Testes básicos
make test-assembly  # Teste de assembly
make test-bytecode  # Teste de bytecode
\`\`\`

### 🔮 Próximas Versões

#### Versão 2.1.0 (Planejada)
- [ ] Suporte a arrays
- [ ] Ponteiros básicos
- [ ] Estruturas (struct)
- [ ] Preprocessador básico

#### Versão 2.2.0 (Planejada)
- [ ] Otimizações de código
- [ ] Geração de código objeto
- [ ] Linker básico
- [ ] Depuração com símbolos

#### Versão 3.0.0 (Futuro)
- [ ] Suporte completo ao C99
- [ ] IDE integrada
- [ ] Análise de fluxo de dados
- [ ] Otimizações avançadas

### 🐛 Correções de Bugs

- Corrigido parsing de expressões complexas
- Melhorado tratamento de erros léxicos
- Corrigida verificação de tipos em atribuições
- Melhorado gerenciamento de memória na AST

### ⚠️ Limitações Conhecidas

- Arrays ainda não suportados completamente
- Ponteiros em desenvolvimento
- Preprocessador limitado
- Algumas construções C avançadas não implementadas

### 📝 Notas de Migração

Se você estava usando a versão anterior:

1. **Interface CLI**: Novos parâmetros disponíveis
2. **Arquivos de saída**: Nomes padrão alterados
3. **Mensagens de erro**: Formato mais detalhado
4. **Makefile**: Novos targets disponíveis

### 🤝 Contribuições

Este projeto agora está mais estruturado para contribuições:

- Código modularizado e bem documentado
- Testes automatizados
- Sistema de build robusto
- Documentação completa

---

**Versão**: 2.0.0  
**Data**: Janeiro 2025  
**Compatibilidade**: C99, GCC 4.8+, Clang 3.5+
