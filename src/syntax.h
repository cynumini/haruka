#ifndef SYNTAX_H
#define SYNTAX_H

#include "utils.h"
#include "tokenizer.h"

typedef struct {
    const char *name;
    size_t name_len;
} Expression;

typedef struct {
    Expression expression;
} Return;

typedef struct
{
    Return return_element;
    size_t offset;
} ReturnResult;

typedef struct
{
    Expression *items;
    size_t capacity;
    size_t len;
} Arguments;

typedef struct
{
    const char *name;
    size_t name_len;
    Arguments arguments;
} FunctionCall;

typedef struct
{
    FunctionCall function_call;
    size_t offset;
} FunctionCallResult;

typedef struct
{
    const char* value;
    size_t len;
} Macros;

typedef struct
{
    Macros macros;
    size_t offset;
} MacrosResult;

typedef struct {
    struct element *items;
    size_t capacity;
    size_t len;
} Block;

typedef struct
{
    Block block;
    size_t offset;
} BlockResult;

typedef struct
{
    const char* return_type;
    size_t return_type_len;
    const char* name;
    size_t name_len;
    Block block;
} Function;

typedef struct
{
    Function function;
    size_t offset;
} FunctionResult;

typedef enum {
    ELEMENT_TYPE_MACROS,
    ELEMENT_TYPE_FUNCTION,
    ELEMENT_TYPE_FUNCTION_CALL,
    ELEMENT_TYPE_RETURN,
} ElementType;

typedef union {
    Macros macros;
    Function function;
    FunctionCall function_call;
    Return return_element;
} ElementValue;

typedef struct element {
    ElementType type;
    ElementValue value;
} Element;

MacrosResult get_macros(Tokens *tokens, size_t offset);
FunctionResult get_function(Tokens *tokens, size_t offset);
BlockResult get_block(Tokens *tokens, size_t offset);
FunctionCallResult get_function_call(Tokens *tokens, size_t offset);
ReturnResult get_return(Tokens *tokens, size_t offset);

void get_syntax_tree(Tokens *tokens);

#endif
