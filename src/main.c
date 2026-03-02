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
#include <SKN/types.h>

DEFINE_DYNAMIC_ARRAY(Strings, char *);
DYNAMIC_ARRAY_IMPL_ADD(Strings, char *, strings_add)

i32 main(void)
{
    Arena arena = arena_create(MB(1));

    i32 result = mkdir("generated", 0755);
    assert(result == 0 || errno == EEXIST);

    char input_path[PATH_MAX];
    getcwd(input_path, PATH_MAX);
    strcat(input_path, "/generated/hello_world.c");

    {

        FILE *file = fopen(input_path, "w");

        fprintf(file, "#include <stdio.h>\n");
        fprintf(file, "#include <stdlib.h>\n");
        fprintf(file, "#include <SKN/types.h>\n");
        fprintf(file, "i32 main(void)\n");
        fprintf(file, "{\n");
        fprintf(file, "    printf(\"haruka\\n\");\n");
        fprintf(file, "    return EXIT_SUCCESS;\n");
        fprintf(file, "}\n");

        fclose(file);
    }

    // compile
    {
        i32 result = mkdir("out", 0755);
        assert(result == 0 || errno == EEXIST);

        char output_path[PATH_MAX];
        getcwd(output_path, PATH_MAX);
        strcat(output_path, "/out/hello_world");

        Strings argv;
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
            execv("/bin/gcc", argv.items);
        }
        else
        {
            assert(wait(NULL) != -1);
        }
    }

    arena_destroy(&arena);
    return EXIT_SUCCESS;
}
