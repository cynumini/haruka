#include "syntax.h"

ReturnResult get_return(Tokens *tokens, size_t offset) {
    Token token = tokens->items[offset + 1];
    Return return_element = { { token.value, token.len } };
    return (ReturnResult){ return_element, offset + 2 };
}
