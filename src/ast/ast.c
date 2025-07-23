#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode *ast_create_node(ASTNodeType type)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
        return NULL;

    node->type = type;
    node->data_type = TYPE_VOID;
    node->line = 0;
    node->column = 0;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;

    // Inicializar dados específicos com zeros
    memset(&node->data, 0, sizeof(node->data));

    // Inicialização específica baseada no tipo
    switch (type)
    {
    case AST_FUNCTION_DECLARATION:
        node->data.function_decl.name = NULL;
        node->data.function_decl.parameters = NULL;
        node->data.function_decl.body = NULL;
        node->data.function_decl.is_variadic = 0;
        break;

    case AST_VARIABLE_DECLARATION:
        node->data.var_decl.name = NULL;
        node->data.var_decl.initializer = NULL;
        node->data.var_decl.array_size = NULL;
        node->data.var_decl.pointer_level = 0;
        break;

    case AST_IDENTIFIER:
        node->data.identifier.name = NULL;
        break;

    case AST_FUNCTION_CALL:
        node->data.function_call.name = NULL;
        node->data.function_call.arguments = NULL;
        break;

    case AST_BINARY_EXPRESSION:
    case AST_ASSIGNMENT_EXPRESSION:
        node->data.binary_expr.left = NULL;
        node->data.binary_expr.right = NULL;
        break;

    case AST_UNARY_EXPRESSION:
        node->data.unary_expr.operand = NULL;
        break;

    case AST_IF_STATEMENT:
        node->data.if_stmt.condition = NULL;
        node->data.if_stmt.then_stmt = NULL;
        node->data.if_stmt.else_stmt = NULL;
        break;

    case AST_WHILE_STATEMENT:
        node->data.while_stmt.condition = NULL;
        node->data.while_stmt.body = NULL;
        break;

    case AST_RETURN_STATEMENT:
        node->data.return_stmt.expression = NULL;
        break;

    case AST_NUMBER_LITERAL:
    case AST_FLOAT_LITERAL:
    case AST_STRING_LITERAL:
    case AST_CHAR_LITERAL:
        node->data.literal.value = NULL;
        break;

    case AST_PREPROCESSOR_DIRECTIVE:
    case AST_INCLUDE_DIRECTIVE:
    case AST_DEFINE_DIRECTIVE:
        node->data.preprocessor.directive = NULL;
        node->data.preprocessor.content = NULL;
        break;

    default:
        // Para outros tipos, o memset já inicializou com zeros
        break;
    }

    return node;
}

void ast_add_child(ASTNode *parent, ASTNode *child)
{
    if (!parent || !child)
        return;

    if (parent->child_count >= parent->child_capacity)
    {
        parent->child_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        parent->children = realloc(parent->children,
                                   parent->child_capacity * sizeof(ASTNode *));
    }

    parent->children[parent->child_count++] = child;
}

