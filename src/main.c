#include <stdlib.h>

#include "SKN/arena.h"
#include <SKN/types.h>

#include "haruka.h"

void main_file(Arena *arena, Node *body, Haruka hrk)
{
    include(arena, body, "stdio.h", false);
    include(arena, body, "stdlib.h", false);
    include(arena, body, "SKN/types.h", false);

    Node *f = function(arena, body, hrk.type_i32, "main", parameter(arena, hrk.type_void, NULL), NULL);
    {
        Node *f_block = f->function.block;
        call(arena, f_block, "printf", string_make(arena, "Hello, World\\n"), NULL);
        return_stmt(arena, f_block, identifier_make(arena, "EXIT_SUCCESS"));
    }
}

i32 main(void)
{
    Arena arena = arena_create(MB(1));

    Haruka hrk = haruka_create(&arena);
    Program program = program_create(&arena);
    main_file(&arena, program.file->body, hrk);
    program_generate(&arena, &program);
    program_compile(&arena, &program);

    arena_destroy(&arena);
    return EXIT_SUCCESS;
}
