#include "tokenizer.h"

typedef enum
{
    STATE_START,
    STATE_MACROS,
    STATE_LITERAL,
    STATE_SYMBOL,
    STATE_STRING,
    STATE_STRING_ESC,
    STATE_NUMBER,
    STATE_LEN,
    STATE_UNKNOWN,
} State;

typedef enum
{
    CHAR_TYPE_ALPHABET,
    CHAR_TYPE_NUMBER_SIGN,
    CHAR_TYPE_SPACE,
    CHAR_TYPE_SYMBOL,
    CHAR_TYPE_DOT,
    CHAR_TYPE_CR,
    CHAR_TYPE_NUMBER,
    CHAR_TYPE_DOUBLE_QUOTES,
    CHAR_TYPE_BACKSLASH,
    CHAR_TYPE_LEN
} CharType;

static const State states[STATE_LEN][CHAR_TYPE_LEN] =
{
    // STATE_START
    { STATE_LITERAL, STATE_MACROS, STATE_START, STATE_SYMBOL, STATE_UNKNOWN, STATE_START, STATE_NUMBER, STATE_UNKNOWN, STATE_UNKNOWN},
    // STATE_MACROS
    { STATE_MACROS, STATE_UNKNOWN, STATE_MACROS, STATE_MACROS, STATE_MACROS, STATE_START, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN},
    // STATE_LITERAL
    { STATE_LITERAL, STATE_UNKNOWN, STATE_START, STATE_SYMBOL, STATE_UNKNOWN, STATE_UNKNOWN, STATE_LITERAL, STATE_UNKNOWN, STATE_UNKNOWN},
    // STATE_SYMBOL
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_START, STATE_SYMBOL, STATE_UNKNOWN, STATE_START, STATE_UNKNOWN, STATE_STRING, STATE_UNKNOWN},
    // STATE_STRING
    { STATE_STRING, STATE_UNKNOWN, STATE_STRING, STATE_STRING, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_START, STATE_STRING_ESC},
    // STATE_STRING_ESC
    { STATE_STRING, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN},
    // STATE_NUMBER
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_SYMBOL, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN},
};

static CharType char_type_from_char(const char c)
{
    const char symbols[] = {'<', '>', '(', ')', '{', '}', ',', ';'};
    if (c == '#')        return CHAR_TYPE_NUMBER_SIGN;
    else if (c == ' ')   return CHAR_TYPE_SPACE;
    else if (c == '.')   return CHAR_TYPE_DOT;
    else if (c == '\n')  return CHAR_TYPE_CR;
    else if (c == '"')   return CHAR_TYPE_DOUBLE_QUOTES;
    else if (c == '\\')  return CHAR_TYPE_BACKSLASH;
    else if (isalpha(c)) return CHAR_TYPE_ALPHABET;
    else if (isdigit(c)) return CHAR_TYPE_NUMBER;
    else for (size_t i = 0; i < ARRAY_LEN(symbols); i++) if (symbols[i] == c) return CHAR_TYPE_SYMBOL;
    printf("'%c' is not covered\n", c);
    UNREACHABLE_RETURN(0);
}

State get_next_state(State state, CharType char_type)
{
    State next_state = states[state][char_type];
    if (next_state != STATE_UNKNOWN) return next_state;
    printf("%i - %i\n", state, char_type);
    UNREACHABLE_RETURN(0);
}

void add_token(Tokens *tokens, State state, const char *value, size_t start, size_t end)
{
    TokenType token_type;
    switch (state)
    {
    case STATE_STRING: token_type = TOKEN_TYPE_STRING; break;
    case STATE_NUMBER: token_type = TOKEN_TYPE_NUMBER; break;
    case STATE_MACROS: token_type = TOKEN_TYPE_MACROS; break;
    case STATE_LITERAL:
        if (strncmp(value, "return", 6) == 0) token_type = TOKEN_TYPE_RETURN;
        else token_type = TOKEN_TYPE_LITERAL;
        break;
    case STATE_SYMBOL:
        switch (value[0])
        {
        case ';': token_type = TOKEN_TYPE_SEMICOLON; break;
        case '(': token_type = TOKEN_TYPE_OPEN_PARENTHESIS; break;
        case ')': token_type = TOKEN_TYPE_CLOSE_PARENTHESIS; break;
        case '{': token_type = TOKEN_TYPE_OPENING_BRACE; break;
        case '}': token_type = TOKEN_TYPE_CLOSING_BRACE; break;
        default: printf("add '%c'\n", value[0]); UNREACHABLE; break;
        }
        break;
    default: printf("state: %i\n", state); UNREACHABLE;
    }
    Token token = (Token){token_type, value, end - start};
    DYNAMIC_ARRAY_ADD(tokens, token);
}

Tokens tokenize(const char *source_code)
{
    Tokens tokens = { 0 };
    size_t len = strlen(source_code);
    State state = STATE_START;
    int start = -1;
    int end = -1;
    for (size_t i = 0; i < len; i++)
    {
        char c = source_code[i];
        CharType char_type = char_type_from_char(c);
        State next_state = get_next_state(state, char_type);
        //printf("%c %i %i\n", c, state, next_state);

        switch (next_state)
        {
        case STATE_START:
            if (start != -1 && end != -1)
            {
                if (state == STATE_STRING) end++;
                add_token(&tokens, state, &source_code[start], start, end + 1);
                start = end = -1;
            }
            break;
        case STATE_STRING:
        case STATE_STRING_ESC:
        case STATE_NUMBER:
        case STATE_LITERAL:
        case STATE_MACROS:
            if (start == -1) start = i;
            else end = i;
            break;
        case STATE_SYMBOL:
            if (start != -1) {
                if (end == -1) end = start;
                add_token(&tokens, state, &source_code[start], start, end + 1);
                start = end = -1;
            }
            add_token(&tokens, next_state, &source_code[i], i, i + 1);
            break;
        default:
            printf("%i is not covered\n", next_state);
            UNREACHABLE;
        }
        state = next_state;
    }
    return tokens;
}

void token_print(const Token *token)
{
    switch (token->type)
    {
    case TOKEN_TYPE_MACROS: printf("TOKEN_TYPE_MACROS: "); break;
    case TOKEN_TYPE_LITERAL: printf("TOKEN_TYPE_LITERAL: "); break;
    case TOKEN_TYPE_NUMBER: printf("TOKEN_TYPE_NUMBER: "); break;
    case TOKEN_TYPE_STRING: printf("TOKEN_TYPE_STRING: "); break;
    case TOKEN_TYPE_OPEN_PARENTHESIS: printf("TOKEN_TYPE_OPEN_PARENTHESIS: "); break;
    case TOKEN_TYPE_CLOSE_PARENTHESIS: printf("TOKEN_TYPE_CLOSE_PARENTHESIS: "); break;
    case TOKEN_TYPE_OPENING_BRACE: printf("TOKEN_TYPE_OPENING_BRACE: "); break;
    case TOKEN_TYPE_CLOSING_BRACE: printf("TOKEN_TYPE_CLOSING_BRACE: "); break;
    case TOKEN_TYPE_SEMICOLON: printf("TOKEN_TYPE_SEMICOLON: "); break;
    case TOKEN_TYPE_RETURN: printf("TOKEN_TYPE_RETURN: "); break;
    case TOKEN_TYPE_LEN: UNREACHABLE;;
    }

    for (size_t i = 0; i < token->len; i++)
    {
        putchar(token->value[i]);
    }

    putchar('\n');
}
