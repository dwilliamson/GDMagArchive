#include "general.h"
#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>

Lexer::Lexer() {
    input = NULL;
    input_cursor = NULL;

    token_disposal_ring_index = 0;
    int i;
    for (i = 0; i < TOKEN_DISPOSAL_RING_SIZE; i++) {
        token_disposal_ring[i] = NULL;
    }

    current_token = NULL;
    current_input_file = NULL;
}

Lexer::~Lexer() {
    int i;
    for (i = 0; i < TOKEN_DISPOSAL_RING_SIZE; i++) {
        delete token_disposal_ring[i];
    }

    delete current_token;
}

inline int Lexer::peek_next_character() {
    assert(input != NULL);
    if (input_cursor == NULL) return -1;  // Probably we got a parse error!

    if (*input_cursor == 0) {
        if (current_input_file) {
            bool success = spool_file_input();
            if (success) return peek_next_character();
        }

        return -1;
    }

    int result = *input_cursor;
    return result;
}

inline void Lexer::eat_character() {
    if (!input_cursor) return;  // Nothing to eat, eh.

    if (*input_cursor == '\n') current_line_number++;

    if (*input_cursor == 0) input_cursor = NULL;
    else input_cursor++;

    current_character_index++;
}

inline bool starts_identifier(int c) {
    if (isalpha(c)) return true;
    if (c == '_') return true;
    return false;
}

inline bool starts_integer(int c) {
    if (isdigit(c)) return true;
    if (c == '-') return true;
    return false;
}

Token *Lexer::make_token() {
    Token *result = new Token();
    result->line_number = current_line_number;
    result->character_index = current_character_index;

    return result;
}

inline void Lexer::set_num_characters(Token *token) {
    token->num_characters = current_character_index - token->character_index;
}


char *Lexer::parse_ident() {
    char *scratch_pos = scratch_buffer;
    while (1) {
        if (scratch_pos - scratch_buffer == MAX_TOKEN_LENGTH - 1) {
            fprintf(stderr, "Identifier too long!\n");  // @Robustness: Better error reporting please
            break;
        }

        int c = peek_next_character();
        if (isalnum(c) || (c == '_')) {
            eat_character();
            *scratch_pos++ = c;
            continue;
        }

        break;
    }

    *scratch_pos++ = 0;
    assert(scratch_pos - scratch_buffer <= MAX_TOKEN_LENGTH);

    char *result = copy_string(scratch_buffer);  // @Speed: We already know the length of this so we can skip the strlen in copy_string
    return result;
}

bool Lexer::parse_nonnegative_integer_helper(char *scratch_pos, int *result_ret) {
    while (1) {
        if (scratch_pos - scratch_buffer == MAX_TOKEN_LENGTH - 1) {
            fprintf(stderr, "Integer constant too long!\n");  // @Robustness: Better error reporting please
            break;
        }

        int c = peek_next_character();
        if (isdigit(c)) {
            eat_character();
            *scratch_pos++ = c;
            continue;
        }

        break;
    }

    *scratch_pos++ = 0;
    assert(scratch_pos - scratch_buffer <= MAX_TOKEN_LENGTH);

    int result = atoi(scratch_buffer);
    *result_ret = result;
    return true;
}

bool Lexer::parse_nonnegative_integer(int *result_ret) {
    return parse_nonnegative_integer_helper(scratch_buffer, result_ret);
}

bool Lexer::parse_integer(int *result_ret) {
    char *scratch_pos = scratch_buffer;

    // We already know the first character starts an integer...
    int c = peek_next_character();
    eat_character();
    *scratch_pos++ = c;

    return parse_nonnegative_integer_helper(scratch_pos, result_ret);
}


Token *Lexer::make_ident() {
    Token *result = make_token();
    result->type = TOKEN_IDENT;

    char *name = parse_ident();
    result->name = name;

    set_num_characters(result);
    return result;
}

