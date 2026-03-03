#include <assert.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <SKN/arena.h>
#include <SKN/array.h>
#include <SKN/errors.h>
#include <SKN/types.h>

DEFINE_DYNAMIC_ARRAY(Strings, char *);
DYNAMIC_ARRAY_IMPL_ADD(Strings, char *, strings_add)

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

Node *string_make(Arena *arena, const char *body)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

    node->node_type = NODE_TYPE_STRING;
    node->string.body = body;

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

Node *identifier_make(Arena *arena, const char *name)
{
    Node *node = ARENA_PUSH_STRUCT_ZERO(arena, Node);

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

i32 main(void)
{
    Arena arena = arena_create(MB(1));

    i32 result = mkdir("generated", 0755);
    assert(result == 0 || errno == EEXIST);

    char input_path[PATH_MAX];
    getcwd(input_path, PATH_MAX);
    strcat(input_path, "/generated/hello_world.c");

    // generate
    {

        FILE *file = fopen(input_path, "w");

        Node *program = block_create(&arena, true);
        nodes_add(&arena, &program->block.children, macro_create(&arena, "#include <stdio.h>"));
        nodes_add(&arena, &program->block.children, macro_create(&arena, "#include <stdlib.h>"));
        nodes_add(&arena, &program->block.children, macro_create(&arena, "#include <SKN/types.h>"));

        Nodes main_parameters = {0};
        nodes_add(&arena, &main_parameters, parameter_create(&arena, type_create(&arena, "void")));
        Node *main_block = block_create(&arena, false);
        nodes_add(&arena, &program->block.children,
                  function_create(&arena, type_create(&arena, "i32"), "main", main_parameters, main_block));

        Nodes printf_args = {0};
        nodes_add(&arena, &printf_args, string_make(&arena, "Hello, World!\\n"));
        nodes_add(&arena, &main_block->block.children, call_create(&arena, "printf", printf_args));

        nodes_add(&arena, &main_block->block.children,
                  return_stmt_make(&arena, identifier_make(&arena, "EXIT_SUCCESS")));

        node_fprintf(file, program);

        fclose(file);
    }

    // format
    {
        Strings argv = {0};
        strings_add(&arena, &argv, "clang-format");
        strings_add(&arena, &argv, "-i");
        strings_add(&arena, &argv, input_path);

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
        strings_add(&arena, &argv, "gcc");
        strings_add(&arena, &argv, input_path);
        strings_add(&arena, &argv, "-o");
        strings_add(&arena, &argv, output_path);
        strings_add(&arena, &argv, "-Isakana/include");
        strings_add(&arena, &argv, "-Lsakana/out");
        strings_add(&arena, &argv, "-lsakana");
        strings_add(&arena, &argv, NULL);

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

    arena_destroy(&arena);
    return EXIT_SUCCESS;
}
