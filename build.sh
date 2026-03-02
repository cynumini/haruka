#!/usr/bin/env sh

set -e

flags="-Wall -Wpedantic -Wextra -Werror -g -Isakana/include -Lsakana/out -lsakana"
files="./src/main.c ./src/lexer.c ./src/parser.c"
bin="./out/haruka"
run="$bin ./examples/1.hrk ./examples/1.c"

make -C sakana
mkdir -p out
echo $flags | tr ' ' "\n" > ./compile_flags.txt
echo "#### build ####"
gcc $files -o $bin $flags
if [ "$1" = "gf2" ]; then
	echo "#### debug haruka ####"
	gf2 --args $run
else
	echo "#### run haruka ####"
	$run
	echo "#### build and run ####"
	gcc ./examples/1.c -o ./out/1
	./out/1
fi
