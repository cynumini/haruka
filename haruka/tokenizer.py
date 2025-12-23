from enum import Enum, auto
from symtable import Symbol

class TokenType(Enum):
    LITERAL = auto()
    MACROS = auto()
    SYMBOL = auto()

class State(Enum):
    START = auto()
    MACROS = auto()
    LITERAL = auto()
    SYMBOL = auto()
    
class Token:
    token_type: TokenType
    value: str | int

    def __init__(self, token_type: TokenType, value: str | int) -> None:
        self.token_type = token_type
        self.value = value

    @staticmethod
    def from_state(state: State, value: str | int) -> "Token":
        match state:
            case State.LITERAL:
                match value:
                    case _:
                        return Token(TokenType.LITERAL, value)
            case State.MACROS:
                return Token(TokenType.MACROS, value)
            case State.SYMBOL:
                match value:
                    case _:
                        raise Exception(f'"{value}" is not covered')
            case _:
                raise Exception(f"{state} is not covered")
                
            

alphabet = "abcdefghijklmnopqrstuvwxyz"
alphabet += alphabet.upper()
symbols = "<>().,"
numbers = "0123456789"
    
class CharType(Enum):
    ALPHABET = auto()
    NUMBER_SIGN = auto()
    SPACE = auto()
    SYMBOL = auto()
    DOT = auto()
    CR = auto()
    NUMBER = auto()
    @staticmethod
    def from_str(char: str) -> "CharType":
        if char in alphabet:
            return CharType.ALPHABET
        elif char in symbols:
            return CharType.SYMBOL
        elif char in numbers:
            return CharType.NUMBER
        else:
            match char:
                case "#":
                    return CharType.NUMBER_SIGN
                case " ":
                    return CharType.SPACE
                case "\n":
                    return CharType.CR
                case _:
                    raise Exception(f'Can\'t get CharType from "{char}"')

states = {
    (State.START, CharType.NUMBER_SIGN): State.MACROS,
    (State.START, CharType.CR): State.START,
    (State.START, CharType.ALPHABET): State.LITERAL,
    (State.MACROS, CharType.ALPHABET): State.MACROS,
    (State.MACROS, CharType.SPACE): State.MACROS,
    (State.MACROS, CharType.SYMBOL): State.MACROS,
    (State.MACROS, CharType.CR): State.START,
    (State.LITERAL, CharType.ALPHABET): State.LITERAL,
    (State.LITERAL, CharType.NUMBER): State.LITERAL,
    (State.LITERAL, CharType.SPACE): State.START,
    (State.LITERAL, CharType.SYMBOL): State.SYMBOL,
    (State.SYMBOL, CharType.SYMBOL): State.SYMBOL,
    (State.SYMBOL, CharType.SPACE): State.START,
}
            
def tokenize(text: str):
    tokens: list[Token] = []
    state = State.START
    token_value = ""
    for char in text:
        char_type = CharType.from_str(char)
        new_state = states[(state, char_type)]
        match new_state:
            case State.START:
                if state != State.START:
                    tokens.append(Token.from_state(state,
                                                   token_value))
                    token_value = ""
            case State.MACROS | State.LITERAL | State.SYMBOL:
                if state != new_state and state != State.START:
                    tokens.append(Token.from_state(state,
                                                   token_value))
                    token_value = ""
                token_value += char
            case State.
            case _:
                raise Exception(f"{new_state} is not covered")
        state = new_state

    return tokens
