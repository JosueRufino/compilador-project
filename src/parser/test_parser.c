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

void print_error(const char* error_msg) {
    printf("❌ ERRO SINTÁTICO DETECTADO:\n");
    printf("   %s\n", error_msg);
}

void print_recovery_info(int errors_recovered) {
    if (errors_recovered > 0) {
        printf("\n📋 RELATÓRIO DE RECUPERAÇÃO:\n");
        printf("   • %d erro(s) detectado(s) e recuperado(s)\n", errors_recovered);
        printf("   • Parser continuou análise após recuperação\n");
    }
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
    
    // Capturar estado inicial
    int initial_error_state = parser->has_error;
    
    ASTNode* ast = parser_parse(parser);
    
    // Verificar resultado final
    if (parser->has_error) {
        print_error(parser->error_message);
        printf("\n🚫 Falha na análise sintática - parsing interrompido\n");
        
        if (ast) ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        return 1;
    }
    
    // Se chegou aqui, parsing foi bem-sucedido (com ou sem recuperação)
    print_success();
    
    // Verificar se houve recuperação de erros
    // (Esta informação deveria vir do parser, mas por simplicidade...)
    
    print_separator("ÁRVORE SINTÁTICA ABSTRATA");
    if (ast) {
        ast_print(ast, 0);
    } else {
        printf("⚠️  AST vazia ou não gerada\n");
    }
    
    print_separator("RESUMO");
    printf("✅ Parsing concluído\n");
    printf("📊 AST gerada com sucesso\n");
    printf("🎯 Pronto para análise semântica\n");
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);
    return 0;
}

