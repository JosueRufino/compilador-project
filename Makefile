CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
SRCDIR = src
OBJDIR = obj
BINDIR = bin
TESTDIR = tests

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/ccompiler

.PHONY: all clean test install uninstall help

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $@
	@echo "✅ Compilador construído com sucesso!"

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	rm -f *.c *.s *.bc test_*.c
	@echo "🧹 Arquivos limpos!"

test: $(TARGET)
	@echo "🧪 Executando testes..."
	@echo 'int main() { int x = 10; return x + 5; }' > test_simple.c
	./$(TARGET) -v --tokens --ast test_simple.c -o test_simple_out.c
	@echo ""
	@echo 'int fibonacci(int n) { if (n <= 1) return n; return fibonacci(n-1) + fibonacci(n-2); } int main() { return fibonacci(5); }' > test_fibonacci.c
	./$(TARGET) -v test_fibonacci.c -o test_fibonacci_out.c
	@echo ""
	@echo "✅ Testes concluídos!"

test-assembly: $(TARGET)
	@echo "🔧 Testando geração de assembly..."
	@echo 'int main() { int x = 5; int y = 10; return x + y; }' > test_asm.c
	./$(TARGET) -S -v test_asm.c -o test_asm.s
	@echo "✅ Assembly gerado!"

test-bytecode: $(TARGET)
	@echo "📦 Testando geração de bytecode..."
	@echo 'int main() { return 42; }' > test_bc.c
	./$(TARGET) -b -v test_bc.c -o test_bc.bc
	@echo "✅ Bytecode gerado!"

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "📦 Compilador instalado em /usr/local/bin/"

uninstall:
	sudo rm -f /usr/local/bin/ccompiler
	@echo "🗑️ Compilador removido!"

help:
	@echo "📚 Comandos disponíveis:"
	@echo "  make              - Compila o projeto"
	@echo "  make clean        - Remove arquivos compilados"
	@echo "  make test         - Executa testes básicos"
	@echo "  make test-assembly- Testa geração de assembly"
	@echo "  make test-bytecode- Testa geração de bytecode"
	@echo "  make install      - Instala o compilador"
	@echo "  make uninstall    - Remove o compilador"
	@echo "  make help         - Mostra esta ajuda"

# Regras para desenvolvimento
debug: CFLAGS += -DDEBUG -g3
debug: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)

# Análise estática
lint:
	@echo "🔍 Executando análise estática..."
	cppcheck --enable=all --std=c99 $(SRCDIR)/*.c

# Documentação
docs:
	@echo "📖 Gerando documentação..."
	doxygen Doxyfile
