#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

enum token_type {
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
};

struct token {
  enum token_type type;
  char* value;
  size_t len;
};

struct tokens {
  struct token* items;
  size_t capacity;
  size_t len;
};

struct tokens tokenize(char* source_code);
void token_print(const struct token * token);
#endif
