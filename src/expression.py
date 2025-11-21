from ast import Raise
from csv import QUOTE_ALL
from enum import Enum
from token import STRING
from webbrowser import Opera

# from parsing import get_prev_char, get_next_char  # pyright: ignore[reportImplicitRelativeImport]


alphabet = "abcdefghijklmnopqrstuvwxyz"
literal_chars = alphabet + alphabet.upper()


class State(Enum):
    START = 0
    UNARY = 1
    BINARY = 2
    COMPARISON = 3
    LITERAL = 4
    INT = 5
    EXPRESSION = 6
    FLOAT = 7
    STRING_IN = 8
    STRING_OUT = 9
    ENUM_OR_STRUCT = 10


class CharClass(Enum):
    UNARY = 0
    BINARY = 1
    ALPHABET = 2
    NUMBERS = 3
    COMPARISON = 4
    DOT = 5
    AT = 6
    QUOTE = 7
    OPENING_BRACE = 8
    CLOSING_BRACE = 9


state_table = {
    # State.START
    (State.START, CharClass.UNARY): State.UNARY,
    (State.START, CharClass.NUMBERS): State.INT,
    (State.START, CharClass.ALPHABET): State.LITERAL,
    (State.START, CharClass.AT): State.LITERAL,
    (State.START, CharClass.QUOTE): State.STRING_IN,
    (State.START, CharClass.DOT): State.ENUM_OR_STRUCT,
    # State.UNARY
    (State.UNARY, CharClass.ALPHABET): State.LITERAL,
    (State.UNARY, CharClass.AT): State.LITERAL,
    (State.UNARY, CharClass.NUMBERS): State.INT,
    # State.LITERAL
    (State.LITERAL, CharClass.ALPHABET): State.LITERAL,
    (State.LITERAL, CharClass.BINARY): State.BINARY,
    (State.LITERAL, CharClass.NUMBERS): State.LITERAL,
    # State.BINARY
    (State.BINARY, CharClass.NUMBERS): State.INT,
    (State.BINARY, CharClass.ALPHABET): State.LITERAL,
    # State.NUMBER
    (State.INT, CharClass.BINARY): State.BINARY,
    (State.INT, CharClass.UNARY): State.BINARY,
    (State.INT, CharClass.NUMBERS): State.INT,
    (State.INT, CharClass.DOT): State.FLOAT,
    # State.EXPRESSION
    (State.EXPRESSION, CharClass.UNARY): State.BINARY,
    (State.EXPRESSION, CharClass.BINARY): State.BINARY,
    (State.EXPRESSION, CharClass.COMPARISON): State.COMPARISON,
    # State.COMPARISON
    (State.COMPARISON, CharClass.COMPARISON): State.COMPARISON,
    (State.COMPARISON, CharClass.NUMBERS): State.INT,
    # State.FLOAT
    (State.FLOAT, CharClass.NUMBERS): State.FLOAT,
    (State.FLOAT, CharClass.UNARY): State.BINARY,
    # State.STRING_IN
    (State.STRING_IN, CharClass.ALPHABET): State.STRING_IN,
    (State.STRING_IN, CharClass.QUOTE): State.STRING_OUT,
}


def get_next_state(state: State, char: str) -> State:
    char_class: CharClass = None  # pyright: ignore[reportAssignmentType]
    if char in "-+":
        char_class = CharClass.UNARY
    elif char in literal_chars:
        char_class = CharClass.ALPHABET
    elif char in "/*":
        char_class = CharClass.BINARY
    elif char in "0123456789":
        char_class = CharClass.NUMBERS
    elif char in "><=":
        char_class = CharClass.COMPARISON
    elif char == ".":
        char_class = CharClass.DOT
    elif char == "@":
        char_class = CharClass.AT
    elif char == '"':
        char_class = CharClass.QUOTE
    elif char == "{":
        char_class = CharClass.OPENING_BRACE
    elif char == "}":
        char_class = CharClass.CLOSING_BRACE
    else:
        print(char)
        raise BaseException("not implemented yet")

    return state_table[(state, char_class)]


class Operator(Enum):
    UNARY_MINUS = 0
    SUM = 1
    SUBTRACTION = 2
    MULTIPLICATION = 3

    def get_priority(self):
        if self == Operator.UNARY_MINUS:
            return 0
        elif self == Operator.MULTIPLICATION:
            return 1
        elif self in [Operator.SUM, Operator.SUBTRACTION]:
            return 2
        else:
            raise BaseException("not implemented yet" + self)

    def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
        match self:
            case Operator.UNARY_MINUS:
                return "-"
            case Operator.SUBTRACTION:
                return "-"
            case Operator.SUM:
                return "+"
            case Operator.MULTIPLICATION:
                return "*"


class OperandType(Enum):
    INT = 0
    FLOAT = 1
    LITERAL = 2
    FUNCTION = 3
    STRING = 3


class FunctionCall:
    name: str
    args: list["Expression | Operand"]

    def __init__(self, line: str) -> None:
        name_start = 0
        name_end = line.find("(")
        self.name = line[name_start:name_end]
        args_start = name_end + 1
        args_end = len(line) - 1
        args_str = line[args_start:args_end]
        args = []
        arg = ""
        level = 0
        for c in args_str:
            if c == "," and level == 0:
                args.append(arg)
                arg = ""
            else:
                if c == "{":
                    level += 1
                elif c == "}":
                    level -= 1
                arg += c
        args.append(arg)
        self.args = [Expression.from_str(arg) for arg in args]
        print(self.name)
        print(self.args)

    def __str__(self) -> str:
        result = f"({self.name} "
        result += " ".join([str(arg) for arg in self.args])
        return result + ")"


