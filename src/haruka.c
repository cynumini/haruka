#include "haruka.h"
#include "SKN/arena.h"

#include <assert.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <SKN/errors.h>

static Arena *current_arena;
static Node *current_node;

DEFINE_DYNAMIC_ARRAY(Strings, char *);
DYNAMIC_ARRAY_IMPL_ADD(Strings, char *, strings_add)

DYNAMIC_ARRAY_IMPL_ADD(Nodes, Node *, nodes_add)

Node *block_create(Arena *arena, bool is_root)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_BLOCK;
    node->block.is_root = is_root;

    return node;
}

Node *macro_create(Arena *arena, const char *body)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_MACRO;
    node->macro.body = body;

    return node;
}

Node *type_create(Arena *arena, const char *name)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_TYPE;
    node->type.name = name;

    return node;
}

Node *parameter_create(Arena *arena, Node *type)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_PARAMETER;
    node->parameter.type = type;

    return node;
}

Node *parameter_create_with_name(Arena *arena, Node *type, const char *name)
{

    Node *node = parameter_create(arena, type);

    node->parameter.has_name = true;
    node->parameter.name = name;

    return node;
}

// TODO: Nodes parameters vs Node *parameters
Node *function_create(Arena *arena, Node *type, const char *name, Nodes parameters, Node *body)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_FUNCTION;
    node->function.type = type;
    node->function.name = name;
    node->function.parameters = parameters;
    node->function.block = body;

    return node;
}

Node *string_make(const char *body)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(current_arena, Node);

    node->node_type = NODE_TYPE_STRING;
    node->string.body = arena_strdup(current_arena, body);

    return node;
}

// TODO: Nodes args vs Node *args
Node *call_create(Arena *arena, const char *name, Nodes args)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_CALL;
    node->needs_semicolon = true;
    node->call.name = name;
    node->call.args = args;

    return node;
}

Node *identifier_make(const char *name)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(current_arena, Node);

    node->node_type = NODE_TYPE_IDENTIFIER;
    node->call.name = name;

    return node;
}

Node *return_stmt_make(Arena *arena, Node *child)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_RETURN_STATEMENT;
    node->needs_semicolon = true;
    node->return_stmt.child = child;

    return node;
}

void node_fprintf(FILE *file, Node *node)
{
    switch (node->node_type)
    {

    case NODE_TYPE_BLOCK:
        if (!node->block.is_root)
        {
            fputc('{', file);
        }
        for (usize i = 0; i < node->block.children.len; i++)
        {
            Node *child = node->block.children.items[i];
            node_fprintf(file, child);
            if (child->needs_semicolon)
            {
                fputc(';', file);
            }
        }
        if (!node->block.is_root)
        {
            fputc('}', file);
        }
        break;
    case NODE_TYPE_MACRO:
        fprintf(file, "%s\n", node->macro.body);
        break;
    case NODE_TYPE_TYPE:
        fputs(node->type.name, file);
        break;
    case NODE_TYPE_FUNCTION:
        node_fprintf(file, node->function.type);
        fprintf(file, " %s(", node->function.name);
        for (usize i = 0; i < node->function.parameters.len; i++)
        {
            if (i != 0)
            {
                fputc(',', file);
            }
            Node *child = node->function.parameters.items[i];
            node_fprintf(file, child);
        }
        fputc(')', file);
        node_fprintf(file, node->function.block);
        break;
    case NODE_TYPE_PARAMETER:
        node_fprintf(file, node->parameter.type);
        if (node->parameter.has_name)
        {
            fprintf(file, " %s", node->parameter.name);
        }
        break;
    case NODE_TYPE_EXPRESSION:
        UNREACHABLE();
        break;
    case NODE_TYPE_STRING:
        fprintf(file, "\"%s\"", node->string.body);
        break;
    case NODE_TYPE_CALL:
        fprintf(file, "%s(", node->call.name);
        for (usize i = 0; i < node->call.args.len; i++)
        {
            if (i != 0)
            {
                fputc(',', file);
            }
            Node *child = node->call.args.items[i];
            node_fprintf(file, child);
        }
        fputc(')', file);
        break;
    case NODE_TYPE_RETURN_STATEMENT:
        fprintf(file, "return ");
        node_fprintf(file, node->return_stmt.child);
        break;
    case NODE_TYPE_IDENTIFIER:
        fputs(node->identifier.name, file);
        break;
    }
}

