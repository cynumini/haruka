#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "tokenizer.h"
#include "syntax.h"

int main(int argc, char* argv[])
{
    assert(argc == 2);

    char* input_path = argv[1];
    char* source_code = read_text(input_path);

    Tokens tokens = tokenize(source_code);

    get_syntax_tree(&tokens);

    free(tokens.items);
    free(source_code);
    return EXIT_SUCCESS;
}
