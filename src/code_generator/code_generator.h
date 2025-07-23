#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "ast.h"
#include "symbol_table.h"

// Tipos de código de saída
typedef enum {
    OUTPUT_C,           // Código C (transpilação)
    OUTPUT_ASSEMBLY,    // Assembly x86-64
    OUTPUT_BYTECODE     // Bytecode personalizado
} OutputType;

// Gerador de código
typedef struct CodeGenerator {
    FILE* output_file;
    OutputType output_type;
    SymbolTable* symbol_table;
    int label_counter;
    int temp_counter;
    int current_offset;  // Para variáveis locais
    char* current_function;
} CodeGenerator;

// Funções principais
CodeGenerator* code_generator_create(const char* output_filename, OutputType type);
void code_generator_destroy(CodeGenerator* generator);
int generate_code(CodeGenerator* generator, ASTNode* ast, SymbolTable* symbols);

// Geração específica por tipo de nó
void generate_program(CodeGenerator* gen, ASTNode* node);
void generate_function_declaration(CodeGenerator* gen, ASTNode* node);
void generate_variable_declaration(CodeGenerator* gen, ASTNode* node);
void generate_statement(CodeGenerator* gen, ASTNode* node);
void generate_expression(CodeGenerator* gen, ASTNode* node);

// Utilitários
char* generate_label(CodeGenerator* gen, const char* prefix);
char* generate_temp_var(CodeGenerator* gen);
void emit_code(CodeGenerator* gen, const char* format, ...);
void emit_comment(CodeGenerator* gen, const char* comment);

#endif
