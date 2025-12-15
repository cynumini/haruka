from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from haruka.c import compile_to_c
from haruka.asm import compile_to_asm
from haruka.tokenize import tokenize
from haruka.syntax_tree import get_syntax_tree
import argparse


def main():
    parser = argparse.ArgumentParser("haruka")
    _ = parser.add_argument("input", type=Path)
    _ = parser.add_argument("-b", "--backend", choices=["as", "gcc"], default="gcc")
    _ = parser.add_argument("-o", "--output", type=Path, default=Path("a.out"))
    _ = parser.add_argument("-c", nargs="?", const=True, default=False)
    _ = parser.add_argument("-s", "--asm", nargs="?", const=True, default=False)
    args = parser.parse_args()

    assert isinstance(args.input, Path)  # pyright: ignore[reportAny]
    input_path = args.input
    source_code = input_path.read_text()
    assert isinstance(args.output, Path)  # pyright: ignore[reportAny]
    output_path = args.output
    tokens = tokenize(source_code)
    syntax_tree = get_syntax_tree(tokens)

    assert type(args.c) is bool or type(args.c) is str  # pyright: ignore[reportAny]
    assert type(args.asm) is bool or type(args.asm) is str  # pyright: ignore[reportAny]

    assert type(args.backend) is str  # pyright: ignore[reportAny]
    match args.backend:
        case "gcc":
            c_code = compile_to_c(syntax_tree)
            with tempfile.NamedTemporaryFile(
                "w", suffix=".c", delete_on_close=False
            ) as fp:
                _ = fp.write(c_code)
                fp.close()
                _ = subprocess.run(["clang-format", "-i", fp.name])
                if args.c:
                    c_output_path = (
                        input_path.with_suffix(".c").name
                        if args.c and type(args.c) is not str
                        else Path(args.c)
                    )
                    _ = shutil.copy(fp.name, c_output_path)
                elif args.asm:
                    asm_output_path = (
                        input_path.with_suffix(".s").name
                        if args.asm and type(args.asm) is not str
                        else Path(args.asm)
                    )
                    _ = subprocess.run(["gcc", fp.name, "-S", "-fverbose-asm"])
                    _ = Path(Path(fp.name).with_suffix(".s").name).rename(
                        asm_output_path
                    )
                else:
                    _ = subprocess.run(
                        ["gcc", "-nostdlib", fp.name, "-o", str(output_path)]
                    )
        case "as":
            asm_code = compile_to_asm(syntax_tree)
            with tempfile.NamedTemporaryFile(
                "w", suffix=".s", delete_on_close=False
            ) as fp:
                _ = fp.write(asm_code)
                fp.close()
                if args.c:
                    raise Exception("as backend doesn't support C output")
                elif args.asm:
                    asm_output_path = (
                        input_path.with_suffix(".s").name
                        if args.asm and type(args.asm) is not str
                        else Path(args.asm)
                    )
                    _ = shutil.copy(fp.name, asm_output_path)
                else:
                    _ = subprocess.run(
                        ["gcc", "-nostdlib", fp.name, "-o", str(output_path)]
                    )
        case _:
            raise Exception(f"{args.backend} backend is not implemented!")


if __name__ == "__main__":
    main()
