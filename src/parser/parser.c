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

ASTNode *parser_parse(Parser *parser)
{
    return parse_program(parser);
}

ASTNode *parse_program(Parser *parser)
{
    ASTNode *program = ast_create_node(AST_PROGRAM);

    while (!parser_match(parser, TOKEN_EOF) && !parser->has_error)
    {
        ASTNode *decl = parse_declaration(parser);

        // Se parse_declaration retornou NULL mas não há erro,
        // pode ser EOF ou token válido mas não processável
        if (!decl && !parser->has_error)
        {
            // Se não é EOF, avançar um token para evitar loop infinito
            if (!parser_match(parser, TOKEN_EOF))
            {
                printf("DEBUG: Avançando token não processado: %s\n",
                       token_type_to_string(parser->current_token.type));
                parser_advance(parser);
            }
            continue;
        }

        // Se houve erro, parar
        if (parser->has_error)
        {
            if (program)
            {
                ast_destroy(program);
            }
            return NULL;
        }

        // Se temos uma declaração válida, adicionar ao programa
        if (decl)
        {
            ast_add_child(program, decl);
        }
    }

    return program;
}

ASTNode *parse_declaration(Parser *parser)
{
    // Se chegamos ao EOF, retornar NULL sem erro
    if (parser_match(parser, TOKEN_EOF))
    {
        return NULL;
    }

    // Verificar preprocessador primeiro
    if (parser_match(parser, TOKEN_HASH))
    {
        return parse_preprocessor_directive(parser);
    }

    // Verificar se é declaração de tipo
    if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
        parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
    {

        DataType type = TYPE_VOID;
        if (parser_match(parser, TOKEN_INT))
            type = TYPE_INT;
        else if (parser_match(parser, TOKEN_FLOAT_KW))
            type = TYPE_FLOAT;
        else if (parser_match(parser, TOKEN_CHAR_KW))
            type = TYPE_CHAR;
        else if (parser_match(parser, TOKEN_VOID))
            type = TYPE_VOID;

        parser_advance(parser); // consumir tipo

        if (!parser_match(parser, TOKEN_IDENTIFIER))
        {
            parser_error(parser, "Esperado identificador após tipo");
            return NULL;
        }

        char *name = strdup(parser->current_token.value);
        parser_advance(parser); // consumir identificador

        if (parser_match(parser, TOKEN_LPAREN))
        {
            // É uma função
            return parse_function_declaration_with_info(parser, type, name);
        }
        else
        {
            // É uma variável
            return parse_variable_declaration_with_info(parser, type, name);
        }
    }

    // Se chegou aqui e não é EOF, há um token inesperado
    if (!parser_match(parser, TOKEN_EOF))
    {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
                 "Token inesperado: %s. Esperado: tipo de dados (int, float, char, void) ou fim de arquivo",
                 token_type_to_string(parser->current_token.type));
        parser_error(parser, error_msg);
    }

    return NULL;
}

ASTNode *parse_function_declaration_with_info(Parser *parser, DataType return_type, char *name)
{
    ASTNode *func = ast_create_node(AST_FUNCTION_DECLARATION);
    func->data.function_decl.name = name;
    func->data.function_decl.return_type = return_type;
    func->data.function_decl.modifiers = MOD_NONE;
    func->data.function_decl.is_variadic = 0;

    // Já estamos no '('
    parser_advance(parser); // consumir '('

    // Parâmetros (simplificado) - por enquanto apenas pular até ')'
    while (!parser_match(parser, TOKEN_RPAREN) && !parser_match(parser, TOKEN_EOF) && !parser->has_error)
    {
        if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
            parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
        {
            parser_advance(parser); // tipo do parâmetro
            if (parser_match(parser, TOKEN_IDENTIFIER))
            {
                parser_advance(parser); // nome do parâmetro
            }
            if (parser_match(parser, TOKEN_COMMA))
            {
                parser_advance(parser);
            }
        }
        else
        {
            // Se não é um tipo conhecido, avançar para evitar loop infinito
            parser_advance(parser);
        }
    }

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após parâmetros");
        free(name);
        ast_destroy(func);
        return NULL;
    }
    parser_advance(parser); // consumir ')'

    // Corpo da função
    if (parser_match(parser, TOKEN_LBRACE))
    {
        func->data.function_decl.body = parse_compound_statement(parser);
    }
    else
    {
        parser_error(parser, "Esperado '{' para início do corpo da função");
        free(name);
        ast_destroy(func);
        return NULL;
    }

    return func;
}

