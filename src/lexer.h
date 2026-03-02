#ifndef LEXER_H
#define LEXER_H

#include <SKN/types.h>

typedef struct Lexer
{
    const char *text;
    usize position;
} Lexer;

#define TOKEN_TYPE_LIST(X)                                                                                             \
    X(TOKEN_TYPE_UNKNOWN)                                                                                              \
    X(TOKEN_TYPE_IDENTIFIER)                                                                                           \
    X(TOKEN_TYPE_STRING)                                                                                               \
    X(TOKEN_TYPE_NUMBER)                                                                                               \
    X(TOKEN_TYPE_MACROS)                                                                                               \
    X(TOKEN_TYPE_HEADER_NAME)                                                                                          \
    X(TOKEN_TYPE_OPEN_PARENTHESIS)                                                                                     \
    X(TOKEN_TYPE_CLOSE_PARENTHESIS)                                                                                    \
    X(TOKEN_TYPE_OPENING_BRACE)                                                                                        \
    X(TOKEN_TYPE_CLOSING_BRACE)                                                                                        \
    X(TOKEN_TYPE_SEMICOLON)                                                                                            \
    X(TOKEN_TYPE_EOF)

typedef enum TokenType
{
#define X(name) name,
    TOKEN_TYPE_LIST(X)
#undef X
} TokenType;

typedef struct Token
{
    TokenType type;
    const char* text;
    usize len;
} Token;

Lexer lexer_create(const char *text);
Token lexer_get_next_token(Lexer *lexer);
void token_print(Token token);

#endif /* end of include guard: LEXER_H */
