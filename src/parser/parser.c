#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser *parser_create(Lexer *lexer)
{
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->has_error = 0;
    parser->error_message[0] = '\0';
    parser->recovered_errors = malloc(sizeof(ParserErrorList));
    parser->recovered_errors->errors = NULL;
    parser->recovered_errors->count = 0;
    parser->current_token = lexer_next_token(lexer);
    return parser;
}

// parser.c (trecho)
void parser_destroy(Parser* parser) {
    if (parser) {
        token_destroy(&parser->current_token);
        if (parser->recovered_errors) {
            ErrorInfo* current = parser->recovered_errors->errors;
            while (current) {
                ErrorInfo* next = current->next;
                free(current);
                current = next;
            }
            free(parser->recovered_errors);
        }
        free(parser);
    }
}

void parser_add_recovered_error(Parser *parser, const char *message)
{
    ErrorInfo *new_error = malloc(sizeof(ErrorInfo));
    snprintf(new_error->message, sizeof(new_error->message), "%s", message);
    new_error->line = parser->current_token.line;
    new_error->column = parser->current_token.column;
    new_error->next = NULL;

    if (parser->recovered_errors->errors == NULL)
    {
        parser->recovered_errors->errors = new_error;
    }
    else
    {
        ErrorInfo *current = parser->recovered_errors->errors;
        while (current->next)
        {
            current = current->next;
        }
        current->next = new_error;
    }
    parser->recovered_errors->count++;
}

void parser_print_recovered_errors(Parser *parser)
{
    if (!parser->recovered_errors || parser->recovered_errors->count == 0)
    {
        printf("Nenhum erro sintático detectado.\n");
        return;
    }

    printf("\n📋 Erros Sintáticos Encontrados (%d):\n", parser->recovered_errors->count);
    ErrorInfo *current = parser->recovered_errors->errors;
    while (current)
    {
        printf("   • Linha %d, Coluna %d: %s\n", current->line, current->column, current->message);
        current = current->next;
    }
}

void parser_error(Parser *parser, const char *message)
{
    char full_message[256];
    snprintf(full_message, sizeof(full_message),
             "Erro sintático na linha %d, coluna %d: %s",
             parser->current_token.line, parser->current_token.column, message);
    parser_add_recovered_error(parser, full_message);
    parser->has_error = 1; // Ainda usamos para sinalizar erro, mas não paramos
}

int parser_match(Parser *parser, TokenType type)
{
    return parser->current_token.type == type;
}

void parser_consume(Parser *parser, TokenType type, const char *error_msg)
{
    if (parser->current_token.type == type)
    {
        parser_advance(parser);
    }
    else
    {
        parser_error(parser, error_msg);
        parser_skip_to_recovery_point(parser); // Tenta recuperar
    }
}

void parser_advance(Parser *parser)
{
    token_destroy(&parser->current_token);
    parser->current_token = lexer_next_token(parser->lexer);
}

// parser.c (trecho)
void parser_skip_to_recovery_point(Parser *parser)
{
    while (!parser_match(parser, TOKEN_EOF) &&
           !parser_match(parser, TOKEN_SEMICOLON) &&
           !parser_match(parser, TOKEN_RBRACE) &&
           !parser_match(parser, TOKEN_INT) &&
           !parser_match(parser, TOKEN_FLOAT_KW) &&
           !parser_match(parser, TOKEN_CHAR_KW) &&
           !parser_match(parser, TOKEN_VOID))
    {
        parser_advance(parser);
    }
    if (parser_match(parser, TOKEN_SEMICOLON) || parser_match(parser, TOKEN_RBRACE))
    {
        parser_advance(parser);
    }
}

ASTNode *parser_parse(Parser *parser)
{
    ASTNode *programa = parse_programa(parser);
    parser_print_recovered_errors(parser); // Exibe todos os erros no final
    return programa;
}

// ... (O restante do parser.c permanece igual, mas com modificações nas funções de parsing para continuar após erros)

