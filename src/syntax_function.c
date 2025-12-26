#include "syntax.h"

typedef enum
{
    STATE_START,
    STATE_RETURN_TYPE,
    STATE_NAME,
    STATE_PARAMETER_START,
    STATE_PARAMETER_END,
    STATE_BLOCK_START,
    STATE_LEN,
    STATE_UNKNOWN,
} State;

static const int states[STATE_LEN][TOKEN_TYPE_LEN] =
{
    // MACROS, LITERAL, NUMBER, STRING, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, OPENING_BRACE, CLOSING_BRACE, SEMICOLON, RETURN
    // START
    { STATE_UNKNOWN, STATE_RETURN_TYPE, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // RETURN_TYPE
    { STATE_UNKNOWN, STATE_NAME, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // NAME
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_PARAMETER_START, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // PARAMETER_START
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_PARAMETER_END, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // PARAMETER_START
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_BLOCK_START, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // BLOCK_START
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
};

static State get_next_state(State state, TokenType token_type)
{
    State next_state = states[state][token_type];
    if (next_state != STATE_UNKNOWN) return next_state;
    printf("get_function %i - %i\n", state, token_type);
    UNREACHABLE_RETURN(0);
}

FunctionResult get_function(Tokens *tokens, size_t offset) {
    Function function = { 0 };
    State state = STATE_START;
    size_t i = offset;
    while(i < tokens->len)
    {
        Token token = tokens->items[i];
        State next_state = get_next_state(state, token.type);
        printf("get_function - %i - %i - ", state, next_state);
        token_print(&token);
        switch (next_state)
        {
        case STATE_PARAMETER_START:
        case STATE_PARAMETER_END:
            break;
        case STATE_RETURN_TYPE:
            function.return_type = token.value;
            function.return_type_len = token.len;
            break;
        case STATE_NAME:
            function.name = token.value;
            function.name_len = token.len;
            break;
        case STATE_BLOCK_START:
            {
                BlockResult block_result = get_block(tokens, i + 1);
                i = block_result.offset;
                function.block = block_result.block;
                return (FunctionResult){ function, i };
                break;
            }
            break;
        default:
            printf("get_function - %i is not covered\n", next_state);
            UNREACHABLE;
        }
        state = next_state;
        i++;
    }
    UNREACHABLE_RETURN((FunctionResult){ 0 });
}
