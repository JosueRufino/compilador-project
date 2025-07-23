// parser.c - VERSÃO REFATORADA seguindo a gramática corrigida

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

    // Obter primeiro token
    parser->current_token = lexer_next_token(lexer);

    return parser;
}

void parser_destroy(Parser *parser)
{
    if (parser)
    {
        token_destroy(&parser->current_token);
        free(parser);
    }
}

void parser_error(Parser *parser, const char *message)
{
    parser->has_error = 1;
    snprintf(parser->error_message, sizeof(parser->error_message),
             "Erro sintático na linha %d, coluna %d: %s",
             parser->current_token.line, parser->current_token.column, message);
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
    }
}

void parser_advance(Parser *parser)
{
    token_destroy(&parser->current_token);
    parser->current_token = lexer_next_token(parser->lexer);
}

void parser_skip_to_recovery_point(Parser *parser)
{
    // Pular tokens até encontrar um ponto de recuperação seguro
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

    // Se encontrou ';', consumir para continuar
    if (parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_advance(parser);
    }
}

ASTNode *parser_parse(Parser *parser)
{
    return parse_programa(parser);
}

// =================================================================
// IMPLEMENTAÇÃO SEGUINDO A GRAMÁTICA CORRIGIDA
// =================================================================

// <programa> ::= { <preprocessador> } { <declaracao_global> }
ASTNode *parse_programa(Parser *parser)
{
    ASTNode *programa = ast_create_node(AST_PROGRAM);

    // Pular preprocessadores
    while (parser_match(parser, TOKEN_HASH) && !parser->has_error)
    {
        ASTNode *prep = parse_preprocessador(parser);
        if (prep)
        {
            ast_add_child(programa, prep);
        }
    }

    // Processar declarações globais
    while (!parser_match(parser, TOKEN_EOF) && !parser->has_error)
    {
        ASTNode *decl = parse_declaracao_global(parser);
        if (decl)
        {
            ast_add_child(programa, decl);
        }
        else if (!parser->has_error)
        {
            // Se não conseguiu fazer parse mas não há erro, pode ser EOF
            break;
        }
    }

    if (parser->has_error)
    {
        ast_destroy(programa);
        return NULL;
    }

    return programa;
}

