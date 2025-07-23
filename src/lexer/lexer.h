#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Tipos de tokens baseados na gramática fornecida
typedef enum
{
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHAR,

    // Palavras-chave - tipos
    TOKEN_INT,
    TOKEN_FLOAT_KW,
    TOKEN_CHAR_KW,
    TOKEN_VOID,

    // Palavras-chave - modificadores
    TOKEN_STATIC,
    TOKEN_EXTERN,
    TOKEN_CONST,
    TOKEN_VOLATILE,
    TOKEN_TYPEDEF,

    // Palavras-chave - estruturas
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_ENUM,

    // Palavras-chave - controle
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_DO,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,

    // Palavras-chave - valores
    TOKEN_TRUE,
    TOKEN_FALSE,

    // Operadores aritméticos
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,

    // Operadores de atribuição
    TOKEN_ASSIGN,

    // Operadores de comparação
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,

    // Operadores lógicos
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    // Operadores bitwise
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_XOR,
    TOKEN_BITWISE_NOT,
    TOKEN_LEFT_SHIFT,
    TOKEN_RIGHT_SHIFT,

    // Operadores de incremento/decremento
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,

    // Operador ternário
    TOKEN_QUESTION,
    TOKEN_COLON,

    // Delimitadores
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_DOT,
    TOKEN_ARROW,

    // Preprocessador
    TOKEN_HASH,
    TOKEN_INCLUDE,
    TOKEN_DEFINE,
    TOKEN_IFDEF,
    TOKEN_ENDIF,

    // Variadic
    TOKEN_ELLIPSIS,

    TOKEN_ERROR
} TokenType;

typedef struct
{
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

typedef struct
{
    char *source;
    int position;
    int line;
    int column;
    int length;
} Lexer;

// Funções do analisador léxico
Lexer *lexer_create(const char *source);
void lexer_destroy(Lexer *lexer);
Token lexer_next_token(Lexer *lexer);
const char *token_type_to_string(TokenType type);
void token_destroy(Token *token);

#endif