ASTNode *parse_variable_declaration_with_info(Parser *parser, DataType type, char *name)
{
    ASTNode *var = ast_create_node(AST_VARIABLE_DECLARATION);
    var->data.var_decl.name = name;
    var->data.var_decl.var_type = type;
    var->data.var_decl.modifiers = MOD_NONE;
    var->data.var_decl.pointer_level = 0;
    var->data.var_decl.initializer = NULL;

    if (parser_match(parser, TOKEN_ASSIGN))
    {
        parser_advance(parser);
        var->data.var_decl.initializer = parse_expression(parser);
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Esperado ';' após declaração de variável");
        free(name);
        ast_destroy(var);
        return NULL;
    }
    parser_advance(parser); // consumir ';'

    return var;
}

ASTNode *parse_function_declaration(Parser *parser)
{
    // Esta função não é mais usada diretamente
    (void)parser; // Suprimir warning
    return NULL;
}

ASTNode *parse_variable_declaration(Parser *parser)
{
    // Esta função não é mais usada diretamente
    (void)parser; // Suprimir warning
    return NULL;
}

ASTNode *parse_statement(Parser *parser)
{
    if (parser_match(parser, TOKEN_LBRACE))
    {
        return parse_compound_statement(parser);
    }
    else if (parser_match(parser, TOKEN_IF))
    {
        return parse_if_statement(parser);
    }
    else if (parser_match(parser, TOKEN_WHILE))
    {
        return parse_while_statement(parser);
    }
    else if (parser_match(parser, TOKEN_RETURN))
    {
        return parse_return_statement(parser);
    }
    else
    {
        return parse_expression_statement(parser);
    }
}

ASTNode *parse_compound_statement(Parser *parser)
{
    ASTNode *compound = ast_create_node(AST_COMPOUND_STATEMENT);

    if (!parser_match(parser, TOKEN_LBRACE))
    {
        parser_error(parser, "Esperado '{'");
        ast_destroy(compound);
        return NULL;
    }
    parser_advance(parser); // consumir '{'

    while (!parser_match(parser, TOKEN_RBRACE) && !parser_match(parser, TOKEN_EOF) && !parser->has_error)
    {
        ASTNode *stmt = NULL;

        // Verificar se é declaração local ou comando
        if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
            parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
        {
            stmt = parse_declaration(parser);
        }
        else
        {
            stmt = parse_statement(parser);
        }

        if (stmt)
        {
            ast_add_child(compound, stmt);
        }

        if (parser->has_error)
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_RBRACE))
    {
        parser_error(parser, "Esperado '}'");
        ast_destroy(compound);
        return NULL;
    }
    parser_advance(parser); // consumir '}'

    return compound;
}

ASTNode *parse_expression_statement(Parser *parser)
{
    ASTNode *expr_stmt = ast_create_node(AST_EXPRESSION_STATEMENT);

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        ASTNode *expr = parse_expression(parser);
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
    parser_advance(parser); // consumir ';'

    return expr_stmt;
}

ASTNode *parse_if_statement(Parser *parser)
{
    ASTNode *if_stmt = ast_create_node(AST_IF_STATEMENT);

    parser_advance(parser); // 'if'

    if (!parser_match(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Esperado '(' após 'if'");
        ast_destroy(if_stmt);
        return NULL;
    }
    parser_advance(parser);

    if_stmt->data.if_stmt.condition = parse_expression(parser);

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após condição");
        ast_destroy(if_stmt);
        return NULL;
    }
    parser_advance(parser);

    if_stmt->data.if_stmt.then_stmt = parse_statement(parser);

    if (parser_match(parser, TOKEN_ELSE))
    {
        parser_advance(parser);
        if_stmt->data.if_stmt.else_stmt = parse_statement(parser);
    }

    return if_stmt;
}

