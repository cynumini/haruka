#ifndef HARUKA_H
#define HARUKA_H

#include <SKN/arena.h>
#include <SKN/array.h>
#include <SKN/types.h>

typedef struct Node Node;

DEFINE_DYNAMIC_ARRAY(Nodes, Node *);

typedef enum NodeType
{
    NODE_TYPE_BLOCK,
    NODE_TYPE_MACRO,
    NODE_TYPE_TYPE,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_PARAMETER,
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_STRING,
    NODE_TYPE_CALL,
    NODE_TYPE_RETURN_STATEMENT,
    NODE_TYPE_IDENTIFIER,
} NodeType;

typedef struct Node
{
    NodeType node_type;
    Node *parent;
    union {
        struct
        {
            Nodes children;
            bool is_root;
        } block;
        struct
        {
            const char *body;
        } macro;
        struct
        {
            const char *name;
        } type;
        struct
        {
            Node *type;
            bool has_name;
            const char *name;
        } parameter;
        struct
        {
            Node *type;
            const char *name;
            Nodes parameters;
            Node *block;
        } function;
        struct
        {
            const char *body;
        } string;
        struct
        {
            const char *name;
            Nodes args;
        } call;
        struct
        {
            const char *name;
        } identifier;
        struct
        {
            Node *child;
        } return_stmt;
    };
    bool needs_semicolon;
} Node;

typedef struct File
{
    const char *name;
    char *input_path;
    Node *body;
} File;

typedef struct Program
{
    File *file;
} Program;

typedef struct Haruka
{
    Node *type_void;
    Node *type_i32;
} Haruka;

Node *string_make(Arena *arena, const char *body);
Node *identifier_make(Arena *arena, const char *name);
Haruka haruka_create(Arena *arena);
void include(Arena *arena, Node *root, const char *name, bool from_current_dir);
Node *function(Arena *arena, Node *root, Node *type, const char *name, ...);
Node *parameter(Arena *arena, Node *type, const char *name);
Node *call(Arena *arena, Node *root, const char *name, ...);
Node *return_stmt(Arena *arena, Node *root, Node *child);
Program program_create(Arena *arena);
void program_generate(Arena *arena, Program *self);
void program_compile(Arena *arena, Program *self);

#endif /* end of include guard: HARUKA_H */
