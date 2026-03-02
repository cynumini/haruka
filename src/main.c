#include <assert.h>
#include <stdlib.h>

#include <SKN/arena.h>
#include <SKN/filesystem.h>
#include <SKN/types.h>

#include "lexer.h"
#include "parser.h"

DYNAMIC_ARRAY_IMPL_ADD(Nodes, Node, nodes_add)

int main(int argc, char *argv[])
{
    Arena arena = arena_create(MB(1));

    assert(argc == 3);

    const char *input_path = argv[1];
    const char *output_path = argv[2];

    const char *input_text = read_text(&arena, input_path);
    write_text(output_path, input_text);

    Lexer lexer = lexer_create(input_text);

    Node root = {
        .type = NODE_TYPE_ROOT,
    };

    nodes_add(&arena, &root.root.nodes, (Node){0});

    Node *current_node = &root.root.nodes.items[0];

    Token token;
    do
    {
        token = lexer_get_next_token(&lexer);
        current_node = parse(&arena, current_node, token);
        if (current_node->finished)
        {
            nodes_add(&arena, &root.root.nodes, (Node){0});
            Nodes *nodes = &root.root.nodes;
            current_node = &nodes->items[nodes->len - 1];
        }
    } while (token.type != TOKEN_TYPE_EOF);

    node_print(&root, 0);

    arena_destroy(&arena);

    return EXIT_SUCCESS;
}
