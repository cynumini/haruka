from ast import parse
import sys
import pathlib


def get_level(source_line: str):
    spaces = 0
    for i in range(len(source_line)):
        c = source_line[i]
        if c == " ":
            spaces += 1
        else:
            break
    return spaces / 4


def get_tree(source_code_lines: list[str], level=0):
    print("get", source_code_lines)
    result = []
    used = 0
    need_to_skip = 0
    for i, line in enumerate(source_code_lines):
        if need_to_skip:
            need_to_skip -= 1
            used += 1
            continue
        current_level = get_level(line)
        if current_level == level:
            clear_line = line.strip()
            if not clear_line:
                used += 1
                continue
            if clear_line[-1] == ":":
                skip, children = get_tree(source_code_lines[i + 1 :], current_level + 1)
                result.append((clear_line, children))
                need_to_skip = skip
            elif clear_line:
                result.append(clear_line)
            used += 1
        else:
            break

    return (used, result)


def main():
    assert len(sys.argv) == 2
    hrk = pathlib.Path(sys.argv[1])
    source_code = hrk.read_text()

    tree = get_tree(source_code.split("\n"))

    print("=" * 80)
    print(source_code)
    print("=" * 80)
    print(tree)


if __name__ == "__main__":
    main()
