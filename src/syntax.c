#include "syntax.h"

typedef enum
{
    STATE_START,
    STATE_MACROS,
    STATE_LITERAL_1,
    STATE_LITERAL_2,
    STATE_FUNCTION,
    STATE_LEN,
    STATE_UNKNOWN,
} State;

static const int states[STATE_LEN][TOKEN_TYPE_LEN] =
{
    // MACROS, LITERAL, NUMBER, STRING, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, OPENING_BRACE, CLOSING_BRACE, SEMICOLON, RETURN
    // STATE_START
    { STATE_MACROS, STATE_LITERAL_1, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // STATE_MACROS
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // STATE_LITERAL_1
    { STATE_UNKNOWN, STATE_LITERAL_2, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // STATE_LITERAL_2
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_FUNCTION, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
    // STATE_FUNCTION
    { STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN, STATE_UNKNOWN },
};

static State get_next_state(State state, TokenType token_type)
{
    State next_state = states[state][token_type];
    if (next_state != STATE_UNKNOWN) return next_state;
    printf("%i - %i\n", state, token_type);
    UNREACHABLE_RETURN(0);
}

typedef struct {
    Element *items;
    size_t capacity;
    size_t len;
} Elements;

static void print_element(const Element *element) {
    const char *value = NULL;
    const char *type = NULL;
    size_t len = 0;
    switch (element->type)
    {
        case ELEMENT_TYPE_MACROS:
            value = element->value.macros.value;
            len = element->value.macros.len;
            for (size_t i = 0; i < len; i++)
            {
                putchar(value[i]);
            }
            putchar('\n');
            return;
        case ELEMENT_TYPE_FUNCTION:
            Function function = element->value.function;
            for (size_t i = 0; i < function.return_type_len; i++)
            {
                putchar(function.return_type[i]);
            }
            printf(" ");
            for (size_t i = 0; i < function.name_len; i++)
            {
                putchar(function.name[i]);
            }
            putchar('\n');
            for (size_t i = 0; i < function.block.len; i++) {
                printf("\t");
                print_element(&function.block.items[i]);
            }
            return;
        case ELEMENT_TYPE_FUNCTION_CALL:
            FunctionCall function_call = element->value.function_call;
            for (size_t i = 0; i < function_call.name_len; i++)
            {
                putchar(function_call.name[i]);
            }
            putchar(' ');
            for (size_t i = 0; i < function_call.arguments.len; i++)
            {
                Expression expression = function_call.arguments.items[i];
                for (size_t i = 0; i < expression.name_len; i++)
                {
                    putchar(expression.name[i]);
                }
                putchar(',');
            }
            putchar('\n');
            return;
        case ELEMENT_TYPE_RETURN:
            Return return_element = element->value.return_element;
            Expression expression = return_element.expression;
            printf("return ");
            for (size_t i = 0; i < expression.name_len; i++)
            {
                putchar(expression.name[i]);
            }
            putchar('\n');
            return;
        default:
            printf("element->type = %i\n", element->type);
            UNREACHABLE;
    }

}

void get_syntax_tree(Tokens *tokens)
{
    Elements elements = { 0 };
    State state = STATE_START;
    size_t i = 0;
    int offset = -1;
    while(i < tokens->len)
    {
        Token token = tokens->items[i];
        State next_state = get_next_state(state, token.type);
        printf("%i - %i - ", state, next_state);
        token_print(&token);
        switch (next_state)
        {
        case STATE_LITERAL_2:
            break;
        case STATE_MACROS:
            {
                MacrosResult macros_result = get_macros(tokens, i);
                i = macros_result.offset;
                Element element = {ELEMENT_TYPE_MACROS, { .macros = macros_result.macros}};
                DYNAMIC_ARRAY_ADD((&elements), element);
                next_state = STATE_START;
            }
            break;
        case STATE_FUNCTION:
            {
                FunctionResult function_result = get_function(tokens, offset);
                i = function_result.offset;
                Element element = {ELEMENT_TYPE_FUNCTION, { .function = function_result.function }};
                DYNAMIC_ARRAY_ADD((&elements), element);
                next_state = STATE_START;
            }
            break;
        case STATE_LITERAL_1:
            offset = i;
            break;
        default:
            printf("%i is not covered\n", next_state);
            UNREACHABLE;
        }
        state = next_state;
        i++;
        printf("********\n");
        for (size_t j = 0; j < elements.len; j++)
        {
            print_element(&elements.items[j]);
        }
        printf("********\n");
    }
}
