#include "lexer.h"

// Palavras-chave da linguagem C baseadas na gramática
static const struct
{
    const char *keyword;
    TokenType type;
} keywords[] = {
    // Tipos
    {"int", TOKEN_INT},
    {"float", TOKEN_FLOAT_KW},
    {"char", TOKEN_CHAR_KW},
    {"void", TOKEN_VOID},

    // Modificadores
    {"static", TOKEN_STATIC},
    {"extern", TOKEN_EXTERN},
    {"const", TOKEN_CONST},
    {"volatile", TOKEN_VOLATILE},
    {"typedef", TOKEN_TYPEDEF},

    // Estruturas
    {"struct", TOKEN_STRUCT},
    {"union", TOKEN_UNION},
    {"enum", TOKEN_ENUM},

    // Controle
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"do", TOKEN_DO},
    {"switch", TOKEN_SWITCH},
    {"case", TOKEN_CASE},
    {"default", TOKEN_DEFAULT},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},

    // Valores
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},

    // Preprocessador
    {"include", TOKEN_INCLUDE},
    {"define", TOKEN_DEFINE},
    {"ifdef", TOKEN_IFDEF},
    {"endif", TOKEN_ENDIF},

    {NULL, TOKEN_ERROR}};

Lexer *lexer_create(const char *source)
{
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = strdup(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->length = strlen(source);
    return lexer;
}

void lexer_destroy(Lexer *lexer)
{
    if (lexer)
    {
        free(lexer->source);
        free(lexer);
    }
}

static char lexer_current_char(Lexer *lexer)
{
    if (lexer->position >= lexer->length)
    {
        return '\0';
    }
    return lexer->source[lexer->position];
}

static char lexer_peek_char(Lexer *lexer)
{
    if (lexer->position + 1 >= lexer->length)
    {
        return '\0';
    }
    return lexer->source[lexer->position + 1];
}

static char lexer_peek_char_n(Lexer *lexer, int n)
{
    if (lexer->position + n >= lexer->length)
    {
        return '\0';
    }
    return lexer->source[lexer->position + n];
}

static void lexer_advance(Lexer *lexer)
{
    if (lexer->position < lexer->length)
    {
        if (lexer->source[lexer->position] == '\n')
        {
            lexer->line++;
            lexer->column = 1;
        }
        else
        {
            lexer->column++;
        }
        lexer->position++;
    }
}

static void lexer_skip_whitespace(Lexer *lexer)
{
    while (lexer_current_char(lexer) != '\0' && isspace(lexer_current_char(lexer)))
    {
        lexer_advance(lexer);
    }
}

static void lexer_skip_comment(Lexer *lexer)
{
    if (lexer_current_char(lexer) == '/' && lexer_peek_char(lexer) == '/')
    {
        // Comentário de linha
        while (lexer_current_char(lexer) != '\n' && lexer_current_char(lexer) != '\0')
        {
            lexer_advance(lexer);
        }
    }
    else if (lexer_current_char(lexer) == '/' && lexer_peek_char(lexer) == '*')
    {
        // Comentário de bloco
        lexer_advance(lexer); // '/'
        lexer_advance(lexer); // '*'
        while (lexer_current_char(lexer) != '\0')
        {
            if (lexer_current_char(lexer) == '*' && lexer_peek_char(lexer) == '/')
            {
                lexer_advance(lexer); // '*'
                lexer_advance(lexer); // '/'
                break;
            }
            lexer_advance(lexer);
        }
    }
}

static Token lexer_read_identifier(Lexer *lexer)
{
    Token token;
    token.line = lexer->line;
    token.column = lexer->column;

    int start = lexer->position;
    while (isalnum(lexer_current_char(lexer)) || lexer_current_char(lexer) == '_')
    {
        lexer_advance(lexer);
    }

    int length = lexer->position - start;
    token.value = malloc(length + 1);
    strncpy(token.value, &lexer->source[start], length);
    token.value[length] = '\0';

    // Verificar se é palavra-chave
    token.type = TOKEN_IDENTIFIER;
    for (int i = 0; keywords[i].keyword != NULL; i++)
    {
        if (strcmp(token.value, keywords[i].keyword) == 0)
        {
            token.type = keywords[i].type;
            break;
        }
    }

    return token;
}

static Token lexer_read_number(Lexer *lexer)
{
    Token token;
    token.line = lexer->line;
    token.column = lexer->column;

    int start = lexer->position;
    int has_dot = 0;

    // Ler apenas dígitos e um ponto decimal (máximo)
    while (isdigit(lexer_current_char(lexer)) ||
           (lexer_current_char(lexer) == '.' && !has_dot))
    {
        if (lexer_current_char(lexer) == '.')
        {
            has_dot = 1;
        }
        lexer_advance(lexer);
    }

    int length = lexer->position - start;
    token.value = malloc(length + 1);
    strncpy(token.value, &lexer->source[start], length);
    token.value[length] = '\0';

    // Simples: tem ponto = float, senão = number
    token.type = has_dot ? TOKEN_FLOAT : TOKEN_NUMBER;

    return token;
}

static Token lexer_read_string(Lexer *lexer)
{
    Token token;
    token.type = TOKEN_STRING;
    token.line = lexer->line;
    token.column = lexer->column;

    lexer_advance(lexer); // Skip opening quote
    int start = lexer->position;

    // Buffer simples para a string processada
    char temp_buffer[1024];
    int buffer_pos = 0;

    while (lexer_current_char(lexer) != '"' && lexer_current_char(lexer) != '\0')
    {
        if (lexer_current_char(lexer) == '\\')
        {
            lexer_advance(lexer); // Skip escape character

            // Apenas escapes básicos mais comuns
            switch (lexer_current_char(lexer))
            {
            case 'n':
                temp_buffer[buffer_pos++] = '\n';
                break;
            case 't':
                temp_buffer[buffer_pos++] = '\t';
                break;
            case '\\':
                temp_buffer[buffer_pos++] = '\\';
                break;
            case '"':
                temp_buffer[buffer_pos++] = '"';
                break;
            default:
                // Para outros escapes, manter literal
                temp_buffer[buffer_pos++] = lexer_current_char(lexer);
                break;
            }
            lexer_advance(lexer);
        }
        else
        {
            temp_buffer[buffer_pos++] = lexer_current_char(lexer);
            lexer_advance(lexer);
        }
    }

    temp_buffer[buffer_pos] = '\0';
    token.value = strdup(temp_buffer);

    if (lexer_current_char(lexer) == '"')
    {
        lexer_advance(lexer); // Skip closing quote
    }

    return token;
}

static Token lexer_read_char(Lexer *lexer)
{
    Token token;
    token.type = TOKEN_CHAR;
    token.line = lexer->line;
    token.column = lexer->column;

    lexer_advance(lexer); // Skip opening quote
    int start = lexer->position;

    if (lexer_current_char(lexer) == '\\')
    {
        lexer_advance(lexer); // Skip escape character
        if (lexer_current_char(lexer) != '\0')
        {
            lexer_advance(lexer); // Skip escaped character
        }
    }
    else if (lexer_current_char(lexer) != '\'' && lexer_current_char(lexer) != '\0')
    {
        lexer_advance(lexer);
    }

    int length = lexer->position - start;
    token.value = malloc(length + 1);
    strncpy(token.value, &lexer->source[start], length);
    token.value[length] = '\0';

    if (lexer_current_char(lexer) == '\'')
    {
        lexer_advance(lexer); // Skip closing quote
    }

    return token;
}

Token lexer_next_token(Lexer *lexer)
{
    Token token;

    while (lexer_current_char(lexer) != '\0')
    {
        // Skip whitespace and comments in a loop
        int skipped_something = 1;
        while (skipped_something)
        {
            skipped_something = 0;

            // Skip whitespace
            if (isspace(lexer_current_char(lexer)))
            {
                lexer_skip_whitespace(lexer);
                skipped_something = 1;
            }

            // Skip comments
            if (lexer_current_char(lexer) == '/' &&
                (lexer_peek_char(lexer) == '/' || lexer_peek_char(lexer) == '*'))
            {
                lexer_skip_comment(lexer);
                skipped_something = 1;
            }
        }

        // Se chegamos ao fim após pular whitespace/comentários
        if (lexer_current_char(lexer) == '\0')
        {
            break;
        }

        token.line = lexer->line;
        token.column = lexer->column;
        token.value = NULL;

        char current = lexer_current_char(lexer);

        if (isalpha(current) || current == '_')
        {
            return lexer_read_identifier(lexer);
        }

        if (isdigit(current))
        {
            return lexer_read_number(lexer);
        }

        if (current == '"')
        {
            return lexer_read_string(lexer);
        }

        if (current == '\'')
        {
            return lexer_read_char(lexer);
        }

        // Operadores e delimitadores (resto do código igual)
        switch (current)
        {
        case '+':
            if (lexer_peek_char(lexer) == '+')
            {
                token.type = TOKEN_INCREMENT;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_PLUS;
            }
            break;
        case '-':
            if (lexer_peek_char(lexer) == '-')
            {
                token.type = TOKEN_DECREMENT;
                lexer_advance(lexer);
            }
            else if (lexer_peek_char(lexer) == '>')
            {
                token.type = TOKEN_ARROW;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_MINUS;
            }
            break;
        case '*':
            token.type = TOKEN_MULTIPLY;
            break;
        case '/':
            token.type = TOKEN_DIVIDE;
            break;
        case '%':
            token.type = TOKEN_MODULO;
            break;
        case '=':
            if (lexer_peek_char(lexer) == '=')
            {
                token.type = TOKEN_EQUAL;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '!':
            if (lexer_peek_char(lexer) == '=')
            {
                token.type = TOKEN_NOT_EQUAL;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_NOT;
            }
            break;
        case '<':
            if (lexer_peek_char(lexer) == '=')
            {
                token.type = TOKEN_LESS_EQUAL;
                lexer_advance(lexer);
            }
            else if (lexer_peek_char(lexer) == '<')
            {
                token.type = TOKEN_LEFT_SHIFT;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_LESS;
            }
            break;
        case '>':
            if (lexer_peek_char(lexer) == '=')
            {
                token.type = TOKEN_GREATER_EQUAL;
                lexer_advance(lexer);
            }
            else if (lexer_peek_char(lexer) == '>')
            {
                token.type = TOKEN_RIGHT_SHIFT;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_GREATER;
            }
            break;
        case '&':
            if (lexer_peek_char(lexer) == '&')
            {
                token.type = TOKEN_AND;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_BITWISE_AND;
            }
            break;
        case '|':
            if (lexer_peek_char(lexer) == '|')
            {
                token.type = TOKEN_OR;
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_BITWISE_OR;
            }
            break;
        case '^':
            token.type = TOKEN_BITWISE_XOR;
            break;
        case '~':
            token.type = TOKEN_BITWISE_NOT;
            break;
        case '?':
            token.type = TOKEN_QUESTION;
            break;
        case ':':
            token.type = TOKEN_COLON;
            break;
        case ';':
            token.type = TOKEN_SEMICOLON;
            break;
        case ',':
            token.type = TOKEN_COMMA;
            break;
        case '(':
            token.type = TOKEN_LPAREN;
            break;
        case ')':
            token.type = TOKEN_RPAREN;
            break;
        case '{':
            token.type = TOKEN_LBRACE;
            break;
        case '}':
            token.type = TOKEN_RBRACE;
            break;
        case '[':
            token.type = TOKEN_LBRACKET;
            break;
        case ']':
            token.type = TOKEN_RBRACKET;
            break;
        case '.':
            if (lexer_peek_char(lexer) == '.' && lexer_peek_char_n(lexer, 2) == '.')
            {
                token.type = TOKEN_ELLIPSIS;
                lexer_advance(lexer);
                lexer_advance(lexer);
            }
            else
            {
                token.type = TOKEN_DOT;
            }
            break;
        case '#':
            token.type = TOKEN_HASH;
            break;
        default:
            token.type = TOKEN_ERROR;
            break;
        }

        lexer_advance(lexer);
        return token;
    }

    // EOF
    token.type = TOKEN_EOF;
    token.value = NULL;
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_CHAR:
        return "CHAR";
    case TOKEN_INT:
        return "INT";
    case TOKEN_FLOAT_KW:
        return "FLOAT_KW";
    case TOKEN_CHAR_KW:
        return "CHAR_KW";
    case TOKEN_VOID:
        return "VOID";
    case TOKEN_STATIC:
        return "STATIC";
    case TOKEN_EXTERN:
        return "EXTERN";
    case TOKEN_CONST:
        return "CONST";
    case TOKEN_VOLATILE:
        return "VOLATILE";
    case TOKEN_TYPEDEF:
        return "TYPEDEF";
    case TOKEN_STRUCT:
        return "STRUCT";
    case TOKEN_UNION:
        return "UNION";
    case TOKEN_ENUM:
        return "ENUM";
    case TOKEN_IF:
        return "IF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_FOR:
        return "FOR";
    case TOKEN_DO:
        return "DO";
    case TOKEN_SWITCH:
        return "SWITCH";
    case TOKEN_CASE:
        return "CASE";
    case TOKEN_DEFAULT:
        return "DEFAULT";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_BREAK:
        return "BREAK";
    case TOKEN_CONTINUE:
        return "CONTINUE";
    case TOKEN_TRUE:
        return "TRUE";
    case TOKEN_FALSE:
        return "FALSE";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_MULTIPLY:
        return "MULTIPLY";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_MODULO:
        return "MODULO";
    case TOKEN_ASSIGN:
        return "ASSIGN";
    case TOKEN_EQUAL:
        return "EQUAL";
    case TOKEN_NOT_EQUAL:
        return "NOT_EQUAL";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_LESS_EQUAL:
        return "LESS_EQUAL";
    case TOKEN_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TOKEN_AND:
        return "AND";
    case TOKEN_OR:
        return "OR";
    case TOKEN_NOT:
        return "NOT";
    case TOKEN_BITWISE_AND:
        return "BITWISE_AND";
    case TOKEN_BITWISE_OR:
        return "BITWISE_OR";
    case TOKEN_BITWISE_XOR:
        return "BITWISE_XOR";
    case TOKEN_BITWISE_NOT:
        return "BITWISE_NOT";
    case TOKEN_LEFT_SHIFT:
        return "LEFT_SHIFT";
    case TOKEN_RIGHT_SHIFT:
        return "RIGHT_SHIFT";
    case TOKEN_INCREMENT:
        return "INCREMENT";
    case TOKEN_DECREMENT:
        return "DECREMENT";
    case TOKEN_QUESTION:
        return "QUESTION";
    case TOKEN_COLON:
        return "COLON";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_LBRACE:
        return "LBRACE";
    case TOKEN_RBRACE:
        return "RBRACE";
    case TOKEN_LBRACKET:
        return "LBRACKET";
    case TOKEN_RBRACKET:
        return "RBRACKET";
    case TOKEN_DOT:
        return "DOT";
    case TOKEN_ARROW:
        return "ARROW";
    case TOKEN_HASH:
        return "HASH";
    case TOKEN_INCLUDE:
        return "INCLUDE";
    case TOKEN_DEFINE:
        return "DEFINE";
    case TOKEN_IFDEF:
        return "IFDEF";
    case TOKEN_ENDIF:
        return "ENDIF";
    case TOKEN_ELLIPSIS:
        return "ELLIPSIS";
    default:
        return "UNKNOWN";
    }
}

void token_destroy(Token *token)
{
    if (token && token->value)
    {
        free(token->value);
        token->value = NULL;
    }
}