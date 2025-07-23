#include <stdio.h>
#include <stdlib.h>
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"
#include "semantic.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erro: não foi possível abrir %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    
    return content;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("=== TESTADOR DO ANALISADOR SEMÂNTICO ===\n");
        printf("Uso: %s arquivo.c\n", argv[0]);
        return 1;
    }
    
    char* source = read_file(argv[1]);
    if (!source) return 1;
    
    printf("=== ANALISADOR SEMÂNTICO ===\n");
    printf("Arquivo: %s\n\n", argv[1]);
    
    // Parse primeiro
    Lexer* lexer = lexer_create(source);
    Parser* parser = parser_create(lexer);
    ASTNode* ast = parser_parse(parser);
    
    if (parser->has_error) {
        printf("❌ ERRO SINTÁTICO: %s\n", parser->error_message);
        return 1;
    }
    
    // Análise semântica
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    
    if (!semantic_analyze(analyzer, ast)) {
        printf("❌ ERRO SEMÂNTICO: %s\n", analyzer->error_message);
        return 1;
    }
    
    printf("✅ Análise semântica bem-sucedida!\n\n");
    printf("=== TABELA DE SÍMBOLOS ===\n");
    symbol_table_print(analyzer->symbol_table);
    
    semantic_analyzer_destroy(analyzer);
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);
    return 0;
}