import pathlib
import sys

from haruka.tokenizer import tokenize


def main():
    assert len(sys.argv) == 2
    source_code = pathlib.Path(sys.argv[1]).read_text()
    tokens = tokenize(source_code)
    print(source_code)
    print(tokens)


if __name__ == "__main__":
    main()
