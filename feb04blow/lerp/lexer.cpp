#include "general.h"
#include "lexer.h"
#include "interp.h"
#include "parser.h"  // For make_atom

#include "concatenator.h"
#include "unicode.h"

#include <ctype.h>
#include <stdlib.h>

Lexer::Lexer(Lerp_Interp *_interp) {
    interp = _interp;
    input = NULL;
    input_cursor = NULL;

    token_disposal_ring_index = 0;
    int i;
    for (i = 0; i < TOKEN_DISPOSAL_RING_SIZE; i++) {
        token_disposal_ring[i] = NULL;
    }

    current_token = NULL;
    lookahead_token = NULL;
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
            bool success = spool_file_input(true, *(input_cursor - 1));
            if (success) return peek_next_character();
        }

        return -1;
    }

    int result = *input_cursor;
    return result;
}

inline void Lexer::eat_character() {
    if (*input_cursor == '\n') {
        current_line_number++;
        current_character_index = 0;  // Will get incremented to 1 below.
    }

    if (*input_cursor == 0) input_cursor = NULL;
    else input_cursor++;

    current_character_index++;
}

void Lexer::unwind_one_character() {
    assert(input_cursor != NULL);

    if (current_input_file) {
        assert(input_cursor > file_buffer);
    } else {
        assert(input_cursor > input);
    }

    input_cursor--;
    current_character_index--;
}


inline bool starts_identifier(int c) {
    if (isalpha(c)) return true;
    if (c == '_') return true;
    return false;
}

