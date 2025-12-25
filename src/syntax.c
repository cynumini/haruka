#include "syntax.h"

enum state { s_start, s_unknown, s_len };

const int states[s_len][tt_len] = {
    { s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown, s_unknown }
};

// static enum state get_next_state(enum state state, enum token_type token_type)
// {
//     enum state next_state = states[state][token_type];
//     if (next_state == s_unknown) {
// 	printf("%i - %i\n", state, token_type);
// 	unreachable;
//     }
//     return next_state;
// }

enum element_type {et_int};

union element_value {
    int i;
};

struct element {
    enum element_type element_type;
    union element_value element_value;
};

struct elements {
    struct element * items;
    size_t capacity;
    size_t len;
};

void add_element(struct elements * elements, struct element element) {
    size_t index = elements->len;
    if (index >= elements->capacity) {
	if (elements->capacity == 0) {
	    elements->capacity = 8;
	} else {
	    elements->capacity *= 2;
	}
	elements->items = realloc(elements->items,
                                  sizeof(struct element) * elements->capacity);
    }
    elements->items[index] = element;
    elements->len++;
}

void get_syntax_tree(Tokens* tokens) {
    for(size_t i = 0; i < tokens->len; i++) {
	Token token = tokens->items[i];
	token_print(&token);
    }

    struct elements elements = { 0 };
    for (int i = 0; i < 128; i++) {
	struct element element = { 0 };
	add_element(&elements, element);
    }
}
