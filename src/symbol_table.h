#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

// Tipos de símbolos
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER,
    SYMBOL_STRUCT,
    SYMBOL_ENUM,
    SYMBOL_TYPEDEF
} SymbolKind;

// Informações sobre função
typedef struct FunctionInfo {
    DataType return_type;
    int parameter_count;
    DataType* parameter_types;
    char** parameter_names;
    int is_defined;  // Se foi apenas declarada ou também definida
} FunctionInfo;

// Informações sobre estrutura
typedef struct StructInfo {
    int member_count;
    char** member_names;
    DataType* member_types;
    int size;  // Tamanho em bytes
} StructInfo;

// Símbolo na tabela
typedef struct Symbol {
    char* name;
    SymbolKind kind;
    DataType type;
    int line;
    int column;
    int scope_level;
    
    union {
        struct {
            int is_initialized;
            int is_const;
            int is_static;
            int offset;  // Para geração de código
        } variable;
        
        FunctionInfo function;
        StructInfo structure;
        
        struct {
            DataType original_type;
        } typedef_info;
    } info;
    
    struct Symbol* next;
} Symbol;

// Escopo
typedef struct Scope {
    Symbol* symbols;
    struct Scope* parent;
    int level;
    char* name;  // Nome do escopo (função, bloco, etc.)
} Scope;

// Tabela de símbolos
typedef struct SymbolTable {
    Scope* current_scope;
    Scope* global_scope;
    int current_level;
} SymbolTable;

// Funções da tabela de símbolos
SymbolTable* symbol_table_create();
void symbol_table_destroy(SymbolTable* table);

// Gerenciamento de escopos
void symbol_table_enter_scope(SymbolTable* table, const char* scope_name);
void symbol_table_exit_scope(SymbolTable* table);

// Operações com símbolos
Symbol* symbol_table_lookup(SymbolTable* table, const char* name);
Symbol* symbol_table_lookup_current_scope(SymbolTable* table, const char* name);
int symbol_table_insert(SymbolTable* table, Symbol* symbol);

// Criação de símbolos
Symbol* symbol_create_variable(const char* name, DataType type, int line, int column);
Symbol* symbol_create_function(const char* name, DataType return_type, int line, int column);
Symbol* symbol_create_struct(const char* name, int line, int column);

// Utilitários
void symbol_table_print(SymbolTable* table);
const char* symbol_kind_to_string(SymbolKind kind);

#endif
