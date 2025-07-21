#include "error_handler.h"
#include <stdlib.h>
#include <string.h>

ErrorHandler* error_handler_create() {
    ErrorHandler* handler = malloc(sizeof(ErrorHandler));
    handler->errors = NULL;
    handler->error_count = 0;
    handler->warning_count = 0;
    handler->max_errors = 50;  // Máximo de erros antes de parar
    handler->current_filename = NULL;
    return handler;
}

void error_handler_destroy(ErrorHandler* handler) {
    if (!handler) return;
    
    Error* current = handler->errors;
    while (current) {
        Error* next = current->next;
        free(current->message);
        free(current->filename);
        free(current);
        current = next;
    }
    
    free(handler->current_filename);
    free(handler);
}

void error_handler_set_file(ErrorHandler* handler, const char* filename) {
    free(handler->current_filename);
    handler->current_filename = filename ? strdup(filename) : NULL;
}

static void add_error(ErrorHandler* handler, ErrorType type, const char* message,
                     int line, int column) {
    Error* error = malloc(sizeof(Error));
    error->type = type;
    error->message = strdup(message);
    error->filename = handler->current_filename ? strdup(handler->current_filename) : NULL;
    error->line = line;
    error->column = column;
    error->next = handler->errors;
    handler->errors = error;
    
    if (type == WARNING_SEMANTIC) {
        handler->warning_count++;
    } else {
        handler->error_count++;
    }
}

void report_error(ErrorHandler* handler, ErrorType type, const char* message,
                 int line, int column) {
    add_error(handler, type, message, line, column);
}

void report_lexical_error(ErrorHandler* handler, const char* message, int line, int column) {
    add_error(handler, ERROR_LEXICAL, message, line, column);
}

void report_syntax_error(ErrorHandler* handler, const char* message, int line, int column) {
    add_error(handler, ERROR_SYNTACTIC, message, line, column);
}

void report_semantic_error(ErrorHandler* handler, const char* message, int line, int column) {
    add_error(handler, ERROR_SEMANTIC, message, line, column);
}

void report_warning(ErrorHandler* handler, const char* message, int line, int column) {
    add_error(handler, WARNING_SEMANTIC, message, line, column);
}

static const char* error_type_to_string(ErrorType type) {
    switch (type) {
        case ERROR_LEXICAL: return "Erro Léxico";
        case ERROR_SYNTACTIC: return "Erro Sintático";
        case ERROR_SEMANTIC: return "Erro Semântico";
        case WARNING_SEMANTIC: return "Aviso";
        default: return "Erro Desconhecido";
    }
}

void error_handler_print_errors(ErrorHandler* handler) {
    if (!handler->errors) return;
    
    printf("\n=== RELATÓRIO DE ERROS ===\n");
    
    // Inverter a lista para mostrar erros na ordem correta
    Error* reversed = NULL;
    Error* current = handler->errors;
    while (current) {
        Error* next = current->next;
        current->next = reversed;
        reversed = current;
        current = next;
    }
    
    current = reversed;
    while (current) {
        printf("%s", error_type_to_string(current->type));
        if (current->filename) {
            printf(" em %s", current->filename);
        }
        printf(" (linha %d, coluna %d): %s\n", 
               current->line, current->column, current->message);
        current = current->next;
    }
    
    // Restaurar a lista original
    handler->errors = reversed;
}

void error_handler_print_summary(ErrorHandler* handler) {
    printf("\n=== RESUMO DA COMPILAÇÃO ===\n");
    printf("Erros: %d\n", handler->error_count);
    printf("Avisos: %d\n", handler->warning_count);
    
    if (handler->error_count == 0) {
        printf("✅ Compilação bem-sucedida!\n");
    } else {
        printf("❌ Compilação falhou com %d erro(s)\n", handler->error_count);
    }
}

int error_handler_has_errors(ErrorHandler* handler) {
    return handler->error_count > 0;
}

int error_handler_should_continue(ErrorHandler* handler) {
    return handler->error_count < handler->max_errors;
}