ASTNode *parse_programa(Parser *parser)
{
    ASTNode *programa = ast_create_node(AST_PROGRAM);

    while (parser_match(parser, TOKEN_HASH) && !parser_match(parser, TOKEN_EOF))
    {
        ASTNode *prep = parse_preprocessador(parser);
        if (prep)
        {
            ast_add_child(programa, prep);
        }
    }

    while (!parser_match(parser, TOKEN_EOF))
    {
        ASTNode *decl = parse_declaracao_global(parser);
        if (decl)
        {
            ast_add_child(programa, decl);
        }
        else
        {
            if (parser->has_error)
            {
                parser->has_error = 0; // Reseta o erro para continuar
                parser_skip_to_recovery_point(parser);
            }
            else
            {
                break; // EOF ou fim natural
            }
        }
    }

    return programa;
}

ASTNode *parse_preprocessador(Parser *parser)
{
    if (!parser_match(parser, TOKEN_HASH))
    {
        return NULL;
    }

    ASTNode *prep = ast_create_node(AST_PREPROCESSOR_DIRECTIVE);
    parser_advance(parser);

    if (parser_match(parser, TOKEN_INCLUDE))
    {
        prep->data.preprocessor.directive = strdup("include");
        parser_advance(parser);

        if (parser_match(parser, TOKEN_LESS) || parser_match(parser, TOKEN_STRING))
        {
            char *content = malloc(256);
            content[0] = '\0';
            if (parser_match(parser, TOKEN_LESS))
            {
                parser_advance(parser);
                if (parser_match(parser, TOKEN_IDENTIFIER))
                {
                    strcat(content, parser->current_token.value);
                    parser_advance(parser);
                    if (parser_match(parser, TOKEN_DOT))
                    {
                        strcat(content, ".");
                        parser_advance(parser);
                        if (parser_match(parser, TOKEN_IDENTIFIER))
                        {
                            strcat(content, parser->current_token.value);
                            parser_advance(parser);
                        }
                    }
                    if (!parser_match(parser, TOKEN_GREATER))
                    {
                        parser_error(parser, "Esperado '>' após nome do arquivo");
                    }
                    else
                    {
                        parser_advance(parser);
                    }
                }
                else
                {
                    parser_error(parser, "Esperado identificador após '<'");
                }
            }
            else
            {
                strcat(content, parser->current_token.value);
                parser_advance(parser);
            }
            prep->data.preprocessor.content = content;
        }
        else
        {
            parser_error(parser, "Esperado '<' ou string após 'include'");
        }
    }
    else
    {
        prep->data.preprocessor.directive = strdup("unknown");
        prep->data.preprocessor.content = strdup("");
        while (!parser_match(parser, TOKEN_EOF) &&
               !parser_match(parser, TOKEN_INT) &&
               !parser_match(parser, TOKEN_FLOAT_KW) &&
               !parser_match(parser, TOKEN_CHAR_KW) &&
               !parser_match(parser, TOKEN_VOID) &&
               !parser_match(parser, TOKEN_HASH))
        {
            parser_advance(parser);
        }
    }

    return prep;
}

ASTNode *parse_declaracao_global(Parser *parser)
{
    if (parser_match(parser, TOKEN_EOF))
    {
        return NULL;
    }

    if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
        parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
    {
        return parse_declaracao_com_tipo(parser);
    }

    parser_error(parser, "Esperado tipo de dados para declaração");
    return NULL;
}

ASTNode *parse_declaracao_com_tipo(Parser *parser)
{
    DataType tipo = TYPE_VOID;
    if (parser_match(parser, TOKEN_INT))
        tipo = TYPE_INT;
    else if (parser_match(parser, TOKEN_FLOAT_KW))
        tipo = TYPE_FLOAT;
    else if (parser_match(parser, TOKEN_CHAR_KW))
        tipo = TYPE_CHAR;
    else if (parser_match(parser, TOKEN_VOID))
        tipo = TYPE_VOID;

    parser_advance(parser);

    if (!parser_match(parser, TOKEN_IDENTIFIER))
    {
        if (parser_match(parser, TOKEN_SEMICOLON))
        {
            parser_error(parser, "Declaração incompleta: esperado identificador após tipo");
        }
        else
        {
            parser_error(parser, "Esperado identificador após tipo");
        }
        return NULL;
    }

    char *nome = strdup(parser->current_token.value);
    parser_advance(parser);

    if (parser_match(parser, TOKEN_LPAREN))
    {
        ASTNode *funcao = parse_definicao_funcao_com_info(parser, tipo, nome);
        if (!funcao && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
        }
        return funcao;
    }
    else
    {
        ASTNode *var = parse_declaracao_variavel_com_info(parser, tipo, nome);
        if (!var && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
        }
        return var;
    }
}