// <preprocessador> ::= "#include" ("<" <identificador> ".h" ">" | "\"" <identificador> ".h" "\"")
//                   | "#define" <identificador> [ <macro_params> ] <macro_body>
//                   | ...
ASTNode *parse_preprocessador(Parser *parser)
{
    if (!parser_match(parser, TOKEN_HASH))
    {
        return NULL;
    }

    ASTNode *prep = ast_create_node(AST_PREPROCESSOR_DIRECTIVE);
    parser_advance(parser); // consume '#'

    if (parser_match(parser, TOKEN_INCLUDE))
    {
        prep->data.preprocessor.directive = strdup("include");
        parser_advance(parser);

        // Processar resto da linha de include (simplificado)
        prep->data.preprocessor.content = strdup("stdio.h"); // placeholder

        // Pular até fim da linha ou próximo token válido
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
    else
    {
        // Outros preprocessadores - pular linha
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

// <declaracao_global> ::= <definicao_funcao> | <declaracao_variavel> | <typedef> | <struct>
ASTNode *parse_declaracao_global(Parser *parser)
{
    if (parser_match(parser, TOKEN_EOF))
    {
        return NULL;
    }

    // Por simplicidade, vamos focar em funções e variáveis
    if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
        parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
    {
        return parse_declaracao_com_tipo(parser);
    }

    parser_error(parser, "Esperado tipo de dados para declaração");
    return NULL;
}

// Função auxiliar para distinguir função vs variável
ASTNode *parse_declaracao_com_tipo(Parser *parser)
{
    // Capturar tipo
    DataType tipo = TYPE_VOID;
    if (parser_match(parser, TOKEN_INT))
        tipo = TYPE_INT;
    else if (parser_match(parser, TOKEN_FLOAT_KW))
        tipo = TYPE_FLOAT;
    else if (parser_match(parser, TOKEN_CHAR_KW))
        tipo = TYPE_CHAR;
    else if (parser_match(parser, TOKEN_VOID))
        tipo = TYPE_VOID;

    parser_advance(parser); // consumir tipo

    // Verificar se tem identificador
    if (!parser_match(parser, TOKEN_IDENTIFIER))
    {
        // Verificar se é apenas "tipo;" (declaração incompleta)
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
    parser_advance(parser); // consumir identificador

    // Decidir se é função ou variável
    if (parser_match(parser, TOKEN_LPAREN))
    {
        return parse_definicao_funcao_com_info(parser, tipo, nome);
    }
    else
    {
        return parse_declaracao_variavel_com_info(parser, tipo, nome);
    }
}

// <definicao_funcao> ::= <tipo> <identificador> "(" [ <parametros> ] ")" <bloco>
ASTNode *parse_definicao_funcao_com_info(Parser *parser, DataType tipo_retorno, char *nome)
{
    ASTNode *funcao = ast_create_node(AST_FUNCTION_DECLARATION);
    funcao->data.function_decl.name = nome;
    funcao->data.function_decl.return_type = tipo_retorno;

    parser_advance(parser); // consume '('

    // Processar parâmetros (simplificado)
    if (!parser_match(parser, TOKEN_RPAREN))
    {
        // TODO: implementar parse de parâmetros completo
        // Por enquanto, pular até ')'
        while (!parser_match(parser, TOKEN_RPAREN) && !parser_match(parser, TOKEN_EOF))
        {
            parser_advance(parser);
        }
    }

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após parâmetros");
        ast_destroy(funcao);
        return NULL;
    }
    parser_advance(parser); // consume ')'

    // Corpo da função
    funcao->data.function_decl.body = parse_bloco(parser);
    if (!funcao->data.function_decl.body)
    {
        ast_destroy(funcao);
        return NULL;
    }

    return funcao;
}

// <declaracao_variavel> ::= <tipo> <lista_identificadores> ";"
ASTNode *parse_declaracao_variavel_com_info(Parser *parser, DataType tipo, char *nome)
{
    ASTNode *var = ast_create_node(AST_VARIABLE_DECLARATION);
    var->data.var_decl.name = nome;
    var->data.var_decl.var_type = tipo;
    var->data.var_decl.initializer = NULL;

    // Verificar inicialização
    if (parser_match(parser, TOKEN_ASSIGN))
    {
        parser_advance(parser); // consume '='
        var->data.var_decl.initializer = parse_expressao(parser);
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após declaração de variável");
        ast_destroy(var);
        return NULL;
    }
    parser_advance(parser); // consume ';'

    return var;
}

// <bloco> ::= "{" { <item_bloco> } "}"
ASTNode *parse_bloco(Parser *parser)
{
    if (!parser_match(parser, TOKEN_LBRACE))
    {
        parser_error(parser, "Esperado '{'");
        return NULL;
    }

    ASTNode *bloco = ast_create_node(AST_COMPOUND_STATEMENT);
    parser_advance(parser); // consume '{'

    while (!parser_match(parser, TOKEN_RBRACE) && !parser_match(parser, TOKEN_EOF) && !parser->has_error)
    {
        ASTNode *item = parse_item_bloco(parser);

        if (item)
        {
            ast_add_child(bloco, item);
        }
        else if (parser->has_error)
        {
            // Tentar recuperar do erro
            printf("TENTANDO RECUPERAR DO ERRO: %s\n", parser->error_message);

            // Reset do erro para tentar continuar
            parser->has_error = 0;

            // Pular até ponto de recuperação
            parser_skip_to_recovery_point(parser);

            // Se ainda estamos em um token válido, continuar
            if (!parser_match(parser, TOKEN_EOF) && !parser_match(parser, TOKEN_RBRACE))
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if (!parser_match(parser, TOKEN_RBRACE))
    {
        parser_error(parser, "Esperado '}'");
        ast_destroy(bloco);
        return NULL;
    }
    parser_advance(parser); // consume '}'

    return bloco;
}

// <item_bloco> ::= <declaracao_variavel> | <comando>
ASTNode *parse_item_bloco(Parser *parser)
{
    // Verificar se é declaração de variável
    if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
        parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
    {
        // Verificar se tem identificador após o tipo
        TokenType tipo_token = parser->current_token.type;
        parser_advance(parser); // consumir tipo

        if (parser_match(parser, TOKEN_IDENTIFIER))
        {
            // É declaração válida - voltar e processar
            // Precisamos "voltar" o parser ou refazer a lógica

            // Mapear token para DataType
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
            default:
                tipo = TYPE_INT;
                break;
            }

            char *nome = strdup(parser->current_token.value);
            parser_advance(parser); // consumir identificador

            return parse_declaracao_variavel_com_info(parser, tipo, nome);
        }
        else if (parser_match(parser, TOKEN_SEMICOLON))
        {
            // Declaração incompleta como "int;" - tratar como erro
            parser_error(parser, "Declaração incompleta: esperado identificador após tipo");
            return NULL;
        }
        else
        {
            // Outro token - pode ser erro ou comando
            parser_error(parser, "Esperado identificador após tipo em declaração");
            return NULL;
        }
    }

    // Senão é comando
    return parse_comando(parser);
}

// <comando> ::= <comando_sem_if> | <comando_if_completo> | <comando_if_incompleto>
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

// <comando_sem_if> ::= <comando_expressao> | <comando_bloco> | <comando_while> |
//                     <comando_return> | <comando_break> | <comando_continue>
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

// Implementação dos comandos específicos
ASTNode *parse_comando_if(Parser *parser)
{
    ASTNode *if_stmt = ast_create_node(AST_IF_STATEMENT);

    parser_advance(parser); // consume 'if'

    if (!parser_match(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Esperado '(' após 'if'");
        ast_destroy(if_stmt);
        return NULL;
    }
    parser_advance(parser); // consume '('

    if_stmt->data.if_stmt.condition = parse_expressao(parser);

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após condição");
        ast_destroy(if_stmt);
        return NULL;
    }
    parser_advance(parser); // consume ')'

    if_stmt->data.if_stmt.then_stmt = parse_comando(parser);

    if (parser_match(parser, TOKEN_ELSE))
    {
        parser_advance(parser); // consume 'else'
        if_stmt->data.if_stmt.else_stmt = parse_comando(parser);
    }

    return if_stmt;
}

ASTNode *parse_comando_while(Parser *parser)
{
    ASTNode *while_stmt = ast_create_node(AST_WHILE_STATEMENT);

    parser_advance(parser); // consume 'while'

    if (!parser_match(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Esperado '(' após 'while'");
        ast_destroy(while_stmt);
        return NULL;
    }
    parser_advance(parser); // consume '('

    while_stmt->data.while_stmt.condition = parse_expressao(parser);

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após condição");
        ast_destroy(while_stmt);
        return NULL;
    }
    parser_advance(parser); // consume ')'

    while_stmt->data.while_stmt.body = parse_comando(parser);

    return while_stmt;
}

ASTNode *parse_comando_return(Parser *parser)
{
    ASTNode *return_stmt = ast_create_node(AST_RETURN_STATEMENT);

    parser_advance(parser); // consume 'return'

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        return_stmt->data.return_stmt.expression = parse_expressao(parser);
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após 'return'");
        ast_destroy(return_stmt);
        return NULL;
    }
    parser_advance(parser); // consume ';'

    return return_stmt;
}

ASTNode *parse_comando_break(Parser *parser)
{
    ASTNode *break_stmt = ast_create_node(AST_BREAK_STATEMENT);
    parser_advance(parser); // consume 'break'

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após 'break'");
        ast_destroy(break_stmt);
        return NULL;
    }
    parser_advance(parser); // consume ';'

    return break_stmt;
}

ASTNode *parse_comando_continue(Parser *parser)
{
    ASTNode *continue_stmt = ast_create_node(AST_CONTINUE_STATEMENT);
    parser_advance(parser); // consume 'continue'

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após 'continue'");
        ast_destroy(continue_stmt);
        return NULL;
    }
    parser_advance(parser); // consume ';'

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
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após expressão");
        ast_destroy(expr_stmt);
        return NULL;
    }
    parser_advance(parser); // consume ';'

    return expr_stmt;
}