class Struct:
    data: dict[str, "Expression | Operand"]

    def __init__(self, line: str):
        args = [arg.split("=") for arg in line[2:-1].split(",")]
        self.data = {key: Expression.from_str(value) for key, value in args}


class EnumValue:
    value: str

    def __init__(self, line: str):
        self.value = line[1:]


class Operand:
    operand_type: OperandType
    int_value: int
    float_value: float
    literal_name: str
    function_call: FunctionCall
    string_value: str

    def __init__(self, line: str, operand_type: OperandType):
        self.operand_type = operand_type
        match self.operand_type:
            case OperandType.INT:
                self.int_value = int(line)
            case OperandType.FLOAT:
                self.float_value = float(line)
            case OperandType.LITERAL:
                self.literal_name = line
            case OperandType.FUNCTION:
                self.function_call = FunctionCall(line)
            case OperandType.STRING:
                self.string_value = line

    @staticmethod
    def from_state(line: str, state: State) -> "Operand":
        match state:
            case State.FLOAT:
                return Operand(line, OperandType.FLOAT)
            case State.INT:
                return Operand(line, OperandType.INT)
            case State.LITERAL:
                return Operand(line, OperandType.LITERAL)

    def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
        match self.operand_type:
            case OperandType.INT:
                return str(self.int_value)
            case OperandType.FLOAT:
                return str(self.float_value)
            case OperandType.LITERAL:
                return self.literal_name
            case OperandType.FUNCTION:
                return str(self.function_call)
            case OperandType.STRING:
                return self.string_value


def get_operator_type(line: str, unary: bool = False) -> Operator:
    if unary:
        if line == "-":
            return Operator.UNARY_MINUS
        else:
            raise BaseException("not implemented yet")
    else:
        if line == "+":
            return Operator.SUM
        if line == "-":
            return Operator.SUBTRACTION
        elif line == "*":
            return Operator.MULTIPLICATION
        else:
            print(line, unary)
            raise BaseException("not implemented yet")


ParseResult = list["Operator | Operand | ParseResult"]


def parse(line: str) -> ParseResult:
    result: ParseResult = []
    element = ""
    state = State.START
    level = 0
    for i, char in enumerate(line):
        if char == "(":
            if state == State.BINARY:
                result.append(get_operator_type(element))
                element = ""
            elif state == State.LITERAL:
                pass
            else:
                print(state)
                raise BaseException("not implemented yet")
            level += 1
        elif char == ")":
            level -= 1
            if level == 0:
                element += char
                if element[0] == "(":
                    result.append(parse(element[1:-1]))
                else:
                    result.append(Operand(element, OperandType.FUNCTION))
                element = ""
                state = State.EXPRESSION
                continue
        if level == 0:
            next_state = get_next_state(state, char)
            print(i, char, result, state, next_state)
            if next_state == State.UNARY:
                result.append(get_operator_type(char, True))
            elif next_state in [
                State.LITERAL,
                State.INT,
                State.STRING_IN,
                State.ENUM_OR_STRUCT,
            ]:
                if state == State.COMPARISON or state == State.BINARY:
                    result.append(get_operator_type(element))
                    element = ""
                element += char
            elif next_state == State.BINARY:
                if state in [State.INT, State.FLOAT, State.LITERAL]:
                    result.append(Operand.from_state(element, state))
                element = ""
                element += char
            elif next_state == State.COMPARISON or next_state == State.FLOAT:
                element += char
            elif next_state == State.STRING_OUT:
                result.append(Operand(element[1:], OperandType.STRING))
                element = ""
            else:
                raise BaseException("not implemented yet")
            state = next_state
        else:
            element += char
    if element:
        if state in [State.INT, State.FLOAT, State.LITERAL]:
            result.append(Operand.from_state(element, state))
    return result


def expression_from_parse_result(parse_result: ParseResult) -> "Expression | Operand":
    if len(parse_result) == 1:
        if type(parse_result[0]) is list:
            return expression_from_parse_result(parse_result[0])
        else:
            return parse_result[0]
    max_priority = -1
    index = -1
    for i, element in enumerate(parse_result):
        if type(element) is Operator:
            priority = element.get_priority()
            if priority > max_priority:
                max_priority = priority
                index = i
    if index == 0:
        return Expression(
            parse_result[index],
            [expression_from_parse_result(parse_result[index + 1 :])],
        )
    return Expression(
        parse_result[index],
        [
            expression_from_parse_result(parse_result[:index]),
            expression_from_parse_result(parse_result[index + 1 :]),
        ],
    )


class Expression:
    operator: Operator
    operands: list["Operand | Expression"]

    def __init__(
        self, operator: Operator, operands: list["Operand | Expression"]
    ) -> None:
        self.operator = operator
        self.operands = operands

    @staticmethod
    def from_str(line: str) -> "Expression | Operand":
        return expression_from_parse_result(parse(line))

    def __str__(self) -> str:  # pyright: ignore[reportImplicitOverride]
        result = f"({self.operator} "
        result += " ".join([str(operand) for operand in self.operands])
        return result + ")"


if __name__ == "__main__":
    expression = Expression.from_str("main(10,20)*10")
    print(expression)
