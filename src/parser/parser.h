#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current_token;
    int has_error;
    char error_message[256];
} Parser;

// Funções do analisador sintático
Parser* parser_create(Lexer* lexer);
void parser_destroy(Parser* parser);
ASTNode* parser_parse(Parser* parser);

// Funções auxiliares
void parser_error(Parser* parser, const char* message);
int parser_match(Parser* parser, TokenType type);
void parser_consume(Parser* parser, TokenType type, const char* error_msg);
void parser_advance(Parser* parser);

// Funções de parsing por precedência
ASTNode* parse_program(Parser* parser);
ASTNode* parse_declaration(Parser* parser);
ASTNode* parse_function_declaration(Parser* parser);
ASTNode* parse_variable_declaration(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_compound_statement(Parser* parser);
ASTNode* parse_expression_statement(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_while_statement(Parser* parser);
ASTNode* parse_return_statement(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_assignment_expression(Parser* parser);
ASTNode* parse_logical_or_expression(Parser* parser);
ASTNode* parse_logical_and_expression(Parser* parser);
ASTNode* parse_equality_expression(Parser* parser);
ASTNode* parse_relational_expression(Parser* parser);
ASTNode* parse_additive_expression(Parser* parser);
ASTNode* parse_multiplicative_expression(Parser* parser);
ASTNode* parse_unary_expression(Parser* parser);
ASTNode* parse_postfix_expression(Parser* parser);
ASTNode* parse_primary_expression(Parser* parser);

// Funções auxiliares para parsing com informações já parseadas
ASTNode* parse_function_declaration_with_info(Parser* parser, DataType return_type, char* name);
ASTNode* parse_variable_declaration_with_info(Parser* parser, DataType type, char* name);
ASTNode* parse_preprocessor_directive(Parser* parser);

#endif