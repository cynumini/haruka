from enum import Enum, StrEnum, auto
from typing import override
from haruka.tokenize import Token, TokenType


class State(Enum):
    START = auto()
    NUMBER = auto()
    END = auto()
    LITERAL = auto()
    STRING = auto()
    OPEN_PARENTHESIS = auto()
    CHILD_START = auto()


state_table = {
    (State.START, TokenType.LITERAL): State.LITERAL,
    (State.START, TokenType.NUMBER): State.NUMBER,
    (State.START, TokenType.OPEN_PARENTHESIS): State.CHILD_START,
    (State.START, TokenType.STRING): State.STRING,
    (State.LITERAL, TokenType.CLOSE_PARENTHESIS): State.END,
    (State.NUMBER, TokenType.CLOSE_PARENTHESIS): State.END,
    (State.NUMBER, TokenType.COMMA): State.END,
    (State.STRING, TokenType.COMMA): State.END,
}


class Operator(StrEnum):
    PLUS = "+"


Nodes = list["int | Operator | Expression | str"]


class Expression:
    nodes: Nodes

    def __init__(self) -> None:
        self.nodes = []

    @staticmethod
    def parse(tokens: list[Token], index: int) -> tuple[int, "Expression"]:
        state = State.START
        expression = Expression()

        i = index
        while i < len(tokens):
            token = tokens[i]
            next_state = state_table[(state, token.token_type)]
            print(i, "st_expression:", token, state, next_state)
            match next_state:
                case State.NUMBER:
                    expression.nodes.append(int(token.value))
                case State.END:
                    assert len(tokens) > (i + 1)
                    return i + 1, expression
                case State.CHILD_START:
                    assert len(tokens) > (i + 1)
                    i, child = Expression.parse(tokens, i + 1)
                    expression.nodes.append(child)
                    state = State.START
                    continue
                case State.LITERAL | State.STRING:
                    expression.nodes.append(token.value)
                # case State.CHILD_START:
                #     i, expression_child = Expression.parse(tokens, i + 1)
                #     expression.nodes.append(expression_child)
                #     state = State.CHILD_END
                #     continue
                case _:
                    raise Exception(f"st_expression: {next_state} is not covered!")
            state = next_state
            i += 1
        raise Exception("st_asm: Can't parse")
