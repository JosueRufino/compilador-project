#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// ADICIONE ESTAS ESTRUTURAS AQUI:
typedef struct ErrorInfo {
    char message[256];
    int line;
    int column;
    struct ErrorInfo* next;
} ErrorInfo;

typedef struct ParserErrorList {
    ErrorInfo* errors;
    int count;
} ParserErrorList;

// MODIFIQUE a estrutura Parser existente:
typedef struct {
    Lexer* lexer;
    Token current_token;
    int has_error;
    char error_message[256];
    ParserErrorList* recovered_errors;  // ADICIONE ESTA LINHA
} Parser;


// =================================================================
// FUNÇÕES PRINCIPAIS DO PARSER
// =================================================================

// Funções básicas do analisador sintático
Parser* parser_create(Lexer* lexer);
void parser_destroy(Parser* parser);
ASTNode* parser_parse(Parser* parser);

// Funções auxiliares
void parser_error(Parser* parser, const char* message);
int parser_match(Parser* parser, TokenType type);
void parser_consume(Parser* parser, TokenType type, const char* error_msg);
void parser_advance(Parser* parser);

// =================================================================
// FUNÇÕES SEGUINDO A GRAMÁTICA 
// =================================================================

// Estrutura do programa
ASTNode* parse_programa(Parser* parser);
ASTNode* parse_preprocessador(Parser* parser);
ASTNode* parse_declaracao_global(Parser* parser);

// Declarações
ASTNode* parse_declaracao_com_tipo(Parser* parser);
ASTNode* parse_definicao_funcao_com_info(Parser* parser, DataType tipo_retorno, char* nome);
ASTNode* parse_declaracao_variavel_com_info(Parser* parser, DataType tipo, char* nome);

// Comandos
ASTNode* parse_bloco(Parser* parser);
ASTNode* parse_item_bloco(Parser* parser);
ASTNode* parse_comando(Parser* parser);
ASTNode* parse_comando_sem_if(Parser* parser);
ASTNode* parse_comando_if(Parser* parser);
ASTNode* parse_comando_while(Parser* parser);
ASTNode* parse_comando_return(Parser* parser);
ASTNode* parse_comando_break(Parser* parser);
ASTNode* parse_comando_continue(Parser* parser);
ASTNode* parse_comando_expressao(Parser* parser);

// Expressões (por precedência - da menor para maior)
ASTNode* parse_expressao(Parser* parser);              // Nível mais alto
ASTNode* parse_expressao_virgula(Parser* parser);      // vírgula (menor precedência)
ASTNode* parse_atribuicao(Parser* parser);             // = (associativo à direita)
ASTNode* parse_condicional(Parser* parser);            // ?: (ternário)
ASTNode* parse_logico_ou(Parser* parser);              // ||
ASTNode* parse_logico_e(Parser* parser);               // &&
ASTNode* parse_igualdade(Parser* parser);              // == !=
ASTNode* parse_relacional(Parser* parser);             // < > <= >=
ASTNode* parse_soma(Parser* parser);                   // + -
ASTNode* parse_produto(Parser* parser);                // * / %
ASTNode* parse_unario(Parser* parser);                 // + - ! ~ ++ --
ASTNode* parse_sufixo(Parser* parser);                 // () [] . -> ++ --
ASTNode* parse_primario(Parser* parser);               // identificadores, literais, ()

// =================================================================
// FUNÇÕES DE COMPATIBILIDADE 
// =================================================================

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

// ADICIONE ESTAS LINHAS AQUI:
// Funções para gerenciamento de erros recuperados
void parser_add_recovered_error(Parser *parser, const char *message);
void parser_print_recovered_errors(Parser *parser);


#endif