#!/usr/bin/env sh

gcc -Wall -Wextra -Wpedantic -g src/*.c -o out/haruka
./out/haruka examples/1.hrk
