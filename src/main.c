#include <stdio.h>
#include <stdlib.h>

#include "SKN/arena.h"
#include <SKN/types.h>

#include "haruka.h"

void main_file(Arena *arena, Node *node, Haruka hrk)
{
    haruka_begin(arena, node);

    include("stdio.h", false);
    include("stdlib.h", false);
    include("SKN/types.h", false);

    function(hrk.type_i32, "main", parameter(hrk.type_void, NULL), NULL);
    {
        for (usize i = 0; i < 10; i++)
        {
            static char buf[256] = {0};
            snprintf(buf, 256, "Hello, World %zu\\n", i);
            call("printf", string_make(buf), NULL);
        }
        return_stmt(identifier_make("EXIT_SUCCESS"));
    }
    function_end();
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