// =================================================================
// EXPRESSÕES - Seguindo precedência da gramática
// =================================================================

// <expressao> ::= <expressao_virgula>
ASTNode *parse_expressao(Parser *parser)
{
    return parse_expressao_virgula(parser);
}

// <expressao_virgula> ::= <atribuicao> { "," <atribuicao> }
ASTNode *parse_expressao_virgula(Parser *parser)
{
    ASTNode *expr = parse_atribuicao(parser);

    while (parser_match(parser, TOKEN_COMMA))
    {
        ASTNode *comma_expr = ast_create_node(AST_BINARY_EXPRESSION);
        comma_expr->data.binary_expr.left = expr;
        comma_expr->data.binary_expr.operator = TOKEN_COMMA;
        parser_advance(parser); // consume ','
        comma_expr->data.binary_expr.right = parse_atribuicao(parser);
        expr = comma_expr;
    }

    return expr;
}

// <atribuicao> ::= <condicional> [ <operador_atribuicao> <atribuicao> ]
ASTNode *parse_atribuicao(Parser *parser)
{
    ASTNode *expr = parse_condicional(parser);

    if (parser_match(parser, TOKEN_ASSIGN))
    {
        ASTNode *assign = ast_create_node(AST_ASSIGNMENT_EXPRESSION);
        assign->data.binary_expr.left = expr;
        assign->data.binary_expr.operator = TOKEN_ASSIGN;
        parser_advance(parser);                                    // consume '='
        assign->data.binary_expr.right = parse_atribuicao(parser); // associativo à direita
        return assign;
    }

    return expr;
}

