#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char* read_text(char* path);
void __unreachable(const char *file, int line, const char *func);

#define unreachable __unreachable(__FILE__, __LINE__, __func__)

#endif
