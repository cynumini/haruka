#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "tokenizer.h"


int main(int argc, char* argv[]) {
  assert(argc == 2);
  
  char* input_path = argv[1];
  char * source_code = read_text(input_path);

  struct tokens tokens = tokenize(source_code);

  for(int i = 0; i < tokens.len; i++) {
    struct token token = tokens.items[i];
    token_print(&token);
  }

  free(tokens.items);
  free(source_code);
  return EXIT_SUCCESS;
}
