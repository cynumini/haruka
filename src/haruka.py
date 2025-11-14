from enum import Enum
import sys
import pathlib
from typing import override


def clean(source_code: str) -> str:
    source_code = source_code.replace("\n", " ")
    result = ""
    skip = True
    skip_chars = [";", ",", "{", ")", "{", "}", '"', ":"]
    for i, c in enumerate(source_code):
        next_char = source_code[i + 1] if (i + 1) < len(source_code) else None
        if c in skip_chars:
            skip = True
            result += c
        elif c == " " and (skip or next_char == " " or next_char in skip_chars):
            pass
        else:
            skip = False
            result += c
    return result


class BranchType(Enum):
    RETURN = 1
    BLOCK = 2
    OTHER = 3


Tree = list["Branch"]


class Branch:
    line: str
    branch_type: BranchType
    children: Tree

    def __init__(
        self, line: str, branch_type: BranchType = BranchType.OTHER, children: Tree = []
    ):
        self.line = line
        self.branch_type = branch_type
        self.children = children

    @override
    def __str__(self) -> str:
        return f"{self.line} | {self.branch_type} | {self.children}"


class TreeResult:
    end: int
    tree: Tree

    def __init__(self, end: int, tree: Tree):
        self.end = end
        self.tree = tree


def get_tree(source_code: str, offset: int = 0) -> TreeResult:
    root: Tree = []
    i = offset
    key: str = ""
    while i < len(source_code):
        char = source_code[i]
        if char == "{":
            sub_result = get_tree(source_code, i + 1)
            i = sub_result.end + 1
            root.append(Branch(key, BranchType.BLOCK, sub_result.tree))
            key = ""
        elif char == "}":
            break
        elif char == ";":
            root.append(Branch(key + ";"))
            key = ""
            i += 1
        else:
            key += char
            i += 1
    if key:
        branch_type = BranchType.RETURN
        if key[-1] != ";":
            branch_type.OTHER
        root.append(Branch(key + ";", BranchType.RETURN))
    return TreeResult(i, root)


def to_c(tree: Tree) -> str:
    c_code = ""
    for branch in tree:
        print(branch)
        match branch.branch_type:
            case BranchType.RETURN:
                c_code += f"return {branch.line}"
            case BranchType.BLOCK:
                c_code += branch.line + "{"
                c_code += to_c(branch.children)
                c_code += "}"
            case BranchType.OTHER:
                c_code += branch.line
    return c_code


def main():
    assert len(sys.argv) == 2
    hrk = pathlib.Path(sys.argv[1])
    source_code = hrk.read_text()
    source_code = clean(source_code)
    tree_result = get_tree(source_code)
    tree = tree_result.tree
    c_code = "typedef unsigned long u64;"
    c_code += to_c(tree)
    c = hrk.with_suffix(".c")
    assert c.write_text(c_code)


if __name__ == "__main__":
    main()