ASTNode *parse_definicao_funcao_com_info(Parser *parser, DataType tipo_retorno, char *nome)
{
    ASTNode *funcao = ast_create_node(AST_FUNCTION_DECLARATION);
    funcao->data.function_decl.name = nome;
    funcao->data.function_decl.return_type = tipo_retorno;
    funcao->data.function_decl.parameters = NULL;

    parser_advance(parser);

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        ASTNode *param_list = ast_create_node(AST_PARAMETER_LIST);
        do
        {
            if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
                parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
            {
                DataType param_type = TYPE_VOID;
                switch (parser->current_token.type)
                {
                case TOKEN_INT:
                    param_type = TYPE_INT;
                    break;
                case TOKEN_FLOAT_KW:
                    param_type = TYPE_FLOAT;
                    break;
                case TOKEN_CHAR_KW:
                    param_type = TYPE_CHAR;
                    break;
                case TOKEN_VOID:
                    param_type = TYPE_VOID;
                    break;
                }
                parser_advance(parser);

                if (!parser_match(parser, TOKEN_IDENTIFIER))
                {
                    parser_error(parser, "Esperado identificador para parâmetro");
                    ast_destroy(funcao);
                    ast_destroy(param_list);
                    return NULL;
                }

                ASTNode *param = ast_create_node(AST_PARAMETER);
                param->data.parameter.name = strdup(parser->current_token.value);
                param->data.parameter.param_type = param_type;
                parser_advance(parser);
                ast_add_child(param_list, param);

                if (parser_match(parser, TOKEN_COMMA))

                {
                    parser_advance(parser);
                }
                else
                {
                    break;
                }
            }
            else
            {
                parser_error(parser, "Esperado tipo de parâmetro");
                ast_destroy(funcao);
                ast_destroy(param_list);
                return NULL;
            }
        } while (!parser_match(parser, TOKEN_RPAREN));

        funcao->data.function_decl.parameters = param_list;
    }

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após parâmetros");
        ast_destroy(funcao);
        return NULL;
    }
    parser_advance(parser);

    funcao->data.function_decl.body = parse_bloco(parser);
    if (!funcao->data.function_decl.body && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        ast_destroy(funcao);
        return NULL;
    }

    return funcao;
}

ASTNode *parse_declaracao_variavel_com_info(Parser *parser, DataType tipo, char *nome)
{
    ASTNode *var = ast_create_node(AST_VARIABLE_DECLARATION);
    var->data.var_decl.name = nome;
    var->data.var_decl.var_type = tipo;
    var->data.var_decl.initializer = NULL;

    if (parser_match(parser, TOKEN_ASSIGN))
    {
        parser_advance(parser);
        var->data.var_decl.initializer = parse_expressao(parser);
        if (!var->data.var_decl.initializer && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(var);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após declaração de variável");
        ast_destroy(var);
        return NULL;
    }
    parser_advance(parser);

    return var;
}

ASTNode *parse_bloco(Parser *parser)
{
    if (!parser_match(parser, TOKEN_LBRACE))
    {
        parser_error(parser, "Esperado '{'");
        return NULL;
    }

    ASTNode *bloco = ast_create_node(AST_COMPOUND_STATEMENT);
    parser_advance(parser);

    while (!parser_match(parser, TOKEN_RBRACE) && !parser_match(parser, TOKEN_EOF))
    {
        ASTNode *item = parse_item_bloco(parser);
        if (item)
        {
            ast_add_child(bloco, item);
        }
        else if (parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            parser_skip_to_recovery_point(parser);
        }
        else
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_RBRACE))
    {
        parser_error(parser, "Esperado '}'");
        ast_destroy(bloco);
        return NULL;
    }
    parser_advance(parser);

    return bloco;
}