Token *Lexer::make_variable() {
    Token *result = make_token();
    result->type = TOKEN_VARIABLE;

    char *name = parse_ident();
    result->name = name;

    set_num_characters(result);

    if (!starts_identifier(name[0])) {
        // Hmm, they started the ident with a character
        // that can only go in the middle.  That is technically
        // a parse error but we are ignoring it for now.
        // XXX
    }

    return result;
}

Token *Lexer::make_integer() {
    Token *result = make_token();
    result->type = TOKEN_INTEGER;

    int integer_value;
    bool success = parse_integer(&integer_value);
    if (!success) integer_value = 0;
    result->integer_value = integer_value;

    set_num_characters(result);

    return result;
}

Token *Lexer::make_entity_id() {
    Token *result = make_token();
    result->type = TOKEN_ENTITY_ID;

    int integer_value;
    bool success = parse_nonnegative_integer(&integer_value);
    if (!success) integer_value = 0;
    result->integer_value = integer_value;

    set_num_characters(result);

    return result;
}

Token *Lexer::make_one_character_token(int c) {
    Token *result = make_token();
    result->type = (Token_Type)c;
    result->num_characters = 1;

    eat_character();

    return result;
}

void Lexer::eat_token() {
    assert(current_token);
    int index = token_disposal_ring_index + 1;
    if (index == TOKEN_DISPOSAL_RING_SIZE) index = 0;

    delete token_disposal_ring[index];
    token_disposal_ring[index] = current_token;
    current_token = NULL;
}

Token *Lexer::peek_next_token() {
    if (current_token) return current_token;

    int c = peek_next_character();

    while (isspace(c)) {
        eat_character();
        c = peek_next_character();
    }

    if (c == -1) {
        current_token = make_one_character_token(TOKEN_END_OF_INPUT);
        return current_token;
    }

    if (starts_identifier(c)) {
        current_token = make_ident();
        return current_token;
    }

    if (starts_integer(c)) {
        current_token = make_integer();
        return current_token;
    }

    Token *result;
    switch (c) {
    case '(': 
    case ')': 
    case ',': 
    case ';': 
    default:
        result = make_one_character_token(c);
        break;
    case '<':
        eat_character();
        c = peek_next_character();
        if (c == '-') {
            result = make_one_character_token(TOKEN_IMPLIED_BY);
        } else {
            result = make_one_character_token('<');
        }
        break;
    case '?':
        eat_character();
        result = make_variable();
        break;
    case '#':
        eat_character();
        result = make_entity_id();
        break;
    case '/':
        eat_character();
        c = peek_next_character();
        if (c == '/') {
            eat_character();

            // Eat until newline or end-of-input or zero
            while (1) {
                c = peek_next_character();
                if (c == 0) break;
                if (c == '\n') break;
                if (c == -1) break;
                eat_character();
            }

            return peek_next_token();
        } else {
            result = make_one_character_token(c);
        }
        break;
    }

    current_token = result;
    return current_token;
}

void Lexer::set_input_from_string(char *s) {
    input = s;
    input_cursor = s;
    current_line_number = 1;
    current_character_index = 1;
    current_input_file = NULL;

    delete current_token;
    current_token = NULL;
}

void Lexer::set_input_from_file(void *file_ptr) {
    assert(file_ptr);

    current_input_file = file_ptr;

    delete current_token;
    current_token = NULL;

    spool_file_input();

    current_line_number = 1;
    current_character_index = 1;
}

bool Lexer::spool_file_input() {
    assert(current_input_file);

    int len_max = MAX_FILE_BUFFER_SIZE - 1;

    int length = fread(file_buffer, 1, len_max-1, (FILE *)current_input_file);
    if (length <= 0) return false;   // XXX See if it's EOF or a read error, log read error if so

    file_buffer[length] = '\0';

    input = file_buffer;
    input_cursor = input;

    return true;
}


Token::~Token() {
    switch (type) {
    case TOKEN_IDENT:
    case TOKEN_VARIABLE:
        delete [] name;
    }
}
