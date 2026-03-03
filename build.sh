#!/usr/bin/env sh

set -e

warnings="-Wall -Wpedantic -Wextra -Werror"
sakana="-Isakana/include -Lsakana/out -lsakana"
flags="$warnings $sakana -g "
files="./src/main.c ./src/haruka.c"
bin="./out/haruka"
run="$bin"

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
	./out/hello_world
fi
