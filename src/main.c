#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "error_handler.h"
#include "code_generator.h"

typedef struct CompilerOptions {
    char* input_file;
    char* output_file;
    OutputType output_type;
    int verbose;
    int show_tokens;
    int show_ast;
    int show_symbols;
    int optimize;
} CompilerOptions;

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(length + 1);
    fread(content, 1, length, file);
    content[length] = '\0';
    
    fclose(file);
    return content;
}

void print_usage(const char* program_name) {
    printf("Uso: %s [opções] <arquivo.c>\n", program_name);
    printf("\nOpções:\n");
    printf("  -o <arquivo>    Arquivo de saída\n");
    printf("  -S              Gerar assembly\n");
    printf("  -c              Gerar código C (transpilação)\n");
    printf("  -b              Gerar bytecode\n");
    printf("  -v              Modo verboso\n");
    printf("  --tokens        Mostrar tokens\n");
    printf("  --ast           Mostrar AST\n");
    printf("  --symbols       Mostrar tabela de símbolos\n");
    printf("  -O              Otimizar código\n");
    printf("  -h, --help      Mostrar esta ajuda\n");
}

CompilerOptions parse_arguments(int argc, char* argv[]) {
    CompilerOptions options = {0};
    options.output_type = OUTPUT_C;
    options.output_file = "output.c";
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            options.output_file = argv[++i];
        } else if (strcmp(argv[i], "-S") == 0) {
            options.output_type = OUTPUT_ASSEMBLY;
            if (!strstr(options.output_file, ".")) {
                options.output_file = "output.s";
            }
        } else if (strcmp(argv[i], "-c") == 0) {
            options.output_type = OUTPUT_C;
        } else if (strcmp(argv[i], "-b") == 0) {
            options.output_type = OUTPUT_BYTECODE;
            if (!strstr(options.output_file, ".")) {
                options.output_file = "output.bc";
            }
        } else if (strcmp(argv[i], "-v") == 0) {
            options.verbose = 1;
        } else if (strcmp(argv[i], "--tokens") == 0) {
            options.show_tokens = 1;
        } else if (strcmp(argv[i], "--ast") == 0) {
            options.show_ast = 1;
        } else if (strcmp(argv[i], "--symbols") == 0) {
            options.show_symbols = 1;
        } else if (strcmp(argv[i], "-O") == 0) {
            options.optimize = 1;
        } else if (argv[i][0] != '-') {
            options.input_file = argv[i];
        }
    }
    
    return options;
}

void print_tokens(const char* source, int verbose) {
    if (!verbose) return;
    
    printf("=== ANÁLISE LÉXICA ===\n");
    
    Lexer* lexer = lexer_create(source);
    Token token;
    
    do {
        token = lexer_next_token(lexer);
        printf("%-15s", token_type_to_string(token.type));
        if (token.value) {
            printf(" %-15s", token.value);
        }
        printf(" [%d:%d]\n", token.line, token.column);
        token_destroy(&token);
    } while (token.type != TOKEN_EOF);
    
    lexer_destroy(lexer);
    printf("\n");
}

// Adicione esta função de debug ao main.c para diagnosticar o problema

void debug_tokens(const char* source) {
    printf("=== DEBUG DETALHADO DOS TOKENS ===\n");
    
    Lexer* lexer = lexer_create(source);
    Token token;
    int count = 0;
    
    do {
        token = lexer_next_token(lexer);
        printf("Token %d: %-15s", ++count, token_type_to_string(token.type));
        if (token.value) {
            printf(" valor='%s'", token.value);
        }
        printf(" [linha %d, coluna %d]\n", token.line, token.column);
        
        // Mostrar o caractere atual do lexer para debug
        if (lexer->position < lexer->length) {
            char current = lexer->source[lexer->position];
            if (current == '\n') {
                printf("    -> Próximo char: '\\n'\n");
            } else if (current == '\0') {
                printf("    -> Próximo char: EOF\n");
            } else {
                printf("    -> Próximo char: '%c'\n", current);
            }
        }
        
        token_destroy(&token);
    } while (token.type != TOKEN_EOF && count < 50); // Limite para evitar loop infinito
    
    lexer_destroy(lexer);
    printf("=== FIM DEBUG TOKENS ===\n\n");
}

