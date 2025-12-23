import argparse

from pathlib import Path
from .tokenizer import tokenize

def main():
    parser = argparse.ArgumentParser("haruka")
    _ = parser.add_argument("input", type=Path)

    args = parser.parse_args()

    assert isinstance(args.input, Path)  # pyright: ignore[reportAny]
    input_file: Path = args.input

    source_code = input_file.read_text()

    tokens = tokenize(source_code);

    print(tokens)


if __name__ == "__main__":
    main()
