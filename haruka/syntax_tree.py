from enum import Enum, auto
from typing import override
from haruka.tokenize import Token, TokenType


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
    ASM = auto()
    ASM_VOLATILE = auto()
    ASM_PARAMETERS = auto()
    ASM_TEMPLATE = auto()
    ASM_OO_START = auto()  # OO = OutputOperands
    ASM_OO_KEY = auto()
    ASM_OO_VALUE_START = auto()
    ASM_OO_VALUE = auto()
    ASM_OO_VALUE_END = auto()
    ASM_IO_START = auto()  # IO = InputOperands
    ASM_IO_KEY = auto()
    ASM_IO_VALUE_START = auto()
    ASM_IO_VALUE = auto()
    ASM_IO_VALUE_END = auto()
    ASM_CLOBBERS_START = auto()
    ASM_CLOBBER = auto()
    ASM_END = auto()
    NEXT = auto()
    RETURN = auto()
    RETURN_VALUE = auto()
    FNC = auto()
    FNC_ARG = auto()
    FNC_ARG_NEXT = auto()
    FNC_END = auto()


state_table = {
    (State.BODY, TokenType.LITERAL): State.STATE0,
    (State.BODY_END, TokenType.LITERAL): State.STATE0,
    (State.STATE0, TokenType.LITERAL): State.STATE1,
    (State.STATE0, TokenType.OPEN_PARENTHESIS): State.FNC,
    (State.STATE1, TokenType.OPEN_PARENTHESIS): State.FN,
    (State.STATE1, TokenType.SEMICOLON): State.VAR_UNDEFINED,
    (State.FN, TokenType.LITERAL): State.FN_ARG_TYPE,
    (State.FN, TokenType.CLOSE_PARENTHESIS): State.FN_ARG_END,
    (State.FN_ARG_TYPE, TokenType.LITERAL): State.FN_ARG_NAME,
    (State.FN_ARG_NAME, TokenType.COMMA): State.FN_ARG_NEXT,
    (State.FN_ARG_NAME, TokenType.CLOSE_PARENTHESIS): State.FN_ARG_END,
    (State.FN_ARG_NEXT, TokenType.LITERAL): State.FN_ARG_TYPE,
    (State.FN_ARG_END, TokenType.OPENING_BRACE): State.BODY,
    (State.VAR_UNDEFINED, TokenType.ASM): State.ASM,
    (State.ASM, TokenType.VOLATILE): State.ASM_VOLATILE,
    (State.ASM_VOLATILE, TokenType.OPEN_PARENTHESIS): State.ASM_PARAMETERS,
    (State.ASM_PARAMETERS, TokenType.STRING): State.ASM_TEMPLATE,
    (State.ASM_TEMPLATE, TokenType.COLON): State.ASM_OO_START,
    (State.ASM_OO_START, TokenType.STRING): State.ASM_OO_KEY,
    (State.ASM_OO_KEY, TokenType.OPEN_PARENTHESIS): State.ASM_OO_VALUE_START,
    (State.ASM_OO_VALUE_START, TokenType.LITERAL): State.ASM_OO_VALUE,
    (State.ASM_OO_VALUE, TokenType.CLOSE_PARENTHESIS): State.ASM_OO_VALUE_END,
    (State.ASM_OO_VALUE_END, TokenType.COLON): State.ASM_IO_START,
    (State.ASM_IO_START, TokenType.STRING): State.ASM_IO_KEY,
    (State.ASM_IO_KEY, TokenType.OPEN_PARENTHESIS): State.ASM_IO_VALUE_START,
    (State.ASM_IO_VALUE_START, TokenType.LITERAL): State.ASM_IO_VALUE,
    (State.ASM_IO_VALUE, TokenType.CLOSE_PARENTHESIS): State.ASM_IO_VALUE_END,
    (State.ASM_IO_VALUE_END, TokenType.COMMA): State.ASM_IO_START,
    (State.ASM_IO_VALUE_END, TokenType.COLON): State.ASM_CLOBBERS_START,
    (State.ASM_CLOBBERS_START, TokenType.STRING): State.ASM_CLOBBER,
    (State.ASM_CLOBBER, TokenType.COMMA): State.ASM_CLOBBERS_START,
    (State.ASM_CLOBBER, TokenType.CLOSE_PARENTHESIS): State.ASM_END,
    (State.ASM_END, TokenType.SEMICOLON): State.NEXT,
    (State.NEXT, TokenType.RETURN): State.RETURN,
    (State.NEXT, TokenType.CLOSING_BRACE): State.BODY_END,
    (State.RETURN, TokenType.LITERAL): State.RETURN_VALUE,
    (State.RETURN_VALUE, TokenType.SEMICOLON): State.NEXT,
    (State.FNC, TokenType.NUMBER): State.FNC_ARG,
    (State.FNC_ARG, TokenType.COMMA): State.FNC_ARG_NEXT,
    (State.FNC_ARG_NEXT, TokenType.NUMBER): State.FNC_ARG,
    (State.FNC_ARG, TokenType.CLOSE_PARENTHESIS): State.FNC_END,
    (State.FNC_END, TokenType.SEMICOLON): State.NEXT,
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


class FunctionCall:
    name: str
    args: list[str]

    def __init__(self, name: str):
        self.name = name
        self.args = []

    @override
    def __str__(self) -> str:
        return f"{self.name} {self.args}"


class Variable:
    variable_type: str
    name: str

    def __init__(self, variable_type: str, name: str):
        self.variable_type = variable_type
        self.name = name

    @override
    def __str__(self) -> str:
        return f"{self.name} {self.variable_type}"


class Assembler:
    volatile: bool = False
    template: str | None = None
    output_operands: dict[str, str]
    input_operands: dict[str, str]
    clobbers: list[str]

    def __init__(self):
        self.output_operands = {}
        self.input_operands = {}
        self.clobbers = []

    @override
    def __str__(self) -> str:
        return f"asm volatile={self.volatile} {self.template} {self.output_operands} {self.input_operands} {self.clobbers}"


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
    current_function_call: FunctionCall | None = None
    current_assembler: Assembler | None = None
    for token in tokens:
        next_state = state_table[(state, token.token_type)]
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
                # print(current_function)
            case (
                State.FN_ARG_NEXT
                | State.FN_ARG_END
                | State.ASM_PARAMETERS
                | State.ASM_OO_START
                | State.ASM_OO_VALUE_START
                | State.ASM_OO_VALUE_END
                | State.ASM_IO_START
                | State.ASM_IO_VALUE_START
                | State.ASM_IO_VALUE_END
                | State.ASM_CLOBBERS_START
                | State.NEXT
                | State.RETURN
                | State.FNC_ARG_NEXT
            ):
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
            case State.ASM:
                current_assembler = Assembler()
            case State.ASM_VOLATILE:
                assert current_assembler
                current_assembler.volatile = True
            case State.ASM_TEMPLATE:
                assert current_assembler
                current_assembler.template = token.value
            case State.ASM_OO_KEY:
                assert current_assembler
                temp.append(token)  # token[0] = OutputOperand's key
                # assert False
            case State.ASM_OO_VALUE:
                assert current_assembler
                assert len(temp) == 1
                current_assembler.output_operands[temp.pop().value] = token.value
            case State.ASM_IO_KEY:
                assert current_assembler
                temp.append(token)  # token[0] = OutputOperand's key
                # assert False
            case State.ASM_IO_VALUE:
                assert current_assembler
                assert len(temp) == 1
                current_assembler.input_operands[temp.pop().value] = token.value
            case State.ASM_CLOBBER:
                assert current_assembler
                current_assembler.clobbers.append(token.value)
            case State.ASM_END:
                assert current_assembler
                current_branch.append(current_assembler)
                # print(current_assembler)
                current_assembler = None
            case State.RETURN_VALUE:
                current_branch.append(Return(token.value))
                pass
            case State.FNC:
                assert len(temp) == 1
                current_function_call = FunctionCall(temp.pop().value)
            case State.FNC_ARG:
                assert current_function_call
                current_function_call.args.append(token.value)
            case State.FNC_END:
                assert current_function_call
                current_branch.append(current_function_call)
            case _:
                raise Exception(f"{next_state} is not covered!")
        state = next_state

    return syntax_tree