ASTNode *parse_item_bloco(Parser *parser)
{
    if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
        parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
    {
        TokenType tipo_token = parser->current_token.type;
        parser_advance(parser);

        if (parser_match(parser, TOKEN_IDENTIFIER))
        {
            DataType tipo = TYPE_VOID;
            switch (tipo_token)
            {
            case TOKEN_INT:
                tipo = TYPE_INT;
                break;
            case TOKEN_FLOAT_KW:
                tipo = TYPE_FLOAT;
                break;
            case TOKEN_CHAR_KW:
                tipo = TYPE_CHAR;
                break;
            case TOKEN_VOID:
                tipo = TYPE_VOID;
                break;
            }

            char *nome = strdup(parser->current_token.value);
            parser_advance(parser);
            return parse_declaracao_variavel_com_info(parser, tipo, nome);
        }
        else if (parser_match(parser, TOKEN_SEMICOLON))
        {
            parser_error(parser, "Declaração incompleta: esperado identificador após tipo");
            return NULL;
        }
        else
        {
            parser_error(parser, "Esperado identificador após tipo em declaração");
            return NULL;
        }
    }

    return parse_comando(parser);
}

ASTNode *parse_comando(Parser *parser)
{
    if (parser_match(parser, TOKEN_IF))
    {
        return parse_comando_if(parser);
    }
    else
    {
        return parse_comando_sem_if(parser);
    }
}

ASTNode *parse_comando_sem_if(Parser *parser)
{
    if (parser_match(parser, TOKEN_LBRACE))
    {
        return parse_bloco(parser);
    }
    else if (parser_match(parser, TOKEN_WHILE))
    {
        return parse_comando_while(parser);
    }
    else if (parser_match(parser, TOKEN_RETURN))
    {
        return parse_comando_return(parser);
    }
    else if (parser_match(parser, TOKEN_BREAK))
    {
        return parse_comando_break(parser);
    }
    else if (parser_match(parser, TOKEN_CONTINUE))
    {
        return parse_comando_continue(parser);
    }
    else
    {
        return parse_comando_expressao(parser);
    }
}

ASTNode *parse_comando_if(Parser *parser)
{
    ASTNode *if_stmt = ast_create_node(AST_IF_STATEMENT);
    parser_advance(parser);

    if (!parser_match(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Esperado '(' após 'if'");
        ast_destroy(if_stmt);
        return NULL;
    }
    parser_advance(parser);

    if_stmt->data.if_stmt.condition = parse_expressao(parser);
    if (!if_stmt->data.if_stmt.condition && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        ast_destroy(if_stmt);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após condição");
        ast_destroy(if_stmt);
        return NULL;
    }
    parser_advance(parser);

    if_stmt->data.if_stmt.then_stmt = parse_comando(parser);
    if (!if_stmt->data.if_stmt.then_stmt && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        ast_destroy(if_stmt);
        return NULL;
    }

    if (parser_match(parser, TOKEN_ELSE))
    {
        parser_advance(parser);
        if_stmt->data.if_stmt.else_stmt = parse_comando(parser);
        if (!if_stmt->data.if_stmt.else_stmt && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(if_stmt);
            return NULL;
        }
    }

    return if_stmt;
}

ASTNode *parse_comando_while(Parser *parser)
{
    ASTNode *while_stmt = ast_create_node(AST_WHILE_STATEMENT);
    parser_advance(parser);

    if (!parser_match(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Esperado '(' após 'while'");
        ast_destroy(while_stmt);
        return NULL;
    }
    parser_advance(parser);

    while_stmt->data.while_stmt.condition = parse_expressao(parser);
    if (!while_stmt->data.while_stmt.condition && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        ast_destroy(while_stmt);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após condição");
        ast_destroy(while_stmt);
        return NULL;
    }
    parser_advance(parser);

    while_stmt->data.while_stmt.body = parse_comando(parser);
    if (!while_stmt->data.while_stmt.body && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        ast_destroy(while_stmt);
        return NULL;
    }

    return while_stmt;
}

ASTNode *parse_comando_return(Parser *parser)
{
    ASTNode *return_stmt = ast_create_node(AST_RETURN_STATEMENT);
    parser_advance(parser);

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        return_stmt->data.return_stmt.expression = parse_expressao(parser);
        if (!return_stmt->data.return_stmt.expression && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(return_stmt);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após 'return'");
        ast_destroy(return_stmt);
        return NULL;
    }
    parser_advance(parser);

    return return_stmt;
}

ASTNode *parse_comando_break(Parser *parser)
{
    ASTNode *break_stmt = ast_create_node(AST_BREAK_STATEMENT);
    parser_advance(parser);

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após 'break'");
        ast_destroy(break_stmt);
        return NULL;
    }
    parser_advance(parser);

    return break_stmt;
}

ASTNode *parse_comando_continue(Parser *parser)
{
    ASTNode *continue_stmt = ast_create_node(AST_CONTINUE_STATEMENT);
    parser_advance(parser);

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após 'continue'");
        ast_destroy(continue_stmt);
        return NULL;
    }
    parser_advance(parser);

    return continue_stmt;
}

