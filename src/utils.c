#include "utils.h"

// Reads the entire file and returns its contents as text.
// The caller must free the returned value.
char* read_text(char* path) {
  FILE* file = fopen(path, "r");
  assert(file);
  assert(fseek(file, 0, SEEK_END) == 0);
  long size = ftell(file);
  assert(size != -1);
  rewind(file);
  char* buffer = malloc(size + 1);
  assert(buffer);
  assert(fread(buffer, 1, size, file) == size);
  assert(fclose(file) == 0);
  buffer[size] = '\0';
  return buffer;
}

void __unreachable(const char *file, int line, const char *func) {
  fprintf(stderr, "Unreachable: (%s: %s: %d)\n", file, func, line);
  abort();
}
