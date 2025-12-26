#include "syntax.h"

typedef enum
{
    STATE_START,
    STATE_LITERAL_1,
    STATE_LITERAL_2,
    STATE_FUNCTION_CALL,
    STATE_RETURN,
    STATE_END,
    STATE_LEN,
    STATE_UNKNOWN,
} State;

static const int states[STATE_LEN][TOKEN_TYPE_LEN] =
{
    // MACROS, LITERAL, NUMBER, STRING, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, OPENING_BRACE, CLOSING_BRACE, SEMICOLON, RETURN
    // START
    { STATE_UNKNOWN, STATE_LITERAL_1, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_END, STATE_UNKNOWN, STATE_RETURN },
    // LITERAL_1
    { STATE_UNKNOWN, STATE_LITERAL_2, STATE_UNKNOWN, STATE_UNKNOWN, STATE_FUNCTION_CALL, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // LITERAL_2
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // FUNCTION_CALL
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // RETURN
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
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

BlockResult get_block(Tokens *tokens, size_t offset) {
    Block block = { 0 };
    State state = STATE_START;
    size_t i = offset;
    int local_offset = -1;
    while(i < tokens->len)
    {
        Token token = tokens->items[i];
        printf("a - %li\n", i);
        State next_state = get_next_state(state, token.type);
        printf("get_block - %i - %i - ", state, next_state);
        token_print(&token);
        switch (next_state)
        {
        case STATE_LITERAL_2:
            break;
        case STATE_LITERAL_1:
            local_offset = i;
            break;
        case STATE_FUNCTION_CALL:
            {
                FunctionCallResult function_call_result = get_function_call(tokens, local_offset);
                i = function_call_result.offset;
                Element element = {ELEMENT_TYPE_FUNCTION_CALL, { .function_call = function_call_result.function_call }};
                DYNAMIC_ARRAY_ADD((&block), element);
                next_state = STATE_START;
            }
            break;
        case STATE_RETURN:
            {
                ReturnResult return_result = get_return(tokens, i);
                i = return_result.offset;
                Element element = {ELEMENT_TYPE_RETURN, { .return_element = return_result.return_element }};
                DYNAMIC_ARRAY_ADD((&block), element);
                next_state = STATE_START;
            }
            break;
        case STATE_END:
            return (BlockResult){ block, i };
        default:
            printf("get_block - %i is not covered\n", next_state);
            UNREACHABLE;
        }
        state = next_state;
        i++;
    }
    UNREACHABLE_RETURN((BlockResult){ 0 });
}
