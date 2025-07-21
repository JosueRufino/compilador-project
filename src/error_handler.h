#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>

// Tipos de erro
typedef enum {
    ERROR_LEXICAL,
    ERROR_SYNTACTIC,
    ERROR_SEMANTIC,
    WARNING_SEMANTIC
} ErrorType;

// Estrutura para armazenar informações de erro
typedef struct Error {
    ErrorType type;
    char* message;
    char* filename;
    int line;
    int column;
    struct Error* next;
} Error;

// Gerenciador de erros
typedef struct ErrorHandler {
    Error* errors;
    int error_count;
    int warning_count;
    int max_errors;
    char* current_filename;
} ErrorHandler;

// Funções do gerenciador de erros
ErrorHandler* error_handler_create();
void error_handler_destroy(ErrorHandler* handler);
void error_handler_set_file(ErrorHandler* handler, const char* filename);

// Reportar erros
void report_error(ErrorHandler* handler, ErrorType type, const char* message, 
                 int line, int column);
void report_lexical_error(ErrorHandler* handler, const char* message, int line, int column);
void report_syntax_error(ErrorHandler* handler, const char* message, int line, int column);
void report_semantic_error(ErrorHandler* handler, const char* message, int line, int column);
void report_warning(ErrorHandler* handler, const char* message, int line, int column);

// Exibir erros
void error_handler_print_errors(ErrorHandler* handler);
void error_handler_print_summary(ErrorHandler* handler);

// Verificações
int error_handler_has_errors(ErrorHandler* handler);
int error_handler_should_continue(ErrorHandler* handler);

#endif
