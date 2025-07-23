CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRCDIR = src
BINDIR = bin

# Criar diretório bin
$(shell mkdir -p $(BINDIR))

# Caminhos dos módulos
LEXER_DIR = $(SRCDIR)/lexer
PARSER_DIR = $(SRCDIR)/parser
AST_DIR = $(SRCDIR)/ast
SEMANTIC_DIR = $(SRCDIR)/semantic
SYMBOL_TABLE_DIR = $(SRCDIR)/symbol_table
CODE_GEN_DIR = $(SRCDIR)/code_generator
ERROR_DIR = $(SRCDIR)/error_handler

# Arquivos principais de cada módulo
LEXER_SRCS = $(LEXER_DIR)/lexer.c
PARSER_SRCS = $(PARSER_DIR)/parser.c
AST_SRCS = $(AST_DIR)/ast.c
SEMANTIC_SRCS = $(SEMANTIC_DIR)/semantic.c
SYMBOL_TABLE_SRCS = $(SYMBOL_TABLE_DIR)/symbol_table.c
CODE_GEN_SRCS = $(CODE_GEN_DIR)/code_generator.c
ERROR_SRCS = $(ERROR_DIR)/error_handler.c

# Todos os módulos principais
ALL_MODULES = $(LEXER_SRCS) $(PARSER_SRCS) $(AST_SRCS) $(SEMANTIC_SRCS) \
              $(SYMBOL_TABLE_SRCS) $(CODE_GEN_SRCS) $(ERROR_SRCS)

# Executáveis
MAIN = $(BINDIR)/compiler
LEXER_TEST = $(BINDIR)/test-lexer
PARSER_TEST = $(BINDIR)/test-parser
SEMANTIC_TEST = $(BINDIR)/test-semantic

# Includes para compilação
INCLUDES = -I$(LEXER_DIR) -I$(PARSER_DIR) -I$(AST_DIR) -I$(SEMANTIC_DIR) \
           -I$(SYMBOL_TABLE_DIR) -I$(CODE_GEN_DIR) -I$(ERROR_DIR)

.PHONY: all clean test-lexer test-parser test-semantic setup

all: $(MAIN) $(LEXER_TEST) $(PARSER_TEST) $(SEMANTIC_TEST)

# Compilador principal
$(MAIN): $(ALL_MODULES) $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

# Testador do lexer
$(LEXER_TEST): $(LEXER_SRCS) $(AST_SRCS) $(LEXER_DIR)/test_lexer.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

# Testador do parser
$(PARSER_TEST): $(LEXER_SRCS) $(PARSER_SRCS) $(AST_SRCS) $(PARSER_DIR)/test_parser.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

# Testador semântico
$(SEMANTIC_TEST): $(LEXER_SRCS) $(PARSER_SRCS) $(AST_SRCS) $(SEMANTIC_SRCS) \
                  $(SYMBOL_TABLE_SRCS) $(SEMANTIC_DIR)/test_semantic.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

# Testes individuais
test-lexer: $(LEXER_TEST)
	@echo "=== TESTANDO ANALISADOR LÉXICO ==="
	./$(LEXER_TEST) examples/exemplo1.c

test-parser: $(PARSER_TEST)
	@echo "=== TESTANDO ANALISADOR SINTÁTICO ==="
	./$(PARSER_TEST) examples/exemplo1.c

test-semantic: $(SEMANTIC_TEST)
	@echo "=== TESTANDO ANALISADOR SEMÂNTICO ==="
	./$(SEMANTIC_TEST) examples/exemplo2.c

# Teste completo
test-all: test-lexer test-parser test-semantic
	@echo "=== TESTANDO COMPILADOR COMPLETO ==="
	./$(MAIN) examples/exemplo1.c

clean:
	rm -rf $(BINDIR)

# Criar estrutura de pastas em src/
setup:
	@echo "Criando estrutura modular em src/..."
	mkdir -p $(LEXER_DIR) $(PARSER_DIR) $(AST_DIR) $(SEMANTIC_DIR) \
	         $(SYMBOL_TABLE_DIR) $(CODE_GEN_DIR) $(ERROR_DIR) examples $(BINDIR)
	@echo "Estrutura criada!"

help:
	@echo "=== COMPILADOR C MODULAR ==="
	@echo "Comandos:"
	@echo "  make all           - Compilar tudo"  
	@echo "  make test-lexer    - Testar só o analisador léxico"
	@echo "  make test-parser   - Testar só o analisador sintático"
	@echo "  make test-semantic - Testar só o analisador semântico"
	@echo "  make test-all      - Testar tudo"
	@echo "  make setup         - Criar estrutura de pastas"
	@echo "  make clean         - Limpar executáveis"
	@echo ""
	@echo "Uso individual:"
	@echo "  ./bin/test-lexer arquivo.c"
	@echo "  ./bin/test-parser arquivo.c"  
	@echo "  ./bin/test-semantic arquivo.c"
	@echo "  ./bin/compiler arquivo.c"