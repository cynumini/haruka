from enum import Enum, auto
from typing import override


class State(Enum):
    END = auto()
    LITERAL = auto()
    SYMBOL = auto()
    STRING = auto()
    NUMBER = auto()


class CharType(Enum):
    ALPHABET = auto()
    NUMBER = auto()
    SPACE = auto()
    SYMBOL = auto()
    DOUBLE_QUOTES = auto()

    @staticmethod
    def get_char_type(char: str):
        alphabet = "abcdefghijklmnopqrstuvwxyz"
        alphabet += alphabet.upper()
        alphabet += "_"  #
        numbers = "0123456789"
        symbols = "(){},;:="
        spaces = " \n"
        if char in alphabet:
            return CharType.ALPHABET
        elif char in numbers:
            return CharType.NUMBER
        elif char in spaces:
            return CharType.SPACE
        elif char in symbols:
            return CharType.SYMBOL
        elif char == '"':
            return CharType.DOUBLE_QUOTES
        else:
            raise Exception(f"Make a new char type for '{char}'!")


state_table = {
    (State.END, CharType.ALPHABET): State.LITERAL,
    (State.END, CharType.DOUBLE_QUOTES): State.STRING,
    (State.END, CharType.SPACE): State.END,
    (State.END, CharType.SYMBOL): State.SYMBOL,
    (State.END, CharType.NUMBER): State.NUMBER,
    (State.LITERAL, CharType.ALPHABET): State.LITERAL,
    (State.LITERAL, CharType.NUMBER): State.LITERAL,
    (State.LITERAL, CharType.SPACE): State.END,
    (State.LITERAL, CharType.SYMBOL): State.SYMBOL,
    (State.NUMBER, CharType.NUMBER): State.NUMBER,
    (State.NUMBER, CharType.SYMBOL): State.SYMBOL,
    (State.STRING, CharType.ALPHABET): State.STRING,
    (State.STRING, CharType.DOUBLE_QUOTES): State.END,
    (State.STRING, CharType.NUMBER): State.STRING,
    (State.STRING, CharType.SYMBOL): State.STRING,
    (State.SYMBOL, CharType.ALPHABET): State.LITERAL,
    (State.SYMBOL, CharType.DOUBLE_QUOTES): State.STRING,
    (State.SYMBOL, CharType.NUMBER): State.NUMBER,
    (State.SYMBOL, CharType.SPACE): State.END,
    (State.SYMBOL, CharType.SYMBOL): State.SYMBOL,
}


class TokenType(Enum):
    LITERAL = auto()
    STRING = auto()
    NUMBER = auto()
    SYMBOL = auto()
    OPEN_PARENTHESIS = auto()
    CLOSE_PARENTHESIS = auto()
    OPENING_BRACE = auto()
    CLOSING_BRACE = auto()
    COMMA = auto()
    SEMICOLON = auto()
    ASM = auto()
    VOLATILE = auto()
    COLON = auto()
    RETURN = auto()


class Token:
    value: str
    token_type: TokenType

    def __init__(self, value: str, state: State):
        self.value = value
        match state:
            case State.LITERAL:
                match value:
                    case "asm":
                        self.token_type = TokenType.ASM
                    case "volatile":
                        self.token_type = TokenType.VOLATILE
                    case "return":
                        self.token_type = TokenType.RETURN
                    case _:
                        self.token_type = TokenType.LITERAL
            case State.STRING:
                self.token_type = TokenType.STRING
            case State.NUMBER:
                self.token_type = TokenType.NUMBER
            case State.SYMBOL:
                match value:
                    case "(":
                        self.token_type = TokenType.OPEN_PARENTHESIS
                    case ")":
                        self.token_type = TokenType.CLOSE_PARENTHESIS
                    case "{":
                        self.token_type = TokenType.OPENING_BRACE
                    case "}":
                        self.token_type = TokenType.CLOSING_BRACE
                    case ",":
                        self.token_type = TokenType.COMMA
                    case ";":
                        self.token_type = TokenType.SEMICOLON
                    case ":":
                        self.token_type = TokenType.COLON
                    case _:
                        self.token_type = TokenType.SYMBOL
            case _:
                raise Exception(f"Can't convert {state} to TokenType")

    @override
    def __str__(self) -> str:
        return self.value


def tokenize(text: str):
    tokens: list[Token] = []
    current_token = ""
    state = State.END
    for char in text:
        next_state = state_table[(state, CharType.get_char_type(char))]
        match next_state:
            case State.LITERAL | State.STRING | State.NUMBER:
                if state == State.SYMBOL and current_token:
                    tokens.append(Token(current_token, state))
                    current_token = ""
                current_token += char
            case State.END:
                if current_token:
                    if char not in " \n":
                        current_token += char
                    tokens.append(Token(current_token, state))
                    current_token = ""
            case State.SYMBOL:
                if current_token:
                    tokens.append(Token(current_token, state))
                    current_token = ""
                current_token += char
        state = next_state
    return tokens