ASTNode *parse_comando_expressao(Parser *parser)
{
    ASTNode *expr_stmt = ast_create_node(AST_EXPRESSION_STATEMENT);

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        ASTNode *expr = parse_expressao(parser);
        if (expr)
        {
            ast_add_child(expr_stmt, expr);
        }
        else if (parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(expr_stmt);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após expressão");
        ast_destroy(expr_stmt);
        return NULL;
    }
    parser_advance(parser);

    return expr_stmt;
}

ASTNode *parse_expressao(Parser *parser)
{
    return parse_expressao_virgula(parser);
}

ASTNode *parse_expressao_virgula(Parser *parser)
{
    ASTNode *expr = parse_atribuicao(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_COMMA))
    {
        ASTNode *comma_expr = ast_create_node(AST_BINARY_EXPRESSION);
        comma_expr->data.binary_expr.left = expr;
        comma_expr->data.binary_expr.operator = TOKEN_COMMA;
        parser_advance(parser);
        comma_expr->data.binary_expr.right = parse_atribuicao(parser);
        if (!comma_expr->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(comma_expr);
            return expr;
        }
        expr = comma_expr;
    }

    return expr;
}

ASTNode *parse_atribuicao(Parser *parser)
{
    ASTNode *expr = parse_condicional(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    if (parser_match(parser, TOKEN_ASSIGN))
    {
        ASTNode *assign = ast_create_node(AST_ASSIGNMENT_EXPRESSION);
        assign->data.binary_expr.left = expr;
        assign->data.binary_expr.operator = TOKEN_ASSIGN;
        parser_advance(parser);
        assign->data.binary_expr.right = parse_atribuicao(parser);
        if (!assign->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(assign);
            return NULL;
        }
        return assign;
    }

    return expr;
}

ASTNode *parse_condicional(Parser *parser)
{
    ASTNode *expr = parse_logico_ou(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    if (parser_match(parser, TOKEN_QUESTION))
    {
        ASTNode *ternary = ast_create_node(AST_TERNARY_EXPRESSION);
        ternary->data.ternary_expr.condition = expr;
        parser_advance(parser);
        ternary->data.ternary_expr.true_expr = parse_expressao(parser);
        if (!ternary->data.ternary_expr.true_expr && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(ternary);
            return NULL;
        }

        if (!parser_match(parser, TOKEN_COLON))
        {
            parser_error(parser, "Esperado ':' em operador ternário");
            ast_destroy(ternary);
            return NULL;
        }
        parser_advance(parser);
        ternary->data.ternary_expr.false_expr = parse_condicional(parser);
        if (!ternary->data.ternary_expr.false_expr && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(ternary);
            return NULL;
        }
        return ternary;
    }

    return expr;
}

ASTNode *parse_logico_ou(Parser *parser)
{
    ASTNode *expr = parse_logico_e(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_OR))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = TOKEN_OR;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_logico_e(parser);
        if (!binary->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(binary);
            return expr;
        }
        expr = binary;
    }

    return expr;
}

ASTNode *parse_logico_e(Parser *parser)
{
    ASTNode *expr = parse_igualdade(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_AND))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = TOKEN_AND;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_igualdade(parser);
        if (!binary->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(binary);
            return expr;
        }
        expr = binary;
    }

    return expr;
}

ASTNode *parse_igualdade(Parser *parser)
{
    ASTNode *expr = parse_relacional(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_EQUAL) || parser_match(parser, TOKEN_NOT_EQUAL))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_relacional(parser);
        if (!binary->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(binary);
            return expr;
        }
        expr = binary;
    }

    return expr;
}

ASTNode *parse_relacional(Parser *parser)
{
    ASTNode *expr = parse_soma(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_LESS) || parser_match(parser, TOKEN_GREATER) ||
           parser_match(parser, TOKEN_LESS_EQUAL) || parser_match(parser, TOKEN_GREATER_EQUAL))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_soma(parser);
        if (!binary->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(binary);
            return expr;
        }
        expr = binary;
    }

    return expr;
}

