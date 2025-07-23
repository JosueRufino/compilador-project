#include <stdio.h>
#include <stdlib.h>
#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "parser.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("❌ Erro: não foi possível abrir %s\n", filename);
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

void print_separator(const char* title) {
    printf("\n=== %s ===\n", title);
}

void print_success() {
    printf("✅ Análise sintática concluída com sucesso!\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("=== TESTADOR DO ANALISADOR SINTÁTICO ===\n");
        printf("Uso: %s arquivo.c\n", argv[0]);
        return 1;
    }
    
    char* source = read_file(argv[1]);
    if (!source) return 1;
    
    print_separator("ANALISADOR SINTÁTICO");
    printf("📁 Arquivo: %s\n", argv[1]);
    
    Lexer* lexer = lexer_create(source);
    Parser* parser = parser_create(lexer);
    
    ASTNode* ast = parser_parse(parser);
    
    if (parser->recovered_errors->count == 0) {
        print_success();
    } else {
        printf("⚠️ Análise sintática concluída com %d erro(s)\n", parser->recovered_errors->count);
    }
    
    print_separator("ÁRVORE SINTÁTICA ABSTRATA");
    if (ast) {
        ast_print(ast, 0);
    } else {
        printf("⚠️ AST vazia ou não gerada\n");
    }
    
    print_separator("RESUMO");
    printf("✅ Parsing concluído\n");
    printf("📊 AST %s\n", ast ? "gerada com sucesso" : "não gerada");
    printf("🎯 Pronto para análise semântica (com %d erro(s) sintático(s))\n", 
           parser->recovered_errors->count);
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);
    return parser->recovered_errors->count > 0 ? 1 : 0;
}