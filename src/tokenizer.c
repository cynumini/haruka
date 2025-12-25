#include "tokenizer.h"

enum state {
  s_start,
  s_macros,
  s_literal,
  s_symbol,
  s_string,
  s_string_esc,
  s_number,
  s_unknown,
  s_len,
};

enum char_type {
  ct_alphabet,
  ct_number_sign,
  ct_space,
  ct_symbol,
  ct_dot,
  ct_cr,
  ct_number,
  ct_double_quotes,
  ct_backslash,
  ct_len
};

enum char_type char_type_from_char(char c) {
  const size_t symbols_len = 8;
  const char symbols[] = {'<', '>', '(', ')', '{', '}', ',', ';'};
  if (c == '#') {
    return ct_number_sign;
  } else if (c == ' ') {
    return ct_space;
  } else if (c == '.') {
    return ct_dot;
  } else if (c == '\n') {
    return ct_cr;
  } else if (c == '"') {
    return ct_double_quotes;
  } else if (c == '\\') {
    return ct_backslash;
  } else if (isalpha(c)) {
    return ct_alphabet;
  } else if (isdigit(c)) {
    return ct_number;
  } else {
    for (size_t i = 0; i < symbols_len; i++) {
      if (symbols[i] == c) return ct_symbol;
    }
    printf("'%c' is not covered\n", c);
    unreachable;
  }
  return 0; // -Wall
}

static const enum state states[s_len][ct_len] = {
  { s_literal, s_macros, s_start, s_symbol, s_unknown, s_start, s_number, s_unknown, s_unknown},
  { s_macros, s_unknown, s_macros, s_macros, s_macros, s_start, s_unknown, s_unknown, s_unknown},
  { s_literal, s_unknown, s_start, s_symbol, s_unknown, s_unknown, s_literal, s_unknown, s_unknown},
  { s_unknown, s_unknown, s_start, s_symbol, s_unknown, s_start, s_unknown, s_string, s_unknown},
  { s_string, s_unknown, s_string, s_string, s_unknown, s_unknown, s_unknown, s_start, s_string_esc},
  { s_string, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown},
  { s_unknown, s_unknown, s_unknown, s_symbol, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown},
};

enum state get_next_state(enum state state, enum char_type char_type) {
  enum state next_state = states[state][char_type];
  if (next_state == s_unknown) {
    printf("%i - %i\n", state, char_type);
    unreachable;
  }
  return next_state;
}

void add_token(Tokens* tokens, enum state state, const char* value, size_t start, size_t end) {
  size_t index = tokens->len;
  if (index >= tokens->capacity) {
    if (tokens->capacity == 0) {
      tokens->capacity = 8;
    } else {
      tokens->capacity *= 2;
    }
    tokens->items = realloc(tokens->items,
			    sizeof(Token) * tokens->capacity);
  }
  TokenType token_type;
  switch (state) {
  case s_literal:
    if (strncmp(value, "return", 6) == 0) {
      token_type = tt_return;
    } else {
      token_type = tt_literal;
    }
    break;
  case s_macros:
    token_type = tt_macros;
    break;
  case s_symbol:
    switch (value[0]) {
    case ';':
      token_type = tt_semicolon;
      break;
    case '(':
      token_type = tt_open_parenthesis;
      break;
    case ')':
      token_type = tt_close_parenthesis;
      break;
    case '{':
      token_type = tt_opening_brace;
      break;
    case '}':
      token_type = tt_closing_brace;
      break;
    default:
      printf("add '%c'\n", value[0]);
      unreachable;
      break;
    }
    break;
  case s_string:
    token_type = tt_string;
    break;
  case s_number:
    token_type = tt_number;
    break;
  default:
    printf("state: %i\n", state);
    unreachable;
  }
  tokens->items[index] = (Token){token_type, value, end - start};
  tokens->len++;
}

Tokens tokenize(const char* source_code) {
  Tokens tokens = {0};
  size_t len = strlen(source_code);
  enum state state = s_start;
  int start = -1;
  int end = -1;
  for (size_t i = 0; i < len; i++) {
    char c = source_code[i];
    enum char_type char_type = char_type_from_char(c);
    enum state next_state = get_next_state(state, char_type);
    //printf("%c %i %i\n", c, state, next_state);

    switch (next_state) {
    case s_start:
      if (start != -1 && end != -1) {
	end++;
	if (state == s_string) {
	  end++;
	}
	add_token(&tokens, state, &source_code[start], start, end);
	start = -1;
	end = -1;
      }      
      break;
    case s_string:
    case s_string_esc:
    case s_number:
    case s_literal:
    case s_macros:
      if (start == -1)
	start = i;
      else
	end = i;
      break;
    case s_symbol:
      if (start != -1) {
	if (end == -1) end = start;
	end++;
	add_token(&tokens, state, &source_code[start], start, end);
	start = -1;
	end = -1;
      }
      add_token(&tokens, next_state, &source_code[i], i, i + 1);
      break;
    default:
      printf("%i is not covered\n", next_state);
      unreachable;
    }
    state = next_state;
  }

  return tokens;
}

void token_print(const Token * token) {
  switch (token->type) {
  case tt_macros:
    printf("tt_macros: ");
    break;
  case tt_literal:
    printf("tt_literal: ");
    break;
  case tt_number:
    printf("tt_number: ");
    break;
  case tt_string:
    printf("tt_string: ");
    break;
  case tt_open_parenthesis:
    printf("tt_open_parenthesis: ");
    break;
  case tt_close_parenthesis:
      printf("tt_close_parenthesis: ");
      break;
  case tt_opening_brace:
      printf("tt_opening_brace: ");
      break;
  case tt_closing_brace:
      printf("tt_closing_brace: ");
      break;
  case tt_semicolon:
      printf("tt_semicolon: ");
      break;
  case tt_return:
      printf("tt_return: ");
      break;
  case tt_len:
      unreachable;
  }
  for (size_t i = 0; i < token->len; i++) {
    putchar(token->value[i]);
  }
  putchar('\n');
}