ASTNode *parse_soma(Parser *parser)
{
    ASTNode *expr = parse_produto(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_PLUS) || parser_match(parser, TOKEN_MINUS))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_produto(parser);
        if (!binary->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(binary);
            return expr;
        }
        expr = binary;
    }

    return expr;
}

ASTNode *parse_produto(Parser *parser)
{
    ASTNode *expr = parse_unario(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_MULTIPLY) || parser_match(parser, TOKEN_DIVIDE) ||
           parser_match(parser, TOKEN_MODULO))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_unario(parser);
        if (!binary->data.binary_expr.right && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(binary);
            return expr;
        }
        expr = binary;
    }

    return expr;
}

ASTNode *parse_unario(Parser *parser)
{
    if (parser_match(parser, TOKEN_PLUS) || parser_match(parser, TOKEN_MINUS) ||
        parser_match(parser, TOKEN_NOT) || parser_match(parser, TOKEN_BITWISE_NOT) ||
        parser_match(parser, TOKEN_INCREMENT) || parser_match(parser, TOKEN_DECREMENT))
    {
        ASTNode *unary = ast_create_node(AST_UNARY_EXPRESSION);
        switch (parser->current_token.type)
        {
        case TOKEN_PLUS:
            unary->data.unary_expr.operator = UNARY_PLUS;
            break;
        case TOKEN_MINUS:
            unary->data.unary_expr.operator = UNARY_MINUS;
            break;
        case TOKEN_NOT:
            unary->data.unary_expr.operator = UNARY_NOT;
            break;
        case TOKEN_BITWISE_NOT:
            unary->data.unary_expr.operator = UNARY_BITWISE_NOT;
            break;
        case TOKEN_INCREMENT:
            unary->data.unary_expr.operator = UNARY_PRE_INCREMENT;
            break;
        case TOKEN_DECREMENT:
            unary->data.unary_expr.operator = UNARY_PRE_DECREMENT;
            break;
        }
        parser_advance(parser);
        unary->data.unary_expr.operand = parse_unario(parser);
        if (!unary->data.unary_expr.operand && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            ast_destroy(unary);
            return NULL;
        }
        return unary;
    }

    return parse_sufixo(parser);
}

ASTNode *parse_sufixo(Parser *parser)
{
    ASTNode *expr = parse_primario(parser);
    if (!expr && parser->has_error)
    {
        parser->has_error = 0; // Reseta para continuar
        return NULL;
    }

    while (parser_match(parser, TOKEN_LPAREN) || parser_match(parser, TOKEN_LBRACKET) ||
           parser_match(parser, TOKEN_DOT) || parser_match(parser, TOKEN_ARROW) ||
           parser_match(parser, TOKEN_INCREMENT) || parser_match(parser, TOKEN_DECREMENT))
    {
        if (parser_match(parser, TOKEN_LPAREN))
        {
            ASTNode *call = ast_create_node(AST_FUNCTION_CALL);
            if (expr && expr->type == AST_IDENTIFIER)
            {
                call->data.function_call.name = strdup(expr->data.identifier.name);
            }
            else
            {
                call->data.function_call.name = strdup("unknown");
            }
            parser_advance(parser);

            if (!parser_match(parser, TOKEN_RPAREN))
            {
                do
                {
                    ASTNode *arg = parse_atribuicao(parser);
                    if (arg)
                    {
                        ast_add_child(call, arg);
                    }
                    else if (parser->has_error)
                    {
                        parser->has_error = 0; // Reseta para continuar
                    }
                    if (parser_match(parser, TOKEN_COMMA))
                    {
                        parser_advance(parser);
                    }
                    else
                    {
                        break;
                    }
                } while (!parser_match(parser, TOKEN_RPAREN));
            }

            if (!parser_match(parser, TOKEN_RPAREN))
            {
                parser_error(parser, "Esperado ')' após argumentos");
                ast_destroy(call);
                return expr;
            }
            parser_advance(parser);
            if (expr)
                ast_destroy(expr);
            expr = call;
        }
        else
        {
            break; // TODO: implementar [], ., ->, ++, --
        }
    }

    return expr;
}

