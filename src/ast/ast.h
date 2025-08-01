#ifndef AST_H
#define AST_H

#include "lexer.h"

// Tipos de nós da AST baseados na gramática fornecida
typedef enum {
    // Programa e declarações
    AST_PROGRAM,
    AST_DECLARATION_LIST,
    AST_FUNCTION_DECLARATION,
    AST_VARIABLE_DECLARATION,
    AST_TYPE_DECLARATION,
    AST_PARAMETER_LIST,
    AST_PARAMETER,
    
    // Tipos e modificadores
    AST_TYPE_SPECIFIER,
    AST_MODIFIER,
    AST_POINTER,
    AST_ARRAY,
    
    // Estruturas de dados
    AST_STRUCT_DECLARATION,
    AST_UNION_DECLARATION,
    AST_ENUM_DECLARATION,
    AST_FIELD_DECLARATION,
    AST_ENUM_VALUE,
    
    // Comandos
    AST_COMPOUND_STATEMENT,
    AST_EXPRESSION_STATEMENT,
    AST_IF_STATEMENT,
    AST_WHILE_STATEMENT,
    AST_FOR_STATEMENT,
    AST_DO_WHILE_STATEMENT,
    AST_SWITCH_STATEMENT,
    AST_CASE_STATEMENT,
    AST_DEFAULT_STATEMENT,
    AST_RETURN_STATEMENT,
    AST_BREAK_STATEMENT,
    AST_CONTINUE_STATEMENT,
    
    // Expressões
    AST_ASSIGNMENT_EXPRESSION,
    AST_TERNARY_EXPRESSION,
    AST_BINARY_EXPRESSION,
    AST_UNARY_EXPRESSION,
    AST_POSTFIX_EXPRESSION,
    AST_FUNCTION_CALL,
    AST_ARRAY_ACCESS,
    AST_MEMBER_ACCESS,
    AST_POINTER_ACCESS,
    
    // Expressões primárias
    AST_IDENTIFIER,
    AST_NUMBER_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_CHAR_LITERAL,
    AST_BOOLEAN_LITERAL,
    
    // Preprocessador
    AST_PREPROCESSOR_DIRECTIVE,
    AST_INCLUDE_DIRECTIVE,
    AST_DEFINE_DIRECTIVE,
    AST_IFDEF_DIRECTIVE
} ASTNodeType;

// Tipos de dados expandidos
typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM,
    TYPE_TYPEDEF,
    TYPE_FUNCTION
} DataType;

// Modificadores de tipo
typedef enum {
    MOD_NONE = 0,
    MOD_STATIC = 1,
    MOD_EXTERN = 2,
    MOD_CONST = 4,
    MOD_VOLATILE = 8
} TypeModifier;

// Operadores unários
typedef enum {
    UNARY_PLUS,
    UNARY_MINUS,
    UNARY_NOT,
    UNARY_BITWISE_NOT,
    UNARY_ADDRESS,
    UNARY_DEREFERENCE,
    UNARY_PRE_INCREMENT,
    UNARY_PRE_DECREMENT,
    UNARY_POST_INCREMENT,
    UNARY_POST_DECREMENT
} UnaryOperator;

// Definição de estruturas para os dados da união
typedef struct {
    char* name;
    DataType param_type;
} ASTParameter;

typedef struct {
    char* directive;
    char* content;
} ASTPreprocessor;

typedef struct {
    char* name;
    DataType return_type;
    TypeModifier modifiers;
    struct ASTNode* parameters;
    struct ASTNode* body;
    int is_variadic;
} ASTFunctionDecl;

typedef struct {
    char* name;
    DataType var_type;
    TypeModifier modifiers;
    struct ASTNode* initializer;
    struct ASTNode* array_size;
    int pointer_level;
} ASTVariableDecl;

typedef struct {
    char* name;
    DataType base_type;
    struct ASTNode* fields;
} ASTStructDecl;

typedef struct {
    char* name;
    struct ASTNode* values;
} ASTEnumDecl;

typedef struct {
    TokenType operator;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTBinaryExpr;

typedef struct {
    UnaryOperator operator;
    struct ASTNode* operand;
} ASTUnaryExpr;

typedef struct {
    struct ASTNode* condition;
    struct ASTNode* true_expr;
    struct ASTNode* false_expr;
} ASTTernaryExpr;

typedef struct {
    struct ASTNode* condition;
    struct ASTNode* then_stmt;
    struct ASTNode* else_stmt;
} ASTIfStmt;

typedef struct {
    struct ASTNode* condition;
    struct ASTNode* body;
} ASTWhileStmt;

typedef struct {
    struct ASTNode* init;
    struct ASTNode* condition;
    struct ASTNode* update;
    struct ASTNode* body;
} ASTForStmt;

typedef struct {
    struct ASTNode* expression;
    struct ASTNode* cases;
} ASTSwitchStmt;

typedef struct {
    struct ASTNode* value;
    struct ASTNode* statements;
} ASTCaseStmt;

typedef struct {
    struct ASTNode* expression;
} ASTReturnStmt;

typedef struct {
    char* name;
    struct ASTNode* arguments;
} ASTFunctionCall;

typedef struct {
    struct ASTNode* array;
    struct ASTNode* index;
} ASTArrayAccess;

typedef struct {
    struct ASTNode* object;
    char* member;
    int is_pointer_access; // -> vs .
} ASTMemberAccess;

typedef struct {
    char* name;
} ASTIdentifier;

typedef struct {
    char* value;
} ASTLiteral;

// Estrutura do nó AST
typedef struct ASTNode {
    ASTNodeType type;
    DataType data_type;
    int line;
    int column;
    struct ASTNode** children;
    int child_count;
    int child_capacity;
    
    // Dados específicos do nó
    union {
        ASTPreprocessor preprocessor;
        ASTFunctionDecl function_decl;
        ASTVariableDecl var_decl;
        ASTStructDecl struct_decl;
        ASTEnumDecl enum_decl;
        ASTBinaryExpr binary_expr;
        ASTUnaryExpr unary_expr;
        ASTTernaryExpr ternary_expr;
        ASTIfStmt if_stmt;
        ASTWhileStmt while_stmt;
        ASTForStmt for_stmt;
        ASTSwitchStmt switch_stmt;
        ASTCaseStmt case_stmt;
        ASTReturnStmt return_stmt;
        ASTFunctionCall function_call;
        ASTArrayAccess array_access;
        ASTMemberAccess member_access;
        ASTIdentifier identifier;
        ASTLiteral literal;
        ASTParameter parameter;  // Campo adicionado para AST_PARAMETER
    } data;
} ASTNode;

// Funções para manipulação da AST
ASTNode* ast_create_node(ASTNodeType type);
void ast_add_child(ASTNode* parent, ASTNode* child);
void ast_destroy(ASTNode* node);
void ast_print(ASTNode* node, int indent);
const char* ast_node_type_to_string(ASTNodeType type);
const char* data_type_to_string(DataType type);
const char* unary_operator_to_string(UnaryOperator op);

#endif