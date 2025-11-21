def get_next_char(line: str, i: int) -> str | None:
    return line[i + 1] if (i + 1) < len(line) else None

def get_prev_char(line: str, i: int) -> str | None:
    return line[i - 1] if (i - 1) >= 0 else None