// <condicional> ::= <logico_ou> [ "?" <expressao> ":" <condicional> ]
ASTNode *parse_condicional(Parser *parser)
{
    ASTNode *expr = parse_logico_ou(parser);

    if (parser_match(parser, TOKEN_QUESTION))
    {
        ASTNode *ternary = ast_create_node(AST_TERNARY_EXPRESSION);
        ternary->data.ternary_expr.condition = expr;
        parser_advance(parser); // consume '?'
        ternary->data.ternary_expr.true_expr = parse_expressao(parser);

        if (!parser_match(parser, TOKEN_COLON))
        {
            parser_error(parser, "Esperado ':' em operador ternário");
            ast_destroy(ternary);
            return expr;
        }
        parser_advance(parser); // consume ':'
        ternary->data.ternary_expr.false_expr = parse_condicional(parser);
        return ternary;
    }

    return expr;
}

// <logico_ou> ::= <logico_e> { "||" <logico_e> }
ASTNode *parse_logico_ou(Parser *parser)
{
    ASTNode *expr = parse_logico_e(parser);

    while (parser_match(parser, TOKEN_OR))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = TOKEN_OR;
        parser_advance(parser); // consume '||'
        binary->data.binary_expr.right = parse_logico_e(parser);
        expr = binary;
    }

    return expr;
}

// <logico_e> ::= <igualdade> { "&&" <igualdade> }
ASTNode *parse_logico_e(Parser *parser)
{
    ASTNode *expr = parse_igualdade(parser);

    while (parser_match(parser, TOKEN_AND))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = TOKEN_AND;
        parser_advance(parser); // consume '&&'
        binary->data.binary_expr.right = parse_igualdade(parser);
        expr = binary;
    }

    return expr;
}