inline bool starts_number(int c) {
    if (isdigit(c)) return true;
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
            // XXX Report an error here about ident too long
            // and maybe just eat the rest of the ident 
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

bool Lexer::parse_nonnegative_integer(int *result_ret) {
    char *buffer = scratch_buffer;
    char *cursor = buffer;
    int buffer_len = MAX_TOKEN_LENGTH;

    while (cursor - buffer < buffer_len - 1) {
        int c = peek_next_character();
        if (!isdigit(c)) break;

        eat_character();

        *cursor++ = c;
    }

    if (cursor == buffer) return false;

    *cursor = '\0';

    *result_ret = atoi(buffer);
    return true;
}

Token_Type check_for_keyword(Atom *atom) {
    Text_Utf8 *name = atom->name;
    
    // @Speed: Instead of doing all these dumb strcmps against the atom name,
    // we could just have a bunch of pre-looked-up atoms: atom_if, atom_then, etc...
    // then we just do a pointer compare and we know which keyword it is.

    if (Unicode::strings_match(name, "if")) return TOKEN_KEYWORD_IF;
    if (Unicode::strings_match(name, "then")) return TOKEN_KEYWORD_THEN;
    if (Unicode::strings_match(name, "else")) return TOKEN_KEYWORD_ELSE;
    if (Unicode::strings_match(name, "proc")) return TOKEN_KEYWORD_PROC;
    if (Unicode::strings_match(name, "each")) return TOKEN_KEYWORD_EACH;
    if (Unicode::strings_match(name, "sort")) return TOKEN_KEYWORD_SORT;
    if (Unicode::strings_match(name, "return")) return TOKEN_KEYWORD_RETURN;
    if (Unicode::strings_match(name, "struct")) return TOKEN_KEYWORD_STRUCT;
    if (Unicode::strings_match(name, "while")) return TOKEN_KEYWORD_WHILE;
    if (Unicode::strings_match(name, "break")) return TOKEN_KEYWORD_BREAK;
    if (Unicode::strings_match(name, "using")) return TOKEN_KEYWORD_USING;
    return (Token_Type)0;
}

Token *Lexer::make_ident_or_keyword() {
    Token *result = make_token();
    result->type = TOKEN_IDENT;

    char *name = parse_ident();
    result->name = interp->parser->make_atom(name);
    delete [] name;

    set_num_characters(result);

    Token_Type keyword_type = check_for_keyword(result->name);
    if (keyword_type) result->type = keyword_type;

    return result;
}

Token *Lexer::make_query_variable() {
    Token *result = make_token();
    result->type = TOKEN_QUERY_VARIABLE;

    char *name = parse_ident();
    result->name = interp->parser->make_atom(name);
    delete [] name;

    set_num_characters(result);

    if (!starts_identifier(name[0])) {
        // Hmm, they started the ident with a character
        // that can only go in the middle.  That is technically
        // a parse error but we are ignoring it for now.
        // XXX
    }

    return result;
}

Token *Lexer::make_integer_or_float(int preload_character) {
    Token *result = make_token();

    bool found_decimal = false;

    char *buffer = scratch_buffer;
    char *cursor = buffer;
    int buffer_len = MAX_TOKEN_LENGTH;

    if (preload_character != -1) *cursor++ = preload_character;

    while (cursor - buffer < buffer_len - 1) {
        int c = peek_next_character();

        if (c == '.') {
            if (found_decimal) {
                interp->report_error((Token *)NULL, "Can't have two decimals in a number!\n");
                break;
            }

            eat_character();

            c = peek_next_character();
            if (c == '.') {
                // Uhh.. this isn't a decimal, it's part of an integer range
                // specifier, e.g.:  (1..3)
                // so we are going to return an integer here and push
                // a '..' token into the token queue.

                unwind_one_character();
                break;
            } else {
                *cursor++ = '.';
                found_decimal = true;
                continue;
            }
        } else if (buffer == cursor) {
            // We already know the first character is valid or we would not have gotten here...
            // so we just accept it no matter what (it could be + or - or something).
            // To accept it, we just do nothing here (to prevent us from breaking via the else below).
        } else if (!isdigit(c)) break;

        eat_character();
        *cursor++ = c;
    }

    *cursor = '\0';

    if (found_decimal) {
        result->type = TOKEN_FLOAT;
        result->float_value = atof(buffer);
    } else {
        result->type = TOKEN_INTEGER;
        result->integer_value = atoi(buffer);
    }

    set_num_characters(result);

    return result;
}

/*
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
*/

Token *Lexer::make_string_constant() {
    Concatenator buffer;
    while (1) {
        int c = peek_next_character();
        eat_character();

        if (c == '"') {
            break;
        }

        if (c == '\n') {
            interp->report_error((Token *)NULL, "Newline in string constant!\n");
            break;
        }

        if (c == '\\') {
            // @Robustness: Barf if they put a backslash in a weird place....

            c = peek_next_character();
            if (c == 'n') {
                c = '\n';
                eat_character();
            } else if (c == '\\') {
                c = '\\';
            }
        }

        buffer.add((char)c);
    }

    char *data;
    unsigned int length;

    data = buffer.get_nozeroterm_result(&length);

    Token *result = make_token();
    result->type = TOKEN_STRING;
    char *value = new char[length + 1];
    memcpy(value, data, length);
    value[length] = '\0';

    result->string_value = value;

    return result;
}

Token *Lexer::make_implicit_variable() {
    Token *result = make_token();
    result->type = TOKEN_IMPERATIVE_IMPLICIT_VARIABLE;

    int integer_value;
    int c = peek_next_character();
    if (c == '_') {
        eat_character();
        integer_value = 0;
    } else {
        bool success = parse_nonnegative_integer(&integer_value);
        if (!success) integer_value = 0;
    }

    result->integer_value = integer_value;

    set_num_characters(result);

    return result;
}

Token *Lexer::make_one_character_token(int c) {
    Token *result = make_token();
    result->type = (Token_Type)c;
    result->num_characters = 1;

    return result;
}

void Lexer::eat_token() {
    assert(current_token);
    int index = token_disposal_ring_index + 1;
    if (index == TOKEN_DISPOSAL_RING_SIZE) index = 0;

    delete token_disposal_ring[index];
    token_disposal_ring[index] = current_token;

    current_token = lookahead_token;
    lookahead_token = NULL;
}

Token *Lexer::check_for_equals(int c, int augmented) {
    Token *result;
    eat_character();

    int next = peek_next_character();
    if (next == '=') {
        eat_character();
        result = make_one_character_token(augmented);
    } else {
        result = make_one_character_token(c);
    }

    return result;
}

//
// @Robustness: We might want to modify eat_input_due_to_block_comment so that
// characters in front of comment items need to be clear... e.g. right now it considers
// the string "/////*" to be a nested open-comment... not sure if we want that.
//
void Lexer::eat_input_due_to_block_comment() {
    int comment_depth = 1;

    while (comment_depth) {
        int c = peek_next_character();

        if (c == -1) { // End of input!
            interp->report_error((Token *)NULL, "End of input from within comment at line XXX char XXX\n");  // XXX Do a real error message here...
            return;
        }

        if (c == '/') {
            eat_character();
            int c = peek_next_character();
            if (c == '*') {
                eat_character();
                comment_depth++;
            }
        } else if (c == '*') {
            eat_character();
            int c = peek_next_character();
            if (c == '/') {
                eat_character();
                comment_depth--;
            }
        } else {
            eat_character();
        }
    }
}

Token *Lexer::compose_new_token() {
    int c = peek_next_character();

    while (isspace(c)) {
        eat_character();
        c = peek_next_character();
    }

    if (c == -1) {
        return make_one_character_token(TOKEN_END_OF_INPUT);
    }

    if (starts_identifier(c)) {
        return make_ident_or_keyword();
    }

    if (c == '.') {
        // Check for double-dot... but if it is not specifically double-dot
        // then it might be a single-dot, and it might be a floating-point
        // number...

        eat_character();
        c = peek_next_character();
        if (c == '.') {
            eat_character();
            return make_one_character_token(TOKEN_DOUBLE_DOT);
        }

        if (c == '+') {
            eat_character();
            return make_one_character_token(TOKEN_DOTPLUS);
        }

        if (c == '-') {
            eat_character();
            return make_one_character_token(TOKEN_DOTMINUS);
        }

        if (isdigit(c)) {
            return make_integer_or_float('.');
        } 

        return make_one_character_token('.');
    }

    if (starts_number(c)) {
        return make_integer_or_float();
    }

    Token *result = NULL;
    int old_c = c;
    switch (c) {
    case '(': 
    case ')': 
    case ',': 
    case ';': 
    default:
        result = make_one_character_token(c);
        eat_character();
        break;
    case '\'':{
        eat_character();
        char *ident_name = parse_ident();
        if (!*ident_name) {
             interp->report_error(NULL, "Need a proper ident name following single-quote.\n");
        }

        result = make_token();
        result->type = TOKEN_STRING;
        result->string_value = ident_name;

        break;
    }
    case '+':
        result = check_for_equals(c, TOKEN_PLUSEQUALS);
        break;
    case '-':
        result = check_for_equals(c, TOKEN_MINUSEQUALS);
        break;
    case '*':
        result = check_for_equals(c, TOKEN_TIMESEQUALS);
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
        } else if (c == '*') {
            eat_character();
            eat_input_due_to_block_comment();
            return peek_next_token();
        } else if (c == '=') {
            eat_character();
            result = make_one_character_token(TOKEN_DIVEQUALS);
        } else {
            result = make_one_character_token('/');
        }
        break;
    case '!':
        result = check_for_equals(c, TOKEN_ISNOTEQUAL);
        break;
    case '=':
        result = check_for_equals(c, TOKEN_ISEQUAL);
        break;
    case '[':
        eat_character();
        c = peek_next_character();
        if (c == '[') {
            result = make_one_character_token(TOKEN_DOUBLEBRACKET_OPEN);
            eat_character();
        } else {
            result = make_one_character_token('[');
        }
        break;
    case ']':
        eat_character();
        c = peek_next_character();
        if (c == '[') {
            result = make_one_character_token(TOKEN_DOUBLEBRACKET_CLOSE);
            eat_character();
        } else {
            result = make_one_character_token(']');
        }
        break;
    case '<':
        eat_character();
        c = peek_next_character();
        if (c == '-') {
            result = make_one_character_token(TOKEN_IMPLIED_BY);
            eat_character();
        } else if (c == '=') {
            result = make_one_character_token(TOKEN_LESSEQUALS);
            eat_character();
        } else {
            result = make_one_character_token('<');
        }
        break;
    case '>':
        eat_character();
        c = peek_next_character();
        if (c == '=') {
            result = make_one_character_token(TOKEN_GREATEREQUALS);
            eat_character();
        } else {
            result = make_one_character_token('>');
        }
        break;
    case '?':
        eat_character();
        c = peek_next_character();
        if (c == '?') {
            eat_character();
            result = make_one_character_token(TOKEN_DOUBLE_QUESTION_MARK);
        } else if (c == '*') {
            eat_character();
            result = make_one_character_token(TOKEN_QUERY_WILDCARD);
        } else {
            result = make_query_variable();
        }

        break;
    case '$':
        eat_character();
        result = make_implicit_variable();
        break;
/*
    case '#':
        eat_character();
        result = make_entity_id();
        break;
*/
    case '"':
        eat_character();
        result = make_string_constant();
        break;
    }

    assert(result != NULL);
    return result;
}


Token *Lexer::peek_next_token() {
    if (current_token) return current_token;

    current_token = compose_new_token();
    return current_token;
}

Token *Lexer::peek_token(int lookahead_index) {
    assert(lookahead_index <= 1);
    assert(lookahead_index >= 0);

    if (lookahead_index == 0) return peek_next_token();

    Token *old_c = current_token;
    Token *old_l = lookahead_token;

    if (!current_token) current_token = compose_new_token();
    if (!lookahead_token) lookahead_token = compose_new_token();
    
    assert(lookahead_token != current_token);

    return lookahead_token;
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

bool Lexer::spool_file_input(bool buffer_first_character, int character) {
    assert(current_input_file);

    int len_max = MAX_FILE_BUFFER_SIZE - 1;

    int offset = 0;
    if (buffer_first_character) {
        offset = 1;
        file_buffer[0] = character;
    }

    len_max -= offset;
    char *read_position = file_buffer + offset;

    int length = fread(read_position, 1, len_max-1, (FILE *)current_input_file);
    if (length <= 0) return false;   // XXX See if it's EOF or a read error, log read error if so

    read_position[length] = '\0';

    input = file_buffer;
    input_cursor = read_position;

    return true;
}


Token::~Token() {
}
