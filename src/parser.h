#ifndef PARSER_H
#define PARSER_H

#include "SKN/arena.h"
#include "lexer.h"

#include <SKN/array.h>

typedef struct Node Node;

DEFINE_DYNAMIC_ARRAY(Nodes, Node);

typedef enum NodeType
{
    NODE_TYPE_UNKNOWN,
    NODE_TYPE_ROOT,
    NODE_TYPE_MACROS,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_FUNCTION_CALL,
} NodeType;

typedef struct String
{
    const char *text;
    usize len;
} String;

typedef struct Parameter
{
    String type;
    bool has_name;
    String name;
} Parameter;

DEFINE_DYNAMIC_ARRAY(Parameters, Parameter);

typedef enum FunctionState
{
    FUNCTION_STATE_START,
    FUNCTION_STATE_OPEN_PARENTHESIS,
    FUNCTION_STATE_TYPE,
    FUNCTION_STATE_CLOSE_PARENTHESIS,
    FUNCTION_STATE_OPENING_BRACE,
} FunctionState;

typedef struct FunctionNode
{
    FunctionState state;
    String type;
    String name;
    Parameters parameters;
    Nodes body;
} FunctionNode;

typedef enum FunctionCallState
{
    FUNCTION_CALL_STATE_START,
} FunctionCallState;

typedef struct FunctionCallNode
{
    FunctionCallState state;
    String name;
} FunctionCallNode;

typedef struct Node
{
    NodeType type;
    Node *parent;
    union {
        struct
        {
            u8 state;
            Token one;
        } unknown;
        struct
        {
            Nodes nodes;
        } root;
        struct
        {
            String body;
        } macros;
        FunctionNode function;
        FunctionCallNode function_call;
    };
    bool finished;
} Node;

String string_from_token(Token token);
Node *parse(Arena *arena, Node *node, Token token);
void node_print(Node *node, usize level);

#endif /* end of include guard: PARSER_H */
