#!/usr/bin/env sh

gcc -Wall -Wextra -Werror -Wpedantic src/*.c -o out/haruka
./out/haruka examples/1.hrk
