#include "syntax.h"

typedef enum
{
    STATE_START,
    STATE_NAME,
    STATE_ARGUMENTS_START,
    STATE_ARGUMENT,
    STATE_ARGUMENTS_END,
    STATE_END,
    STATE_LEN,
    STATE_UNKNOWN,
} State;

static const int states[STATE_LEN][TOKEN_TYPE_LEN] =
{
    // MACROS, LITERAL, NUMBER, STRING, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, OPENING_BRACE, CLOSING_BRACE, SEMICOLON, RETURN
    // START
    { STATE_UNKNOWN, STATE_NAME, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // NAME
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_ARGUMENTS_START, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // ARGUMENTS_START
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_ARGUMENT, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // ARGUMENT
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_ARGUMENTS_END, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // ARGUMENT_END
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_END, STATE_UNKNOWN },
    // END
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
};

static State get_next_state(State state, TokenType token_type)
{
    State next_state = states[state][token_type];
    if (next_state != STATE_UNKNOWN) return next_state;
    printf("get_block %i - %i\n", state, token_type);
    UNREACHABLE_RETURN(0);
}

FunctionCallResult get_function_call(Tokens *tokens, size_t offset) {
    FunctionCall function_call = { 0 };
    State state = STATE_START;
    size_t i = offset;
    int local_offset = -1;
    while(i < tokens->len)
    {
        Token token = tokens->items[i];
        State next_state = get_next_state(state, token.type);
        printf("get_function_call - %i - %i - ", state, next_state);
        token_print(&token);
        switch (next_state)
        {
        case STATE_ARGUMENTS_START:
        case STATE_ARGUMENTS_END:
            break;
        case STATE_NAME:
            function_call.name = token.value;
            function_call.name_len = token.len;
            break;
        case STATE_ARGUMENT:
            Expression expression = { token.value, token.len };
            DYNAMIC_ARRAY_ADD((&function_call.arguments), expression);
            break;
        case STATE_END:
            return (FunctionCallResult){ function_call, i };
        default:
            printf("get_function_call - %i is not covered\n", next_state);
            UNREACHABLE;
        }
        state = next_state;
        i++;
    }
    UNREACHABLE_RETURN((FunctionCallResult){ 0 });
}
