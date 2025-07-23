#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

typedef struct SemanticAnalyzer {
    SymbolTable* symbol_table;
    int has_error;
    char error_message[512];
    DataType current_function_return_type;
    int in_loop;  // Para verificar break/continue
    int error_count;
    int warning_count;
} SemanticAnalyzer;

// Funções do analisador semântico
SemanticAnalyzer* semantic_analyzer_create();
void semantic_analyzer_destroy(SemanticAnalyzer* analyzer);
int semantic_analyze(SemanticAnalyzer* analyzer, ASTNode* ast);

// Funções auxiliares
void semantic_error(SemanticAnalyzer* analyzer, const char* message, int line, int column);
void semantic_warning(SemanticAnalyzer* analyzer, const char* message, int line, int column);
DataType analyze_expression(SemanticAnalyzer* analyzer, ASTNode* expr);
void analyze_statement(SemanticAnalyzer* analyzer, ASTNode* stmt);
void analyze_declaration(SemanticAnalyzer* analyzer, ASTNode* decl);
int check_type_compatibility(DataType type1, DataType type2);
DataType get_binary_operation_result_type(DataType left, DataType right, TokenType op);

#endif