ASTNode *parse_while_statement(Parser *parser)
{
    ASTNode *while_stmt = ast_create_node(AST_WHILE_STATEMENT);

    parser_advance(parser); // 'while'

    if (!parser_match(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Esperado '(' após 'while'");
        ast_destroy(while_stmt);
        return NULL;
    }
    parser_advance(parser);

    while_stmt->data.while_stmt.condition = parse_expression(parser);

    if (!parser_match(parser, TOKEN_RPAREN))
    {
        parser_error(parser, "Esperado ')' após condição");
        ast_destroy(while_stmt);
        return NULL;
    }
    parser_advance(parser);

    while_stmt->data.while_stmt.body = parse_statement(parser);

    return while_stmt;
}

ASTNode *parse_return_statement(Parser *parser)
{
    ASTNode *return_stmt = ast_create_node(AST_RETURN_STATEMENT);

    parser_advance(parser); // 'return'

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        return_stmt->data.return_stmt.expression = parse_expression(parser);
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

ASTNode *parse_expression(Parser *parser)
{
    return parse_assignment_expression(parser);
}

ASTNode *parse_assignment_expression(Parser *parser)
{
    ASTNode *expr = parse_logical_or_expression(parser);

    if (parser_match(parser, TOKEN_ASSIGN))
    {
        ASTNode *assign = ast_create_node(AST_ASSIGNMENT_EXPRESSION);
        parser_advance(parser);

        assign->data.binary_expr.left = expr;
        assign->data.binary_expr.operator = TOKEN_ASSIGN;
        assign->data.binary_expr.right = parse_assignment_expression(parser);

        return assign;
    }

    return expr;
}

ASTNode *parse_logical_or_expression(Parser *parser)
{
    ASTNode *expr = parse_logical_and_expression(parser);

    while (parser_match(parser, TOKEN_OR))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_logical_and_expression(parser);
        expr = binary;
    }

    return expr;
}

ASTNode *parse_logical_and_expression(Parser *parser)
{
    ASTNode *expr = parse_equality_expression(parser);

    while (parser_match(parser, TOKEN_AND))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_equality_expression(parser);
        expr = binary;
    }

    return expr;
}

ASTNode *parse_equality_expression(Parser *parser)
{
    ASTNode *expr = parse_relational_expression(parser);

    while (parser_match(parser, TOKEN_EQUAL) || parser_match(parser, TOKEN_NOT_EQUAL))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_relational_expression(parser);
        expr = binary;
    }

    return expr;
}

ASTNode *parse_relational_expression(Parser *parser)
{
    ASTNode *expr = parse_additive_expression(parser);

    while (parser_match(parser, TOKEN_LESS) || parser_match(parser, TOKEN_GREATER) ||
           parser_match(parser, TOKEN_LESS_EQUAL) || parser_match(parser, TOKEN_GREATER_EQUAL))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_additive_expression(parser);
        expr = binary;
    }

    return expr;
}

ASTNode *parse_additive_expression(Parser *parser)
{
    ASTNode *expr = parse_multiplicative_expression(parser);

    while (parser_match(parser, TOKEN_PLUS) || parser_match(parser, TOKEN_MINUS))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_multiplicative_expression(parser);
        expr = binary;
    }

    return expr;
}

ASTNode *parse_multiplicative_expression(Parser *parser)
{
    ASTNode *expr = parse_unary_expression(parser);

    while (parser_match(parser, TOKEN_MULTIPLY) || parser_match(parser, TOKEN_DIVIDE) ||
           parser_match(parser, TOKEN_MODULO))
    {
        ASTNode *binary = ast_create_node(AST_BINARY_EXPRESSION);
        binary->data.binary_expr.left = expr;
        binary->data.binary_expr.operator = parser->current_token.type;
        parser_advance(parser);
        binary->data.binary_expr.right = parse_unary_expression(parser);
        expr = binary;
    }

    return expr;
}

