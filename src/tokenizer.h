#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

typedef enum
{
    TOKEN_TYPE_MACROS,
    TOKEN_TYPE_LITERAL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_OPEN_PARENTHESIS,
    TOKEN_TYPE_CLOSE_PARENTHESIS,
    TOKEN_TYPE_OPENING_BRACE,
    TOKEN_TYPE_CLOSING_BRACE,
    TOKEN_TYPE_SEMICOLON,
    TOKEN_TYPE_RETURN,
    TOKEN_TYPE_LEN,
} TokenType;

typedef struct
{
    TokenType type;
    const char *value; // value: [*:0]const char
    size_t len;
} Token;

typedef struct
{
    Token *items;
    size_t capacity;
    size_t len;
} Tokens;

Tokens tokenize(const char *source_code);
void token_print(const Token *token);

#endif
