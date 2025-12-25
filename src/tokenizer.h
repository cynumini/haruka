#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

typedef enum
{
  tt_macros,
  tt_literal,
  tt_number,
  tt_string,
  tt_open_parenthesis,
  tt_close_parenthesis,
  tt_opening_brace,
  tt_closing_brace,
  tt_semicolon,
  tt_return,
  tt_len,
} TokenType;

typedef struct
{
  TokenType type;
  const char* value;
  size_t len;
} Token;

typedef struct
{
  Token* items;
  size_t capacity;
  size_t len;
} Tokens;

Tokens tokenize(const char* source_code);
void token_print(const Token * token);
#endif