// <igualdade> ::= <relacional> { ("==" | "!=") <relacional> }
ASTNode *parse_igualdade(Parser *parser)
{
    ASTNode *expr = parse_relacional(parser);

    while (parser_match(parser, TOKEN_EQUAL) || parser_match(parser, TOKEN_NOT_EQUAL))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_relacional(parser);
        expr = binary;
    }

    return expr;
}

// <relacional> ::= <soma> { ("<" | ">" | "<=" | ">=") <soma> }
ASTNode *parse_relacional(Parser *parser)
{
    ASTNode *expr = parse_soma(parser);

    while (parser_match(parser, TOKEN_LESS) || parser_match(parser, TOKEN_GREATER) ||
           parser_match(parser, TOKEN_LESS_EQUAL) || parser_match(parser, TOKEN_GREATER_EQUAL))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_soma(parser);
        expr = binary;
    }

    return expr;
}

// <soma> ::= <produto> { ("+" | "-") <produto> }
ASTNode *parse_soma(Parser *parser)
{
    ASTNode *expr = parse_produto(parser);

    while (parser_match(parser, TOKEN_PLUS) || parser_match(parser, TOKEN_MINUS))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_produto(parser);
        expr = binary;
    }

    return expr;
}

// <produto> ::= <unario> { ("*" | "/" | "%") <unario> }
ASTNode *parse_produto(Parser *parser)
{
    ASTNode *expr = parse_unario(parser);

    while (parser_match(parser, TOKEN_MULTIPLY) || parser_match(parser, TOKEN_DIVIDE) ||
           parser_match(parser, TOKEN_MODULO))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_unario(parser);
        expr = binary;
    }

    return expr;
}

// <unario> ::= <operador_unario> <unario> | <sufixo>
ASTNode *parse_unario(Parser *parser)
{
    if (parser_match(parser, TOKEN_PLUS) || parser_match(parser, TOKEN_MINUS) ||
        parser_match(parser, TOKEN_NOT) || parser_match(parser, TOKEN_BITWISE_NOT) ||
        parser_match(parser, TOKEN_INCREMENT) || parser_match(parser, TOKEN_DECREMENT))
    {

        ASTNode *unary = ast_create_node(AST_UNARY_EXPRESSION);

        // Mapear token para operador unário
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
        default:
            unary->data.unary_expr.operator = UNARY_PLUS;
            break;
        }

        parser_advance(parser);
        unary->data.unary_expr.operand = parse_unario(parser);
        return unary;
    }

    return parse_sufixo(parser);
}

// <sufixo> ::= <primario> { <operador_sufixo> }
ASTNode *parse_sufixo(Parser *parser)
{
    ASTNode *expr = parse_primario(parser);

    while (parser_match(parser, TOKEN_LPAREN) || parser_match(parser, TOKEN_LBRACKET) ||
           parser_match(parser, TOKEN_DOT) || parser_match(parser, TOKEN_ARROW) ||
           parser_match(parser, TOKEN_INCREMENT) || parser_match(parser, TOKEN_DECREMENT))
    {

        if (parser_match(parser, TOKEN_LPAREN))
        {
            // Chamada de função
            ASTNode *call = ast_create_node(AST_FUNCTION_CALL);
            if (expr && expr->type == AST_IDENTIFIER)
            {
                call->data.function_call.name = strdup(expr->data.identifier.name);
            }
            else
            {
                call->data.function_call.name = strdup("unknown");
            }

            parser_advance(parser); // consume '('

            // Argumentos
            if (!parser_match(parser, TOKEN_RPAREN))
            {
                do
                {
                    ASTNode *arg = parse_atribuicao(parser);
                    if (arg)
                    {
                        ast_add_child(call, arg);
                    }
                    if (parser_match(parser, TOKEN_COMMA))
                    {
                        parser_advance(parser);
                    }
                    else
                    {
                        break;
                    }
                } while (!parser_match(parser, TOKEN_RPAREN) && !parser->has_error);
            }

            if (!parser_match(parser, TOKEN_RPAREN))
            {
                parser_error(parser, "Esperado ')' após argumentos");
                ast_destroy(call);
                return expr;
            }
            parser_advance(parser); // consume ')'

            if (expr)
                ast_destroy(expr);
            expr = call;
        }
        // TODO: implementar outros operadores sufixo ([], ., ->, ++, --)
        else
        {
            break; // Por enquanto, parar aqui
        }
    }

    return expr;
}