ASTNode *parse_unary_expression(Parser *parser)
{
    if (parser_match(parser, TOKEN_MINUS) || parser_match(parser, TOKEN_NOT))
    {
        ASTNode *unary = ast_create_node(AST_UNARY_EXPRESSION);

        if (parser_match(parser, TOKEN_MINUS))
        {
            unary->data.unary_expr.operator = UNARY_MINUS;
        }
        else if (parser_match(parser, TOKEN_NOT))
        {
            unary->data.unary_expr.operator = UNARY_NOT;
        }

        parser_advance(parser);
        unary->data.unary_expr.operand = parse_unary_expression(parser);
        return unary;
    }

    return parse_postfix_expression(parser);
}

ASTNode *parse_postfix_expression(Parser *parser)
{
    ASTNode *expr = parse_primary_expression(parser);

    while (parser_match(parser, TOKEN_LPAREN))
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

        parser_advance(parser); // '('

        // Argumentos (simplificado)
        while (!parser_match(parser, TOKEN_RPAREN) && !parser_match(parser, TOKEN_EOF) && !parser->has_error)
        {
            ASTNode *arg = parse_expression(parser);
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
        }

        if (!parser_match(parser, TOKEN_RPAREN))
        {
            parser_error(parser, "Esperado ')' após argumentos");
            ast_destroy(call);
            return expr;
        }
        parser_advance(parser);

        if (expr)
        {
            ast_destroy(expr);
        }
        expr = call;
    }

    return expr;
}

ASTNode *parse_primary_expression(Parser *parser)
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
        ASTNode *expr = parse_expression(parser);
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

ASTNode *parse_preprocessor_directive(Parser *parser)
{
    ASTNode *directive = ast_create_node(AST_PREPROCESSOR_DIRECTIVE);

    parser_advance(parser); // '#'

    if (parser_match(parser, TOKEN_INCLUDE))
    {
        directive->data.preprocessor.directive = strdup("include");
        parser_advance(parser);

        // Capturar o resto da linha como conteúdo
        if (parser_match(parser, TOKEN_LESS))
        {
            parser_advance(parser);
            if (parser_match(parser, TOKEN_IDENTIFIER))
            {
                directive->data.preprocessor.content = strdup(parser->current_token.value);
                parser_advance(parser);
            }
            if (parser_match(parser, TOKEN_DOT))
            {
                parser_advance(parser);
                if (parser_match(parser, TOKEN_IDENTIFIER))
                {
                    // Concatenar extensão
                    char *full_name = malloc(strlen(directive->data.preprocessor.content) + 10);
                    sprintf(full_name, "%s.%s", directive->data.preprocessor.content, parser->current_token.value);
                    free(directive->data.preprocessor.content);
                    directive->data.preprocessor.content = full_name;
                    parser_advance(parser);
                }
            }
            if (parser_match(parser, TOKEN_GREATER))
            {
                parser_advance(parser);
            }
        }
        else if (parser_match(parser, TOKEN_STRING))
        {
            directive->data.preprocessor.content = strdup(parser->current_token.value);
            parser_advance(parser);
        }
    }
    else
    {
        // Diretiva desconhecida - pular até o fim da linha
        directive->data.preprocessor.directive = strdup("unknown");
        directive->data.preprocessor.content = strdup("");

        // Pular tokens até encontrar nova linha ou uma nova declaração
        while (!parser_match(parser, TOKEN_EOF) && !parser->has_error)
        {
            // Verificar se é início de nova declaração
            if (parser_match(parser, TOKEN_INT) || parser_match(parser, TOKEN_FLOAT_KW) ||
                parser_match(parser, TOKEN_CHAR_KW) || parser_match(parser, TOKEN_VOID))
            {
                break;
            }
            parser_advance(parser);
        }
    }

    return directive;
}