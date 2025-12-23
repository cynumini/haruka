from enum import Enum, auto
from typing import override
from .assembler import Assembler
from .function_call import FunctionCall
from .expression import Expression
from haruka.tokenize import Token, TokenType

__all__ = ["FunctionCall", "Expression", "Assembler"]


class State(Enum):
    BODY = auto()
    BODY_END = auto()
    STATE0 = auto()  # One literal
    STATE1 = auto()  # Two literals
    FN = auto()  # Two literal + OPEN_PARENTHESIS = FUNCTION ?
    FN_ARG_TYPE = auto()
    FN_ARG_NAME = auto()
    FN_ARG_NEXT = auto()
    FN_ARG_END = auto()
    FN_BODY = auto()
    VAR_UNDEFINED = auto()
    ASM_START = auto()
    ASM_END = auto()
    NEXT = auto()
    RETURN = auto()
    RETURN_VALUE = auto()
    FNC_START = auto()
    FNC_END = auto()


state_table = {
    (State.ASM_END, TokenType.SEMICOLON): State.NEXT,
    (State.BODY, TokenType.LITERAL): State.STATE0,
    (State.BODY_END, TokenType.LITERAL): State.STATE0,
    (State.FN, TokenType.CLOSE_PARENTHESIS): State.FN_ARG_END,
    (State.FN, TokenType.LITERAL): State.FN_ARG_TYPE,
    (State.FNC_END, TokenType.SEMICOLON): State.NEXT,
    (State.FN_ARG_END, TokenType.OPENING_BRACE): State.BODY,
    (State.FN_ARG_NAME, TokenType.CLOSE_PARENTHESIS): State.FN_ARG_END,
    (State.FN_ARG_NAME, TokenType.COMMA): State.FN_ARG_NEXT,
    (State.FN_ARG_NEXT, TokenType.LITERAL): State.FN_ARG_TYPE,
    (State.FN_ARG_TYPE, TokenType.LITERAL): State.FN_ARG_NAME,
    (State.NEXT, TokenType.CLOSING_BRACE): State.BODY_END,
    (State.NEXT, TokenType.RETURN): State.RETURN,
    (State.NEXT, TokenType.LITERAL): State.STATE0,
    (State.RETURN, TokenType.LITERAL): State.RETURN_VALUE,
    (State.RETURN_VALUE, TokenType.SEMICOLON): State.NEXT,
    (State.STATE0, TokenType.LITERAL): State.STATE1,
    (State.STATE0, TokenType.OPEN_PARENTHESIS): State.FNC_START,
    (State.STATE1, TokenType.OPEN_PARENTHESIS): State.FN,
    (State.STATE1, TokenType.SEMICOLON): State.VAR_UNDEFINED,
    (State.VAR_UNDEFINED, TokenType.ASM): State.ASM_START,
}


class Function:
    return_type: str
    name: str
    args: dict[str, str]
    body: "Elements"

    def __init__(self, return_type: str, name: str):
        self.return_type = return_type
        self.name = name
        self.args = {}
        self.body = []

    @override
    def __str__(self) -> str:
        result = f"{self.return_type} {self.name} ({self.args}) {{\n"
        for element in self.body:
            result += "\t" + str(element) + "\n"
        return result


class Variable:
    variable_type: str
    name: str

    def __init__(self, variable_type: str, name: str):
        self.variable_type = variable_type
        self.name = name

    @override
    def __str__(self) -> str:
        return f"{self.name} {self.variable_type}"


class Return:
    value: str

    def __init__(self, value: str):
        self.value = value

    @override
    def __str__(self) -> str:
        return f"return {self.value}"


Element = Variable | Function | Assembler | Return | FunctionCall
Elements = list[Element]


def get_syntax_tree(tokens: list[Token]) -> Elements:
    syntax_tree: Elements = []
    current_branch = syntax_tree
    branch_stack: list[Elements] = []
    state = State.BODY
    temp: list[Token] = []
    current_function: Function | None = None
    i = 0
    while i < len(tokens):
        token = tokens[i]
        next_state = state_table[(state, token.token_type)]
        print(i, "st:", token, state, next_state)
        match next_state:
            case State.STATE0 | State.STATE1:
                temp.append(token)
            case State.FN:
                # temp[0] is return type
                # temp[1] is name
                assert len(temp) == 2
                current_function = Function(temp[0].value, temp[1].value)
                temp.clear()
            case State.FN_ARG_TYPE:
                temp.append(token)
            case State.FN_ARG_NAME:
                # temp[0] is argument type
                assert current_function
                assert len(temp) == 1
                current_function.args[token.value] = temp.pop().value
            case State.FN_ARG_NEXT | State.FN_ARG_END | State.NEXT | State.RETURN:
                pass
            case State.BODY:
                branch_stack.append(current_branch)
                assert current_function
                current_branch = current_function.body
            case State.BODY_END:
                current_branch = branch_stack.pop()
                assert current_function
                current_branch.append(current_function)
                current_function = None
            case State.VAR_UNDEFINED:
                # temp[0] is type
                # temp[1] is name
                assert len(temp) == 2
                current_branch.append(Variable(temp[0].value, temp[1].value))
                temp.clear()
            case State.RETURN_VALUE:
                current_branch.append(Return(token.value))
                pass
            case State.FNC_START:
                assert len(temp) == 1
                assert len(tokens) > (i + 1)
                i, function_call = FunctionCall.parse(tokens, i + 1, temp.pop().value)
                current_branch.append(function_call)
                state = state.FNC_END
                continue
            case State.ASM_START:
                assert len(tokens) > (i + 1)
                i, assembler = Assembler.parse(tokens, i + 1)
                current_branch.append(assembler)
                state = state.ASM_END
                continue
            case _:
                raise Exception(f"{next_state} is not covered!")
        state = next_state
        i += 1

    return syntax_tree
