from enum import Enum, auto
from typing import override
from haruka.syntax_tree.expression import Expression
from haruka.tokenize import Token, TokenType


class State(Enum):
    FNC_START = auto()
    EXP_START = auto()
    EXP_END = auto()
    FNC_END = auto()


state_table = {
    (State.FNC_START, TokenType.NUMBER): State.EXP_START,
    (State.EXP_END, TokenType.NUMBER): State.EXP_START,
    (State.EXP_END, TokenType.OPEN_PARENTHESIS): State.EXP_START,
    (State.EXP_END, TokenType.LITERAL): State.EXP_START,
    (State.EXP_END, TokenType.SEMICOLON): State.FNC_END,
}


class FunctionCall:
    name: str
    args: list[Expression]

    def __init__(self, name: str):
        self.name = name
        self.args = []

    @override
    def __str__(self) -> str:
        return f"{self.name} {self.args}"

    @staticmethod
    def parse(tokens: list[Token], index: int, name: str) -> tuple[int, "FunctionCall"]:
        state = State.FNC_START
        function_call = FunctionCall(name)

        i = index
        while i < len(tokens):
            token = tokens[i]
            next_state = state_table[(state, token.token_type)]
            print(i, "st_function_call:", token, state, next_state)
            match next_state:
                case State.EXP_START:
                    i, expression = Expression.parse(tokens, i)
                    function_call.args.append(expression)
                    state = State.EXP_END
                    continue
                case State.FNC_END:
                    return i, function_call
                case _:
                    raise Exception(f"st_function_call: {next_state} is not covered!")
            state = next_state
            i += 1
