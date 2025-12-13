import pathlib
import sys
from haruka.c import compile_to_c
from haruka.tokenize import tokenize
from haruka.syntax_tree import get_syntax_tree


def main():
    assert len(sys.argv) == 3
    source_code = pathlib.Path(sys.argv[1]).read_text()
    output_path = pathlib.Path(sys.argv[2])
    # print(source_code)
    tokens = tokenize(source_code)
    syntax_tree = get_syntax_tree(tokens)
    c_code = compile_to_c(syntax_tree)
    _ = output_path.write_text(c_code)


if __name__ == "__main__":
    main()
