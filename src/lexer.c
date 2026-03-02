#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SKN/errors.h>

static const char *token_type_to_string(TokenType token_type)
{
    switch (token_type)
    {

#define X(name)                                                                                                        \
    case name:                                                                                                         \
        return #name;
        TOKEN_TYPE_LIST(X)
#undef X
    }
    UNREACHABLE();
}

typedef enum State
{
    STATE_SPACE,
    STATE_IDENTIFIER,
    STATE_STRING,
    STATE_NUMBER,
    STATE_MACROS,
    STATE_HEADER_NAME,
    STATE_SINGLE_SYMBOL,
    // Special
    STATE_UNKNOWN,
    STATE_LEN,
} State;

typedef enum CharType
{
    CHAR_TYPE_NEWLINE,
    CHAR_TYPE_SPACE,
    CHAR_TYPE_LETTER,
    CHAR_TYPE_DIGIT,
    CHAR_TYPE_NUMBER_SIGN,
    CHAR_TYPE_PERIOD,
    CHAR_TYPE_DOUBLE_QUOTES,
    CHAR_TYPE_SINGLE_SYMBOL,
    CHAR_TYPE_LEN
} CharType;

static CharType get_char_type(char c)
{
    if (c == '\n')
    {
        return CHAR_TYPE_NEWLINE;
    }
    else if (isspace(c))
    {
        return CHAR_TYPE_SPACE;
    }
    else if (isalpha(c))
    {
        return CHAR_TYPE_LETTER;
    }
    else if (isdigit(c))
    {
        return CHAR_TYPE_DIGIT;
    }
    else if (c == '#')
    {
        return CHAR_TYPE_NUMBER_SIGN;
    }
    else if (c == '.')
    {
        return CHAR_TYPE_PERIOD;
    }
    else if (c == '"')
    {
        return CHAR_TYPE_DOUBLE_QUOTES;
    }
    else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '<' || c == '>' || c == ',' || c == '\\' || c == ';')
    {
        return CHAR_TYPE_SINGLE_SYMBOL;
    }

    ERROR("\"%c\" is unknown char type\n", c);

    abort();
}

static State get_next_state(State state, char c)
{
    CharType char_type = get_char_type(c);
    (void)char_type;

    switch (state)
    {
    case STATE_SPACE:
        if (char_type == CHAR_TYPE_NUMBER_SIGN)
        {
            return STATE_MACROS;
        }
        else if (char_type == CHAR_TYPE_LETTER)
        {
            return STATE_IDENTIFIER;
        }
        else if (char_type == CHAR_TYPE_DOUBLE_QUOTES)
        {
            return STATE_STRING;
        }
        else if (char_type == CHAR_TYPE_DIGIT)
        {
            return STATE_NUMBER;
        }
        else if (char_type == CHAR_TYPE_NEWLINE || char_type == CHAR_TYPE_SPACE)
        {
            return STATE_SPACE;
        }
        else if (char_type == CHAR_TYPE_SINGLE_SYMBOL)
        {
            return STATE_SINGLE_SYMBOL;
        }
        else
        {
            ERROR("%c + %i", c, state);
        }
        break;
    case STATE_IDENTIFIER:
        if (char_type == CHAR_TYPE_LETTER || char_type == CHAR_TYPE_DIGIT)
        {
            return STATE_IDENTIFIER;
        }
        else if (char_type == CHAR_TYPE_NEWLINE || char_type == CHAR_TYPE_SPACE)
        {
            return STATE_SPACE;
        }
        else if (char_type == CHAR_TYPE_SINGLE_SYMBOL)
        {
            return STATE_SINGLE_SYMBOL;
        }
        else
        {
            ERROR("%c + %i", c, state);
        }
        break;
    case STATE_STRING:
        if (char_type != CHAR_TYPE_DOUBLE_QUOTES)
        {
            return STATE_STRING;
        }
        else
        {
            return STATE_SPACE;
        }
        break;
    case STATE_NUMBER:
        if (char_type == CHAR_TYPE_SINGLE_SYMBOL)
        {
            return STATE_SINGLE_SYMBOL;
        }
        else
        {
            ERROR("%c + %i", c, state);
        }
        break;
    case STATE_MACROS:
        if (char_type != CHAR_TYPE_NEWLINE)
        {
            return STATE_MACROS;
        }
        else
        {
            return STATE_SPACE;
        }
        break;
    case STATE_HEADER_NAME:
        ERROR("%c + %i", c, state);
        break;
    case STATE_SINGLE_SYMBOL:
        ERROR("%c + %i", c, state);
        break;
    case STATE_UNKNOWN:
        ERROR("%c + %i", c, state);
        break;
    case STATE_LEN:
        ERROR("%c + %i", c, state);
        break;
    }

    ERROR("\"%c\" + state(%i) is undefined\n", c, state);
}

Lexer lexer_create(const char *text)
{
    return (Lexer){.text = text};
}

Token lexer_get_next_token(Lexer *lexer)
{
    usize start = lexer->position;
    usize end = lexer->position;
    State state = STATE_SPACE;

    TokenType type = TOKEN_TYPE_UNKNOWN;

    bool finished = false;

    usize len = strlen(lexer->text);

    for (usize i = end; i < len && !finished; i++)
    {
        char c = lexer->text[i];
        State next_state = get_next_state(state, c);
        switch (next_state)
        {
        case STATE_UNKNOWN:
        case STATE_LEN:
            UNREACHABLE();
            break;
        case STATE_SPACE:
            if (state == STATE_SPACE)
            {
                start++;
                if (start == len)
                {
                    type = TOKEN_TYPE_EOF;
                    start--;
                    finished = true;
                }
            }
            else
            {
                finished = true;
            }
            break;
        case STATE_IDENTIFIER:
            type = TOKEN_TYPE_IDENTIFIER;
            break;
        case STATE_STRING:
            type = TOKEN_TYPE_STRING;
            break;
        case STATE_NUMBER:
            type = TOKEN_TYPE_NUMBER;
            break;
        case STATE_MACROS:
            type = TOKEN_TYPE_MACROS;
            break;
        case STATE_HEADER_NAME:
            type = TOKEN_TYPE_HEADER_NAME;
            break;
        case STATE_SINGLE_SYMBOL:
            if (type == TOKEN_TYPE_UNKNOWN)
            {
                if (c == '(')
                {
                    type = TOKEN_TYPE_OPEN_PARENTHESIS;
                }
                else if (c == ')')
                {
                    type = TOKEN_TYPE_CLOSE_PARENTHESIS;
                }
                else if (c == '{')
                {
                    type = TOKEN_TYPE_OPENING_BRACE;
                }
                else if (c == '}')
                {
                    type = TOKEN_TYPE_CLOSING_BRACE;
                }
                else if (c == ';')
                {
                    type = TOKEN_TYPE_SEMICOLON;
                }
            }
            else
            {
                end--;
            }
            finished = true;
            break;
        }
        end++;
        state = next_state;
    }
    lexer->position = end;
    return (Token){.type = type, .text = lexer->text + start, .len = end - start};
}

void token_print(Token token)
{
    printf("%s: ", token_type_to_string(token.type));
    if (token.text[token.len - 1] == '\n')
    {
        if (token.len > 1)
        {
            fwrite(token.text, sizeof(char), token.len - 1, stdout);
        }
        printf("\\n");
    }
    else
    {
        fwrite(token.text, sizeof(char), token.len, stdout);
    }
    printf("\n");
}
