from enum import Enum, auto
from typing import override
from haruka.tokenize import Token, TokenType


class State(Enum):
    ASM_START = auto()
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


state_table = {
    (State.ASM_START, TokenType.VOLATILE): State.ASM_VOLATILE,
    (State.ASM_CLOBBER, TokenType.CLOSE_PARENTHESIS): State.ASM_END,
    (State.ASM_CLOBBER, TokenType.COMMA): State.ASM_CLOBBERS_START,
    (State.ASM_CLOBBERS_START, TokenType.STRING): State.ASM_CLOBBER,
    (State.ASM_IO_KEY, TokenType.OPEN_PARENTHESIS): State.ASM_IO_VALUE_START,
    (State.ASM_IO_START, TokenType.STRING): State.ASM_IO_KEY,
    (State.ASM_IO_VALUE, TokenType.CLOSE_PARENTHESIS): State.ASM_IO_VALUE_END,
    (State.ASM_IO_VALUE_END, TokenType.COLON): State.ASM_CLOBBERS_START,
    (State.ASM_IO_VALUE_END, TokenType.COMMA): State.ASM_IO_START,
    (State.ASM_IO_VALUE_START, TokenType.LITERAL): State.ASM_IO_VALUE,
    (State.ASM_OO_KEY, TokenType.OPEN_PARENTHESIS): State.ASM_OO_VALUE_START,
    (State.ASM_OO_START, TokenType.STRING): State.ASM_OO_KEY,
    (State.ASM_OO_VALUE, TokenType.CLOSE_PARENTHESIS): State.ASM_OO_VALUE_END,
    (State.ASM_OO_VALUE_END, TokenType.COLON): State.ASM_IO_START,
    (State.ASM_OO_VALUE_START, TokenType.LITERAL): State.ASM_OO_VALUE,
    (State.ASM_PARAMETERS, TokenType.STRING): State.ASM_TEMPLATE,
    (State.ASM_TEMPLATE, TokenType.COLON): State.ASM_OO_START,
    (State.ASM_VOLATILE, TokenType.OPEN_PARENTHESIS): State.ASM_PARAMETERS,
}


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

    @staticmethod
    def parse(tokens: list[Token], index: int) -> tuple[int, "Assembler"]:
        state = State.ASM_START
        assembler = Assembler()
        temp: list[Token] = []
        for i in range(index, len(tokens)):
            token = tokens[i]
            next_state = state_table[(state, token.token_type)]
            print(i, "st_assembler:", token, state, next_state)
            match next_state:
                case (
                    State.ASM_PARAMETERS
                    | State.ASM_OO_START
                    | State.ASM_OO_VALUE_START
                    | State.ASM_OO_VALUE_END
                    | State.ASM_IO_START
                    | State.ASM_IO_VALUE_START
                    | State.ASM_IO_VALUE_END
                    | State.ASM_CLOBBERS_START
                ):
                    pass
                case State.ASM_VOLATILE:
                    assembler.volatile = True
                case State.ASM_TEMPLATE:
                    assembler.template = token.value[1:-1]
                case State.ASM_OO_KEY:
                    temp.append(token)  # token[0] = OutputOperand's key
                    # assert False
                case State.ASM_OO_VALUE:
                    assert len(temp) == 1
                    assembler.output_operands[temp.pop().value[1:-1]] = token.value
                case State.ASM_IO_KEY:
                    temp.append(token)  # token[0] = OutputOperand's key
                    # assert False
                case State.ASM_IO_VALUE:
                    assert len(temp) == 1
                    assembler.input_operands[temp.pop().value[1:-1]] = token.value
                case State.ASM_CLOBBER:
                    assembler.clobbers.append(token.value)
                case State.ASM_END:
                    assert len(tokens) > (i + 1)
                    return i + 1, assembler
                case _:
                    raise Exception(f"st_asm: {next_state} is not covered!")
            state = next_state
        raise Exception("st_asm: Can't parse")