void ast_destroy(ASTNode *node)
{
    if (!node)
        return;

    // Destruir filhos primeiro
    for (int i = 0; i < node->child_count; i++)
    {
        if (node->children[i])
        {
            ast_destroy(node->children[i]);
            node->children[i] = NULL; // Evitar double-free
        }
    }

    // Liberar array de filhos
    if (node->children)
    {
        free(node->children);
        node->children = NULL;
    }

    // Destruir dados específicos
    switch (node->type)
    {
    case AST_FUNCTION_DECLARATION:
        if (node->data.function_decl.name)
        {
            free(node->data.function_decl.name);
            node->data.function_decl.name = NULL;
        }
        break;

    case AST_VARIABLE_DECLARATION:
        if (node->data.var_decl.name)
        {
            free(node->data.var_decl.name);
            node->data.var_decl.name = NULL;
        }
        break;

    case AST_STRUCT_DECLARATION:
        if (node->data.struct_decl.name)
        {
            free(node->data.struct_decl.name);
            node->data.struct_decl.name = NULL;
        }
        break;

    case AST_ENUM_DECLARATION:
        if (node->data.enum_decl.name)
        {
            free(node->data.enum_decl.name);
            node->data.enum_decl.name = NULL;
        }
        break;

    case AST_FUNCTION_CALL:
        if (node->data.function_call.name)
        {
            free(node->data.function_call.name);
            node->data.function_call.name = NULL;
        }
        break;

    case AST_MEMBER_ACCESS:
        if (node->data.member_access.member)
        {
            free(node->data.member_access.member);
            node->data.member_access.member = NULL;
        }
        break;

    case AST_IDENTIFIER:
        if (node->data.identifier.name)
        {
            free(node->data.identifier.name);
            node->data.identifier.name = NULL;
        }
        break;

    case AST_NUMBER_LITERAL:
    case AST_FLOAT_LITERAL:
    case AST_STRING_LITERAL:
    case AST_CHAR_LITERAL:
        if (node->data.literal.value)
        {
            free(node->data.literal.value);
            node->data.literal.value = NULL;
        }
        break;

    case AST_PREPROCESSOR_DIRECTIVE:
    case AST_INCLUDE_DIRECTIVE:
    case AST_DEFINE_DIRECTIVE:
        if (node->data.preprocessor.directive)
        {
            free(node->data.preprocessor.directive);
            node->data.preprocessor.directive = NULL;
        }
        if (node->data.preprocessor.content)
        {
            free(node->data.preprocessor.content);
            node->data.preprocessor.content = NULL;
        }
        break;

    default:
        // Outros tipos não têm dados específicos para liberar
        break;
    }

    // Zerar a estrutura antes de liberar (debug)
    memset(node, 0, sizeof(ASTNode));

    // Liberar o nó
    free(node);
}

static void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

void ast_print(ASTNode *node, int indent)
{
    if (!node)
    {
        print_indent(indent);
        printf("(NULL)\n");
        return;
    }

    print_indent(indent);
    printf("%s", ast_node_type_to_string(node->type));

    switch (node->type)
    {
    case AST_FUNCTION_DECLARATION:
        if (node->data.function_decl.name)
            printf(" (%s)", node->data.function_decl.name);
        break;
    case AST_VARIABLE_DECLARATION:
        if (node->data.var_decl.name)
            printf(" (%s)", node->data.var_decl.name);
        break;
    // ... (outros casos com verificação de NULL)
    case AST_NUMBER_LITERAL:
    case AST_FLOAT_LITERAL:
    case AST_STRING_LITERAL:
    case AST_CHAR_LITERAL:
        if (node->data.literal.value)
            printf(" (%s)", node->data.literal.value);
        break;
    default:
        break;
    }

    printf("\n");

    // Imprimir filhos com verificação
    if (node->children && node->child_count > 0)
    {
        for (int i = 0; i < node->child_count; i++)
        {
            if (node->children[i])
            {
                ast_print(node->children[i], indent + 1);
            }
            else
            {
                print_indent(indent + 1);
                printf("(Child %d NULL)\n", i);
            }
        }
    }
}

