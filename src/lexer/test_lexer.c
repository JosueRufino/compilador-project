#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

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
        printf("=== TESTADOR DO ANALISADOR LÉXICO ===\n");
        printf("Uso: %s arquivo.c\n", argv[0]);
        return 1;
    }
    
    char* source = read_file(argv[1]);
    if (!source) return 1;
    
    printf("=== ANALISADOR LÉXICO ===\n");
    printf("Arquivo: %s\n\n", argv[1]);
    
    printf("%-4s | %-16s | %-12s | %s\n", "Nº", "Tipo", "Valor", "Linha:Col");
    printf("-----|------------------|--------------|----------\n");
    
    Lexer* lexer = lexer_create(source);
    Token token;
    int count = 0;
    
    do {
        token = lexer_next_token(lexer);
        count++;
        
        printf("%-4d | %-16s | %-12s | %d:%d\n", 
               count,
               token_type_to_string(token.type),
               token.value ? token.value : "",
               token.line, token.column);
               
        token_destroy(&token);
    } while (token.type != TOKEN_EOF);
    
    printf("\nTotal: %d tokens\n", count - 1); // -1 para não contar EOF
    
    lexer_destroy(lexer);
    free(source);
    return 0;
}