// No main, antes da análise sintática, adicione:

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    CompilerOptions options = parse_arguments(argc, argv);
    
    if (!options.input_file) {
        fprintf(stderr, "Erro: Arquivo de entrada não especificado\n");
        return 1;
    }
    
    // Criar gerenciador de erros
    ErrorHandler* error_handler = error_handler_create();
    error_handler_set_file(error_handler, options.input_file);
    
    // Ler arquivo fonte
    char* source = read_file(options.input_file);
    if (!source) {
        error_handler_destroy(error_handler);
        return 1;
    }
    
    if (options.verbose) {
        printf("Compilador C - Processando: %s\n", options.input_file);
        printf("Arquivo fonte:\n%s\n", source);
        printf("Saída: %s\n", options.output_file);
        printf("========================================\n\n");
         debug_tokens(source);
    }

    
    // Fase 1: Análise Léxica
    if (options.show_tokens || options.verbose) {
        print_tokens(source, 1);
    }
    
    // Fase 2: Análise Sintática
    if (options.verbose) {
        printf("=== INICIANDO ANÁLISE SINTÁTICA ===\n");
    }
    
    Lexer* lexer = lexer_create(source);
    Parser* parser = parser_create(lexer);
    
    // Debug: mostrar primeiro token
    if (options.verbose) {
        printf("Primeiro token: %s", token_type_to_string(parser->current_token.type));
        if (parser->current_token.value) {
            printf(" (%s)", parser->current_token.value);
        }
        printf(" [%d:%d]\n", parser->current_token.line, parser->current_token.column);
    }
    
    ASTNode* ast = parser_parse(parser);
    
    if (parser->has_error) {
        printf("ERRO NO PARSER: %s\n", parser->error_message);
        report_syntax_error(error_handler, parser->error_message, 0, 0);
        error_handler_print_errors(error_handler);
        
        if (ast) ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        error_handler_destroy(error_handler);
        return 1;
    }
    
    if (options.verbose) {
        printf("✅ Análise sintática concluída!\n\n");
    }
    
    if (options.show_ast || options.verbose) {
        printf("=== ÁRVORE SINTÁTICA ===\n");
        if (ast) {
            ast_print(ast, 0);
        } else {
            printf("AST é NULL!\n");
        }
        printf("\n");
    }
    
    // Fase 3: Análise Semântica
    if (options.verbose) {
        printf("=== INICIANDO ANÁLISE SEMÂNTICA ===\n");
    }
    
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    
    if (!semantic_analyze(analyzer, ast)) {
        printf("ERRO SEMÂNTICO: %s\n", analyzer->error_message);
        report_semantic_error(error_handler, analyzer->error_message, 0, 0);
        error_handler_print_errors(error_handler);
        
        semantic_analyzer_destroy(analyzer);
        if (ast) ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        error_handler_destroy(error_handler);
        return 1;
    }
    
    if (options.show_symbols || options.verbose) {
        symbol_table_print(analyzer->symbol_table);
    }
    
    if (options.verbose) {
        printf("✅ Análise semântica concluída com sucesso!\n\n");
    }
    
    // Fase 4: Geração de Código
    if (options.verbose) {
        printf("=== INICIANDO GERAÇÃO DE CÓDIGO ===\n");
    }
    
    CodeGenerator* generator = code_generator_create(options.output_file, options.output_type);
    if (!generator) {
        fprintf(stderr, "Erro: Não foi possível criar arquivo de saída\n");
        semantic_analyzer_destroy(analyzer);
        if (ast) ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        error_handler_destroy(error_handler);
        return 1;
    }
    
    if (generate_code(generator, ast, analyzer->symbol_table)) {
        if (options.verbose) {
            printf("✅ Código gerado com sucesso em: %s\n", options.output_file);
        }
    } else {
        fprintf(stderr, "Erro na geração de código\n");
    }
    
    // Limpeza
    code_generator_destroy(generator);
    semantic_analyzer_destroy(analyzer);
    if (ast) ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);
    
    // Relatório final
    if (options.verbose || error_handler_has_errors(error_handler)) {
        error_handler_print_summary(error_handler);
    }
    
    int exit_code = error_handler_has_errors(error_handler) ? 1 : 0;
    error_handler_destroy(error_handler);
    
    return exit_code;
}