ASTNode *parse_primario(Parser *parser)
{
    if (parser_match(parser, TOKEN_IDENTIFIER))
    {
        ASTNode *id = ast_create_node(AST_IDENTIFIER);
        id->data.identifier.name = strdup(parser->current_token.value);
        parser_advance(parser);
        return id;
    }

    if (parser_match(parser, TOKEN_NUMBER))
    {
        ASTNode *num = ast_create_node(AST_NUMBER_LITERAL);
        num->data.literal.value = strdup(parser->current_token.value);
        parser_advance(parser);
        return num;
    }

    if (parser_match(parser, TOKEN_FLOAT))
    {
        ASTNode *flt = ast_create_node(AST_FLOAT_LITERAL);
        flt->data.literal.value = strdup(parser->current_token.value);
        parser_advance(parser);
        return flt;
    }

    if (parser_match(parser, TOKEN_STRING))
    {
        ASTNode *str = ast_create_node(AST_STRING_LITERAL);
        str->data.literal.value = strdup(parser->current_token.value);
        parser_advance(parser);
        return str;
    }

    if (parser_match(parser, TOKEN_CHAR))
    {
        ASTNode *chr = ast_create_node(AST_CHAR_LITERAL);
        chr->data.literal.value = strdup(parser->current_token.value);
        parser_advance(parser);
        return chr;
    }

    if (parser_match(parser, TOKEN_LPAREN))
    {
        parser_advance(parser);
        ASTNode *expr = parse_expressao(parser);
        if (!expr && parser->has_error)
        {
            parser->has_error = 0; // Reseta para continuar
            return NULL;
        }

        if (!parser_match(parser, TOKEN_RPAREN))
        {
            parser_error(parser, "Esperado ')' após expressão");
            if (expr)
                ast_destroy(expr);
            return NULL;
        }
        parser_advance(parser);
        return expr;
    }

    parser_error(parser, "Expressão primária inválida");
    return NULL;
}

// Funções de compatibilidade
ASTNode *parse_program(Parser *parser) { return parse_programa(parser); }
ASTNode *parse_declaration(Parser *parser) { return parse_declaracao_global(parser); }
ASTNode *parse_function_declaration(Parser *parser) { return NULL; }
ASTNode *parse_variable_declaration(Parser *parser) { return NULL; }
ASTNode *parse_statement(Parser *parser) { return parse_comando(parser); }
ASTNode *parse_compound_statement(Parser *parser) { return parse_bloco(parser); }
ASTNode *parse_expression_statement(Parser *parser) { return parse_comando_expressao(parser); }
ASTNode *parse_if_statement(Parser *parser) { return parse_comando_if(parser); }
ASTNode *parse_while_statement(Parser *parser) { return parse_comando_while(parser); }
ASTNode *parse_return_statement(Parser *parser) { return parse_comando_return(parser); }
ASTNode *parse_expression(Parser *parser) { return parse_expressao(parser); }
ASTNode *parse_assignment_expression(Parser *parser) { return parse_atribuicao(parser); }
ASTNode *parse_logical_or_expression(Parser *parser) { return parse_logico_ou(parser); }
ASTNode *parse_logical_and_expression(Parser *parser) { return parse_logico_e(parser); }
ASTNode *parse_equality_expression(Parser *parser) { return parse_igualdade(parser); }
ASTNode *parse_relational_expression(Parser *parser) { return parse_relacional(parser); }
ASTNode *parse_additive_expression(Parser *parser) { return parse_soma(parser); }
ASTNode *parse_multiplicative_expression(Parser *parser) { return parse_produto(parser); }
ASTNode *parse_unary_expression(Parser *parser) { return parse_unario(parser); }
ASTNode *parse_postfix_expression(Parser *parser) { return parse_sufixo(parser); }
ASTNode *parse_primary_expression(Parser *parser) { return parse_primario(parser); }
ASTNode *parse_function_declaration_with_info(Parser *parser, DataType return_type, char *name)
{
    return parse_definicao_funcao_com_info(parser, return_type, name);
}
ASTNode *parse_variable_declaration_with_info(Parser *parser, DataType type, char *name)
{
    return parse_declaracao_variavel_com_info(parser, type, name);
}
ASTNode *parse_preprocessor_directive(Parser *parser) { return parse_preprocessador(parser); }