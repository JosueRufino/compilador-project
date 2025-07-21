#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable* symbol_table_create() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    
    // Criar escopo global
    table->global_scope = malloc(sizeof(Scope));
    table->global_scope->symbols = NULL;
    table->global_scope->parent = NULL;
    table->global_scope->level = 0;
    table->global_scope->name = strdup("global");
    
    table->current_scope = table->global_scope;
    table->current_level = 0;
    
    return table;
}

static void scope_destroy(Scope* scope) {
    if (!scope) return;
    
    Symbol* current = scope->symbols;
    while (current) {
        Symbol* next = current->next;
        
        free(current->name);
        
        // Limpar informações específicas do símbolo
        if (current->kind == SYMBOL_FUNCTION) {
            free(current->info.function.parameter_types);
            if (current->info.function.parameter_names) {
                for (int i = 0; i < current->info.function.parameter_count; i++) {
                    free(current->info.function.parameter_names[i]);
                }
                free(current->info.function.parameter_names);
            }
        } else if (current->kind == SYMBOL_STRUCT) {
            if (current->info.structure.member_names) {
                for (int i = 0; i < current->info.structure.member_count; i++) {
                    free(current->info.structure.member_names[i]);
                }
                free(current->info.structure.member_names);
            }
            free(current->info.structure.member_types);
        }
        
        free(current);
        current = next;
    }
    
    free(scope->name);
    free(scope);
}

void symbol_table_destroy(SymbolTable* table) {
    if (!table) return;
    
    // Destruir todos os escopos
    Scope* current = table->current_scope;
    while (current) {
        Scope* parent = current->parent;
        scope_destroy(current);
        current = parent;
    }
    
    free(table);
}

void symbol_table_enter_scope(SymbolTable* table, const char* scope_name) {
    Scope* new_scope = malloc(sizeof(Scope));
    new_scope->symbols = NULL;
    new_scope->parent = table->current_scope;
    new_scope->level = table->current_level + 1;
    new_scope->name = strdup(scope_name);
    
    table->current_scope = new_scope;
    table->current_level++;
}

void symbol_table_exit_scope(SymbolTable* table) {
    if (table->current_scope == table->global_scope) {
        return; // Não pode sair do escopo global
    }
    
    Scope* old_scope = table->current_scope;
    table->current_scope = old_scope->parent;
    table->current_level--;
    
    scope_destroy(old_scope);
}

Symbol* symbol_table_lookup(SymbolTable* table, const char* name) {
    Scope* current_scope = table->current_scope;
    
    while (current_scope) {
        Symbol* symbol = current_scope->symbols;
        while (symbol) {
            if (strcmp(symbol->name, name) == 0) {
                return symbol;
            }
            symbol = symbol->next;
        }
        current_scope = current_scope->parent;
    }
    
    return NULL;
}

Symbol* symbol_table_lookup_current_scope(SymbolTable* table, const char* name) {
    Symbol* symbol = table->current_scope->symbols;
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}

int symbol_table_insert(SymbolTable* table, Symbol* symbol) {
    // Verificar se já existe no escopo atual
    if (symbol_table_lookup_current_scope(table, symbol->name)) {
        return 0; // Já existe
    }
    
    // Inserir no início da lista
    symbol->scope_level = table->current_level;
    symbol->next = table->current_scope->symbols;
    table->current_scope->symbols = symbol;
    
    return 1;
}

Symbol* symbol_create_variable(const char* name, DataType type, int line, int column) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->kind = SYMBOL_VARIABLE;
    symbol->type = type;
    symbol->line = line;
    symbol->column = column;
    symbol->next = NULL;
    
    // Inicializar informações da variável
    symbol->info.variable.is_initialized = 0;
    symbol->info.variable.is_const = 0;
    symbol->info.variable.is_static = 0;
    symbol->info.variable.offset = 0;
    
    return symbol;
}

Symbol* symbol_create_function(const char* name, DataType return_type, int line, int column) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->kind = SYMBOL_FUNCTION;
    symbol->type = return_type;
    symbol->line = line;
    symbol->column = column;
    symbol->next = NULL;
    
    // Inicializar informações da função
    symbol->info.function.return_type = return_type;
    symbol->info.function.parameter_count = 0;
    symbol->info.function.parameter_types = NULL;
    symbol->info.function.parameter_names = NULL;
    symbol->info.function.is_defined = 0;
    
    return symbol;
}

Symbol* symbol_create_struct(const char* name, int line, int column) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->kind = SYMBOL_STRUCT;
    symbol->type = TYPE_VOID; // Structs não têm tipo primitivo
    symbol->line = line;
    symbol->column = column;
    symbol->next = NULL;
    
    // Inicializar informações da estrutura
    symbol->info.structure.member_count = 0;
    symbol->info.structure.member_names = NULL;
    symbol->info.structure.member_types = NULL;
    symbol->info.structure.size = 0;
    
    return symbol;
}

void symbol_table_print(SymbolTable* table) {
    printf("=== TABELA DE SÍMBOLOS ===\n");
    
    Scope* scope = table->current_scope;
    while (scope) {
        printf("Escopo: %s (nível %d)\n", scope->name, scope->level);
        
        Symbol* symbol = scope->symbols;
        while (symbol) {
            printf("  %s: %s", symbol->name, symbol_kind_to_string(symbol->kind));
            
            if (symbol->kind == SYMBOL_VARIABLE || symbol->kind == SYMBOL_FUNCTION) {
                printf(" (tipo: %s)", data_type_to_string(symbol->type));
            }
            
            if (symbol->kind == SYMBOL_FUNCTION) {
                printf(" (params: %d)", symbol->info.function.parameter_count);
            }
            
            printf(" [linha %d]\n", symbol->line);
            symbol = symbol->next;
        }
        
        scope = scope->parent;
        if (scope) printf("\n");
    }
    printf("\n");
}

const char* symbol_kind_to_string(SymbolKind kind) {
    switch (kind) {
        case SYMBOL_VARIABLE: return "VARIABLE";
        case SYMBOL_FUNCTION: return "FUNCTION";
        case SYMBOL_PARAMETER: return "PARAMETER";
        case SYMBOL_STRUCT: return "STRUCT";
        case SYMBOL_ENUM: return "ENUM";
        case SYMBOL_TYPEDEF: return "TYPEDEF";
        default: return "UNKNOWN";
    }
}
