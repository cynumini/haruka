from enum import Enum


class State(Enum):
    START = 0
    LITERAL = 1
    SKIP = 2
    SYMBOL = 3
    STRING_IN = 4
    STRING_OUT = 5
    NUMBER = 6


class CharClass(Enum):
    ALPHABET = 0
    SPACE = 1
    NUMBER = 2
    SYMBOL = 3
    AT = 4
    DOUBLE_QUOTES = 5
    UNDERSCORE = 6


state_table = {
    # START
    (State.START, CharClass.ALPHABET): State.LITERAL,
    (State.START, CharClass.SPACE): State.SKIP,
    # LITERAL
    (State.LITERAL, CharClass.ALPHABET): State.LITERAL,
    (State.LITERAL, CharClass.NUMBER): State.LITERAL,
    (State.LITERAL, CharClass.SPACE): State.SKIP,
    (State.LITERAL, CharClass.SYMBOL): State.SYMBOL,
    # SPACE
    (State.SKIP, CharClass.ALPHABET): State.LITERAL,
    (State.SKIP, CharClass.AT): State.LITERAL,
    (State.SKIP, CharClass.UNDERSCORE): State.LITERAL,
    (State.SKIP, CharClass.SPACE): State.SKIP,
    (State.SKIP, CharClass.SYMBOL): State.SYMBOL,
    (State.SKIP, CharClass.NUMBER): State.SYMBOL,
    # SYMBOL
    (State.SYMBOL, CharClass.ALPHABET): State.LITERAL,
    (State.SYMBOL, CharClass.NUMBER): State.NUMBER,
    (State.SYMBOL, CharClass.DOUBLE_QUOTES): State.STRING_IN,
    (State.SYMBOL, CharClass.SPACE): State.SKIP,
    (State.SYMBOL, CharClass.SYMBOL): State.SYMBOL,
    # STRING_IN
    (State.STRING_IN, CharClass.ALPHABET): State.STRING_IN,
    (State.STRING_IN, CharClass.DOUBLE_QUOTES): State.STRING_OUT,
    # STRING_OUT
    (State.STRING_OUT, CharClass.SYMBOL): State.SYMBOL,
    # NUMBER
    (State.NUMBER, CharClass.NUMBER): State.NUMBER,
    (State.NUMBER, CharClass.SYMBOL): State.SYMBOL,
}


class Token:
    content: str
    line: int
    column: int

    def __init__(self, content: str, line: int, column: int) -> None:
        self.content = content
        self.line = line
        self.column = column

    def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
        return f"{self.content}"


class Tokens:
    tokens: list[Token]
    children: list["Tokens"]
    parent: "Tokens | None"

    def __init__(
        self,
        tokens: list[Token],
        parent: "Tokens | None" = None,
    ) -> None:
        self.tokens = tokens
        self.parent = parent
        self.children = []

    def append(self, tokens: list[Token]) -> None:
        self.children.append(Tokens(tokens, self))

    def last(self) -> "Tokens":
        if len(self.children) == 0:
            raise BaseException("Indentation Error")
        return self.children[-1]

    def _str(self, depth: int = 0) -> str:
        result: str = f"{' ' * depth}{str([str(token) for token in self.tokens])}"
        if self.children:
            result += "\n"
        result += "\n".join([child._str(depth + 1) for child in self.children])
        return result

    def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
        return self._str()


def get_level(line: str) -> int:
    spaces = 0
    for c in line:
        if c == " ":
            spaces += 1
        else:
            break
    assert spaces % 4 == 0
    return int(spaces / 4)


def char_class_from_char(char) -> CharClass:
    alphabet = "abcdefghijklmnopqrstuvwxyz"
    if char in (alphabet + alphabet.upper()):
        return CharClass.ALPHABET
    elif char in "1234567890":
        return CharClass.NUMBER
    elif char == " ":
        return CharClass.SPACE
    elif char in "():,{}.=":
        return CharClass.SYMBOL
    elif char == "@":
        return CharClass.AT
    elif char == '"':
        return CharClass.DOUBLE_QUOTES
    elif char == "_":
        return CharClass.UNDERSCORE
    else:
        print(f'Add: "{char}"')
        raise BaseException("No implemented")


def _tokenize_line(line: str, line_number: int) -> tuple[int, list[Token]]:
    level = get_level(line)
    tokens: list[Token] = []
    state = State.START
    element = ""
    # print(line)
    for column_number, char in enumerate(line):
        next_state = state_table[(state, char_class_from_char(char))]
        # print(char, tokens, state, next_state)
        if next_state in [State.LITERAL, State.STRING_IN, State.NUMBER]:
            if state != next_state and element:
                tokens.append(Token(element, line_number, column_number))
                element = ""
            element += char
        elif next_state == State.SYMBOL:
            if element:
                tokens.append(Token(element, line_number, column_number))
                element = ""
            element += char
        elif next_state == State.SKIP:
            if element:
                tokens.append(Token(element, line_number, column_number))
                element = ""
        elif next_state == State.STRING_OUT:
            element += char
            tokens.append(Token(element, line_number, column_number))
            element = ""
        else:
            raise BaseException("No implemented")
        state = next_state
    if element:
        tokens.append(Token(element, line_number, len(line) - 1))
    # print(tokens)
    return level, tokens


def tokenize(source_code: str) -> Tokens:
    result = Tokens([])  # noqa: F811
    lines = source_code.split("\n")
    root = result
    level = 0
    for line_number, line in enumerate(lines):
        if not line.strip():
            continue
        line_level, tokens = _tokenize_line(line, line_number + 1)
        if line_level < level:
            root = root.parent if root.parent else result
        elif line_level > level:
            root = root.last()
        root.append(tokens)
        level = line_level

    return result
