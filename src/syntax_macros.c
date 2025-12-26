#include "syntax.h"

MacrosResult get_macros(Tokens *tokens, size_t offset) {
    Token token = tokens->items[offset];
    return (MacrosResult) { { token.value, token.len }, offset };
}
