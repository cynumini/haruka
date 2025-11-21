# from enum import Enum
import pathlib
import sys
from enum import Enum
from expression import Expression, Operand


def get_level(line: str) -> int:
    spaces = 0
    for c in line:
        if c == " ":
            spaces += 1
        else:
            break
    assert spaces % 4 == 0
    return int(spaces / 4)


class NodeType(Enum):
    ROOT = 0
    FUNCTION = 1
    RETURN = 2


class Function:
    name: str
    args: dict[str, str]
    return_type: str

    def __init__(self, line: str):
        name_start = 4
        name_end = line.find("(")
        self.name = line[name_start:name_end]
        args_start = name_end + 1
        args_end = line.find(")")
        args = [arg.split(":") for arg in line[args_start:args_end].split(",")]
        self.args = {key: value for key, value in args}
        return_type_start = line.find("->") + 2
        self.return_type = line[return_type_start:-1]

    def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
        return f"function(name={self.name}, args={self.args}, return_type={self.return_type})"


class Return:
    expression: Expression | Operand

    def __init__(self, line: str):
        start = len("return") + 1
        end = len(line)
        self.expression = Expression.from_str(line[start:end])


class Node:
    node_type: NodeType
    function: Function
    return_: Return

    children: list["Node"]
    parent: "Node | None"
    level: int

    def __init__(
        self,
        line: str | None = None,
        parent: "Node | None" = None,
        level: int = -1,
    ):
        if line is None:
            self.node_type = NodeType.ROOT
        else:
            print(line)
            if line.startswith("def "):
                self.node_type = NodeType.FUNCTION
            elif line.startswith("return "):
                self.node_type = NodeType.RETURN
            else:
                raise BaseException("not implemented yet")

            match self.node_type:
                case NodeType.FUNCTION:
                    self.function = Function(line)
                case NodeType.RETURN:
                    self.return_ = Return(line)

        self.parent = parent
        self.children = []
        self.level = level

    def append(self, line: str, level: int) -> int:
        node = Node(line, self, level)
        self.children.append(node)
        match node.node_type:
            case NodeType.FUNCTION:
                return level + 1
            case NodeType.RETURN:
                return level
            case _:
                raise BaseException("not implemented yet")

    def get(self):
        match self.node_type:
            case NodeType.ROOT:
                return None
            case NodeType.FUNCTION:
                return self.function

    def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
        result: str = ""
        if self.level != -1:
            result += "\t" * self.level + str(self.get())
        else:
            result += ""
        for child in self.children:
            if result:
                result += "\n" + str(child)
            else:
                result += str(child)
        return result


def minimize(source_code: str) -> str:
    lines: list[str] = []
    special_char = [":", ",", "(", ")", " ", ">", "="]
    for line in source_code.split("\n"):
        if line.strip() == "":
            continue
        else:
            level = get_level(line)
            new_line = " " * (4 * level)
            line = line.strip()
            for i, char in enumerate(line):
                next_char = line[i + 1] if (i + 1) < len(line) else None
                prev_char = line[i - 1] if (i - 1) >= 0 else None
                if next_char in special_char and char == " ":
                    continue
                elif prev_char in special_char and char == " ":
                    continue
                else:
                    new_line += char
            lines.append(new_line)
    return "\n".join(lines)


def get_tree(source_code: str) -> Node:
    lines = source_code.split("\n")
    tree = Node()
    root = tree
    level = 0
    i = 0
    while True:
        if i >= len(lines):
            break
        line = lines[i]
        line_level = get_level(line)
        print("line", line, level, line_level, root.parent)
        if line_level == level:
            level = root.append(line.strip(), level)
        elif line_level < level:
            if root.parent:
                root = root.parent
            else:
                root = tree
                # raise BaseException("Impossible")
            level -= 1
            continue
        i += 1
    return tree


# class Node:
#     level: int
#     line: str
#     children: list["Node"]
#     root: "Node | None"
#
#     def __init__(self, line: str, root: "Node | None" = None, level: int = -1):
#         self.line = line
#         self.root = root
#         self.level = level
#         self.children = []
#
#     @classmethod
#     def from_source_code(cls, source_code: str) -> "Node":
#         tree = Node("")
#         source_code = source_code
#         level = 0
#         root = tree
#         index = 0
#         lines = source_code.split("\n")
#         while True:
#             line = lines[index]
#             print(line)
#             break
#             # if index >= len(lines):
#             #     break
#             # line_level = get_level(line)
#             # line = line.strip()
#             # if not line:
#             #     index += 1
#             #     continue
#             # if line_level == level:
#             #     root.append(line, level)
#             #     if line[-1] == ":":
#             #         level += 1
#             #         root = root.last()
#             # else:
#             #     root = root.last().root
#             #     level -= 1
#             #     continue
#             index += 1
#         return tree
#
#     def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
#         result: str = ""
#         if self.level != -1:
#             result += "\t" * self.level + f"{self.line}"
#         if self.children:
#             for child in self.children:
#                 if result:
#                     result += f"\n{child}"
#                 else:
#                     result += f"{child}"
#         return result
#
#     def append(self, line: str, level: int):
#         self.children.append(Node(line, self, level))
#
#     def last(self):
#         return self.children[-1]


def main():
    assert len(sys.argv) == 2
    hrk = pathlib.Path(sys.argv[1])
    source_code = hrk.read_text()
    source_code = minimize(source_code)
    tree = get_tree(source_code)
    print(tree)
    # source_code = clean(source_code)
    # tree_result = get_tree(source_code)
    # tree = tree_result.tree
    # c_code = "typedef unsigned long u64;"
    # c_code += to_c(tree)
    # c = hrk.with_suffix(".c")
    # asm_code = to_asm(tree)
    # print(asm_code)
    # assert c.write_text(c_code)


if __name__ == "__main__":
    main()