const char *ast_node_type_to_string(ASTNodeType type)
{
    switch (type)
    {
    // Programa e declarações
    case AST_PROGRAM:
        return "PROGRAM";
    case AST_DECLARATION_LIST:
        return "DECLARATION_LIST";
    case AST_FUNCTION_DECLARATION:
        return "FUNCTION_DECLARATION";
    case AST_VARIABLE_DECLARATION:
        return "VARIABLE_DECLARATION";
    case AST_TYPE_DECLARATION:
        return "TYPE_DECLARATION";
    case AST_PARAMETER_LIST:
        return "PARAMETER_LIST";
    case AST_PARAMETER:
        return "PARAMETER";

    // Tipos e modificadores
    case AST_TYPE_SPECIFIER:
        return "TYPE_SPECIFIER";
    case AST_MODIFIER:
        return "MODIFIER";
    case AST_POINTER:
        return "POINTER";
    case AST_ARRAY:
        return "ARRAY";

    // Estruturas de dados
    case AST_STRUCT_DECLARATION:
        return "STRUCT_DECLARATION";
    case AST_UNION_DECLARATION:
        return "UNION_DECLARATION";
    case AST_ENUM_DECLARATION:
        return "ENUM_DECLARATION";
    case AST_FIELD_DECLARATION:
        return "FIELD_DECLARATION";
    case AST_ENUM_VALUE:
        return "ENUM_VALUE";

    // Comandos
    case AST_COMPOUND_STATEMENT:
        return "COMPOUND_STATEMENT";
    case AST_EXPRESSION_STATEMENT:
        return "EXPRESSION_STATEMENT";
    case AST_IF_STATEMENT:
        return "IF_STATEMENT";
    case AST_WHILE_STATEMENT:
        return "WHILE_STATEMENT";
    case AST_FOR_STATEMENT:
        return "FOR_STATEMENT";
    case AST_DO_WHILE_STATEMENT:
        return "DO_WHILE_STATEMENT";
    case AST_SWITCH_STATEMENT:
        return "SWITCH_STATEMENT";
    case AST_CASE_STATEMENT:
        return "CASE_STATEMENT";
    case AST_DEFAULT_STATEMENT:
        return "DEFAULT_STATEMENT";
    case AST_RETURN_STATEMENT:
        return "RETURN_STATEMENT";
    case AST_BREAK_STATEMENT:
        return "BREAK_STATEMENT";
    case AST_CONTINUE_STATEMENT:
        return "CONTINUE_STATEMENT";

    // Expressões
    case AST_ASSIGNMENT_EXPRESSION:
        return "ASSIGNMENT_EXPRESSION";
    case AST_TERNARY_EXPRESSION:
        return "TERNARY_EXPRESSION";
    case AST_BINARY_EXPRESSION:
        return "BINARY_EXPRESSION";
    case AST_UNARY_EXPRESSION:
        return "UNARY_EXPRESSION";
    case AST_POSTFIX_EXPRESSION:
        return "POSTFIX_EXPRESSION";
    case AST_FUNCTION_CALL:
        return "FUNCTION_CALL";
    case AST_ARRAY_ACCESS:
        return "ARRAY_ACCESS";
    case AST_MEMBER_ACCESS:
        return "MEMBER_ACCESS";
    case AST_POINTER_ACCESS:
        return "POINTER_ACCESS";

    // Expressões primárias
    case AST_IDENTIFIER:
        return "IDENTIFIER";
    case AST_NUMBER_LITERAL:
        return "NUMBER_LITERAL";
    case AST_FLOAT_LITERAL:
        return "FLOAT_LITERAL";
    case AST_STRING_LITERAL:
        return "STRING_LITERAL";
    case AST_CHAR_LITERAL:
        return "CHAR_LITERAL";
    case AST_BOOLEAN_LITERAL:
        return "BOOLEAN_LITERAL";

    // Preprocessador
    case AST_PREPROCESSOR_DIRECTIVE:
        return "PREPROCESSOR_DIRECTIVE";
    case AST_INCLUDE_DIRECTIVE:
        return "INCLUDE_DIRECTIVE";
    case AST_DEFINE_DIRECTIVE:
        return "DEFINE_DIRECTIVE";
    case AST_IFDEF_DIRECTIVE:
        return "IFDEF_DIRECTIVE";

    default:
        return "UNKNOWN";
    }
}

const char *data_type_to_string(DataType type)
{
    switch (type)
    {
    case TYPE_VOID:
        return "void";
    case TYPE_INT:
        return "int";
    case TYPE_FLOAT:
        return "float";
    case TYPE_CHAR:
        return "char";
    case TYPE_POINTER:
        return "pointer";
    case TYPE_ARRAY:
        return "array";
    case TYPE_STRUCT:
        return "struct";
    case TYPE_UNION:
        return "union";
    case TYPE_ENUM:
        return "enum";
    case TYPE_TYPEDEF:
        return "typedef";
    case TYPE_FUNCTION:
        return "function";
    default:
        return "unknown";
    }
}

const char *unary_operator_to_string(UnaryOperator op)
{
    switch (op)
    {
    case UNARY_PLUS:
        return "+";
    case UNARY_MINUS:
        return "-";
    case UNARY_NOT:
        return "!";
    case UNARY_BITWISE_NOT:
        return "~";
    case UNARY_ADDRESS:
        return "&";
    case UNARY_DEREFERENCE:
        return "*";
    case UNARY_PRE_INCREMENT:
        return "++";
    case UNARY_PRE_DECREMENT:
        return "--";
    case UNARY_POST_INCREMENT:
        return "++";
    case UNARY_POST_DECREMENT:
        return "--";
    default:
        return "unknown";
    }
}