void include(const char *name, bool from_current_dir)
{
    assert(current_node->node_type == NODE_TYPE_BLOCK);
    const char *base = "#include ";
    char *body = (char *)arena_push_zero(current_arena, strlen(base) + strlen(name) + 2 + 1);
    if (from_current_dir)
    {
        sprintf(body, "#include \"%s\"", name);
    }
    else
    {
        sprintf(body, "#include <%s>", name);
    }
    Node *self = macro_create(current_arena, body);
    self->parent = current_node;
    nodes_add(current_arena, &current_node->block.children, self);
}

Node *parameter(Node *type, const char *name)
{
    if (name)
    {
        return parameter_create_with_name(current_arena, type, name);
    }
    else
    {
        return parameter_create(current_arena, type);
    }
}

void function(Node *type, const char *name, ...)
{
    Nodes parameters = {0};
    va_list va;
    va_start(va, name);
    for (Node *child = va_arg(va, Node *); child != NULL; child = va_arg(va, Node *))
    {
        nodes_add(current_arena, &parameters, child);
    }
    va_end(va);
    Node *self = function_create(current_arena, type, name, parameters, block_create(current_arena, false));
    nodes_add(current_arena, &current_node->block.children, self);
    current_node = self;
}

void function_end(void)
{
    current_node = current_node->parent;
}

Node *call(const char *name, ...)
{
    Node *root = current_node->function.block;
    Nodes args = {0};
    va_list va;
    va_start(va, name);
    for (Node *child = va_arg(va, Node *); child != NULL; child = va_arg(va, Node *))
    {
        nodes_add(current_arena, &args, child);
    }
    va_end(va);
    Node *self = call_create(current_arena, name, args);
    nodes_add(current_arena, &root->block.children, self);
    return self;
}

Node *return_stmt(Node *child)
{
    Node *root = current_node->function.block;
    Node *self = return_stmt_make(current_arena, child);
    nodes_add(current_arena, &root->block.children, self);
    return self;
}

File *file_create(Arena *arena, const char *name)
{
    File *file = ARENA_PUSH_STRUCT_ZERO(arena, File);
    file->name = name;
    file->body = block_create(arena, true);
    return file;
}

Haruka haruka_create(Arena *arena)
{
    return (Haruka){
        .type_i32 = type_create(arena, "i32"),
        .type_void = type_create(arena, "void"),
    };
}

Program program_create(Arena *arena)
{
    return (Program){.file = file_create(arena, "main")};
}

static void make_dir(const char *name)
{
    i32 result = mkdir(name, 0755);
    assert(result == 0 || errno == EEXIST);
}

static void get_relative_path(char *dest, const char *path)
{
    getcwd(dest, PATH_MAX);
    strcat(dest, path);
}

void program_generate(Arena *arena, Program *self)
{
    make_dir("generated");
    char input_path[PATH_MAX];
    get_relative_path(input_path, "/generated/");
    strcat(input_path, self->file->name);
    strcat(input_path, ".c");
    FILE *file = fopen(input_path, "w");
    node_fprintf(file, self->file->body);
    fclose(file);
    self->file->input_path = arena_strdup(arena, input_path);
}

void program_compile(Arena *arena, Program *self)
{
    char *input_path = self->file->input_path;
    // format
    {
        Strings argv = {0};
        strings_add(arena, &argv, "clang-format");
        strings_add(arena, &argv, "-i");
        strings_add(arena, &argv, input_path);

        pid_t pid = fork();
        if (pid == 0)
        {
            execv("/usr/bin/clang-format", argv.items);
        }
        else
        {
            assert(wait(NULL) != -1);
        }
    }

    // compile
    {
        i32 result = mkdir("out", 0755);
        assert(result == 0 || errno == EEXIST);

        char output_path[PATH_MAX];
        getcwd(output_path, PATH_MAX);
        strcat(output_path, "/out/hello_world");

        Strings argv = {0};
        strings_add(arena, &argv, "gcc");
        strings_add(arena, &argv, input_path);
        strings_add(arena, &argv, "-o");
        strings_add(arena, &argv, output_path);
        strings_add(arena, &argv, "-Isakana/include");
        strings_add(arena, &argv, "-Lsakana/out");
        strings_add(arena, &argv, "-lsakana");
        strings_add(arena, &argv, NULL);

        pid_t pid = fork();
        if (pid == 0)
        {
            execv("/usr/bin/gcc", argv.items);
        }
        else
        {
            assert(wait(NULL) != -1);
        }
    }
}

void haruka_begin(Arena *arena, Node *node)
{
    current_arena = arena;
    current_node = node;
}