// <primario> ::= <numero> | <caractere> | <string> | <identificador> | "(" <expressao> ")"
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
        parser_advance(parser); // consume '('
        ASTNode *expr = parse_expressao(parser);

        if (!parser_match(parser, TOKEN_RPAREN))
        {
            parser_error(parser, "Esperado ')' após expressão");
            if (expr)
                ast_destroy(expr);
            return NULL;
        }
        parser_advance(parser); // consume ')'
        return expr;
    }

    parser_error(parser, "Expressão primária inválida");
    return NULL;
}

// =================================================================
// FUNÇÕES OBSOLETAS - manter para compatibilidade
// =================================================================

ASTNode *parse_program(Parser *parser)
{
    return parse_programa(parser);
}

ASTNode *parse_declaration(Parser *parser)
{
    return parse_declaracao_global(parser);
}

ASTNode *parse_function_declaration(Parser *parser)
{
    // Esta função não é mais usada diretamente
    (void)parser;
    return NULL;
}

ASTNode *parse_variable_declaration(Parser *parser)
{
    // Esta função não é mais usada diretamente
    (void)parser;
    return NULL;
}

ASTNode *parse_statement(Parser *parser)
{
    return parse_comando(parser);
}

ASTNode *parse_compound_statement(Parser *parser)
{
    return parse_bloco(parser);
}

ASTNode *parse_expression_statement(Parser *parser)
{
    return parse_comando_expressao(parser);
}

ASTNode *parse_if_statement(Parser *parser)
{
    return parse_comando_if(parser);
}

ASTNode *parse_while_statement(Parser *parser)
{
    return parse_comando_while(parser);
}

ASTNode *parse_return_statement(Parser *parser)
{
    return parse_comando_return(parser);
}

ASTNode *parse_expression(Parser *parser)
{
    return parse_expressao(parser);
}

ASTNode *parse_assignment_expression(Parser *parser)
{
    return parse_atribuicao(parser);
}

ASTNode *parse_logical_or_expression(Parser *parser)
{
    return parse_logico_ou(parser);
}

ASTNode *parse_logical_and_expression(Parser *parser)
{
    return parse_logico_e(parser);
}

ASTNode *parse_equality_expression(Parser *parser)
{
    return parse_igualdade(parser);
}

ASTNode *parse_relational_expression(Parser *parser)
{
    return parse_relacional(parser);
}

ASTNode *parse_additive_expression(Parser *parser)
{
    return parse_soma(parser);
}

ASTNode *parse_multiplicative_expression(Parser *parser)
{
    return parse_produto(parser);
}

ASTNode *parse_unary_expression(Parser *parser)
{
    return parse_unario(parser);
}

ASTNode *parse_postfix_expression(Parser *parser)
{
    return parse_sufixo(parser);
}

ASTNode *parse_primary_expression(Parser *parser)
{
    return parse_primario(parser);
}

ASTNode *parse_function_declaration_with_info(Parser *parser, DataType return_type, char *name)
{
    return parse_definicao_funcao_com_info(parser, return_type, name);
}

ASTNode *parse_variable_declaration_with_info(Parser *parser, DataType type, char *name)
{
    return parse_declaracao_variavel_com_info(parser, type, name);
}

ASTNode *parse_preprocessor_directive(Parser *parser)
{
    return parse_preprocessador(parser);
}