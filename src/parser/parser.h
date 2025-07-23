#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// Estruturas para erros recuperados
typedef struct ErrorInfo {
    char message[256];
    int line;
    int column;
    struct ErrorInfo* next;
} ErrorInfo;

typedef struct {
    ErrorInfo* errors;
    int count;
} ParserErrorList;

typedef struct {
    Lexer* lexer;
    Token current_token;
    int has_error;
    char error_message[256];
    ParserErrorList* recovered_errors;
} Parser;

// Funções do parser
Parser* parser_create(Lexer* lexer);
void parser_destroy(Parser* parser);
void parser_add_recovered_error(Parser* parser, const char* message);
void parser_print_recovered_errors(Parser* parser);
void parser_error(Parser* parser, const char* message);
int parser_match(Parser* parser, TokenType type);
void parser_consume(Parser* parser, TokenType type, const char* error_msg);
void parser_advance(Parser* parser);
void parser_skip_to_recovery_point(Parser* parser);
ASTNode* parser_parse(Parser* parser);
ASTNode* parse_program(Parser* parser);  // Renomeado para consistência
ASTNode* parse_programa(Parser* parser); // Mantido como alias se necessário
ASTNode* parse_preprocessador(Parser* parser);
ASTNode* parse_declaracao_global(Parser* parser);
ASTNode* parse_declaracao_com_tipo(Parser* parser);
ASTNode* parse_definicao_funcao_com_info(Parser* parser, DataType tipo_retorno, char* nome);
ASTNode* parse_declaracao_variavel_com_info(Parser* parser, DataType tipo, char* nome);
ASTNode* parse_bloco(Parser* parser);
ASTNode* parse_item_bloco(Parser* parser);
ASTNode* parse_comando(Parser* parser);
ASTNode* parse_comando_if(Parser* parser);
ASTNode* parse_comando_sem_if(Parser* parser);
ASTNode* parse_comando_while(Parser* parser);
ASTNode* parse_comando_return(Parser* parser);
ASTNode* parse_comando_break(Parser* parser);
ASTNode* parse_comando_continue(Parser* parser);
ASTNode* parse_comando_expressao(Parser* parser);
ASTNode* parse_expressao(Parser* parser);
ASTNode* parse_expressao_virgula(Parser* parser);
ASTNode* parse_atribuicao(Parser* parser);
ASTNode* parse_condicional(Parser* parser);
ASTNode* parse_logico_ou(Parser* parser);
ASTNode* parse_logico_e(Parser* parser);
ASTNode* parse_igualdade(Parser* parser);
ASTNode* parse_relacional(Parser* parser);
ASTNode* parse_soma(Parser* parser);
ASTNode* parse_produto(Parser* parser);
ASTNode* parse_unario(Parser* parser);
ASTNode* parse_sufixo(Parser* parser);
ASTNode* parse_primario(Parser* parser);
ASTNode* parse_declaration(Parser* parser);
ASTNode* parse_function_declaration(Parser* parser);
ASTNode* parse_variable_declaration(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_compound_statement(Parser* parser);
ASTNode* parse_expression_statement(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_while_statement(Parser* parser);
ASTNode* parse_return_statement(Parser* parser);

#endif