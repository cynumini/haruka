#include "parser.h"

#include <SKN/errors.h>
#include <stdio.h>

String string_from_token(Token token)
{
    return (String){.text = token.text, .len = token.len};
}

DYNAMIC_ARRAY_IMPL_ADD(Parameters, Parameter, parameters_add)
DYNAMIC_ARRAY_IMPL_ADD(Nodes, Node, nodes_add)

Node *parse(Arena *arena, Node *node, Token token)
{
    token_print(token);
    switch (token.type)
    {

    case TOKEN_TYPE_UNKNOWN:
        UNREACHABLE();
        break;
    case TOKEN_TYPE_IDENTIFIER:
        if (node->type == NODE_TYPE_UNKNOWN && node->unknown.state == 0)
        {
            node->unknown.state = 1;
            node->unknown.one = token;
        }
        else if (node->type == NODE_TYPE_UNKNOWN && node->unknown.state == 1)
        {
            Token type = node->unknown.one;
            node->type = NODE_TYPE_FUNCTION;
            node->function.state = FUNCTION_STATE_START;
            node->function.type = string_from_token(type);
            node->function.name = string_from_token(token);
        }
        else if (node->type == NODE_TYPE_FUNCTION && node->function.state == FUNCTION_STATE_OPEN_PARENTHESIS)
        {
            FunctionNode *function = &node->function;
            function->state = FUNCTION_STATE_TYPE;
            parameters_add(arena, &function->parameters,
                           (Parameter){.type = string_from_token(token), .has_name = false});
        }
        else
        {
            UNREACHABLE();
        }
        break;
    case TOKEN_TYPE_STRING:
        UNREACHABLE();
        break;
    case TOKEN_TYPE_NUMBER:
        UNREACHABLE();
        break;
    case TOKEN_TYPE_MACROS:
        node->type = NODE_TYPE_MACROS;
        node->macros.body = string_from_token(token);
        node->finished = true;
        return node;
    case TOKEN_TYPE_HEADER_NAME:
        UNREACHABLE();
        break;
    case TOKEN_TYPE_OPEN_PARENTHESIS:
        if (node->type == NODE_TYPE_FUNCTION && node->function.state == FUNCTION_STATE_START)
        {
            node->function.state = FUNCTION_STATE_OPEN_PARENTHESIS;
        }
        else if (node->type == NODE_TYPE_UNKNOWN)
        {
            Token name = node->unknown.one;
            node->type = NODE_TYPE_FUNCTION_CALL;
            node->function_call.state = FUNCTION_CALL_STATE_START;
            node->function_call.name = string_from_token(name);
        }
        else
        {
            UNREACHABLE();
        }
        break;
    case TOKEN_TYPE_CLOSE_PARENTHESIS:
        if (node->type == NODE_TYPE_FUNCTION && node->function.state == FUNCTION_STATE_TYPE)
        {
            node->function.state = FUNCTION_STATE_CLOSE_PARENTHESIS;
        }
        else
        {
            UNREACHABLE();
        }
        break;
    case TOKEN_TYPE_OPENING_BRACE:
        if (node->type == NODE_TYPE_FUNCTION && node->function.state == FUNCTION_STATE_CLOSE_PARENTHESIS)
        {
            node->function.state = FUNCTION_STATE_OPENING_BRACE;
            nodes_add(arena, &node->function.body, (Node){0});
            Nodes *body = &node->function.body;
            Node *child = &body->items[0];
            child->parent = node;
            return child;
        }
        else
        {
            UNREACHABLE();
        }
        break;
    case TOKEN_TYPE_CLOSING_BRACE:
        UNREACHABLE();
        break;
    case TOKEN_TYPE_SEMICOLON:
        UNREACHABLE();
        break;
    case TOKEN_TYPE_EOF:
        UNREACHABLE();
        break;
    }
    return node;
}

void node_print(Node *node, usize level)
{
    for (usize j = 0; j < level; j++)
    {
        putchar('\t');
    }
    switch (node->type)
    {
    case NODE_TYPE_UNKNOWN:
        UNREACHABLE();
        break;
    case NODE_TYPE_ROOT:
        printf("root\n");
        for (usize i = 0; i < node->root.nodes.len; i++)
        {
            Node *child = &node->root.nodes.items[i];
            node_print(child, level + 1);
        }
        break;
    case NODE_TYPE_MACROS:
        printf("macros ");
        fwrite(node->macros.body.text, sizeof(char), node->macros.body.len, stdout);
        putchar('\n');
        break;
    case NODE_TYPE_FUNCTION:
        UNREACHABLE();
        break;
    case NODE_TYPE_FUNCTION_CALL:
        UNREACHABLE();
        break;
    }
}
