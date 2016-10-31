struct Atom;
struct Lerp_Interp;

enum Token_Type {
    // Mentally insert ASCII types here
    TOKEN_IDENT = 256,
    TOKEN_QUERY_VARIABLE,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_FLOAT,
    TOKEN_ENTITY_ID,
    TOKEN_IMPLIED_BY,
    TOKEN_PLUSPLUS,
    TOKEN_MINUSMINUS,
    TOKEN_PLUSEQUALS,
    TOKEN_MINUSEQUALS,
    TOKEN_TIMESEQUALS,
    TOKEN_DIVEQUALS,
    TOKEN_ISEQUAL,
    TOKEN_ISNOTEQUAL,
    TOKEN_TYPE_NAME,
    TOKEN_LESSEQUALS,
    TOKEN_GREATEREQUALS,
    TOKEN_IMPERATIVE_IMPLICIT_VARIABLE,
    TOKEN_DOTPLUS,
    TOKEN_DOTMINUS,

    TOKEN_KEYWORD_PROC,
    TOKEN_KEYWORD_EACH,
    TOKEN_KEYWORD_SORT,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_THEN,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_BREAK,
    TOKEN_KEYWORD_USING,

    TOKEN_DOUBLEBRACKET_OPEN,
    TOKEN_DOUBLEBRACKET_CLOSE,
    TOKEN_DOUBLE_DOT,
    TOKEN_DOUBLE_QUESTION_MARK,
    TOKEN_QUERY_WILDCARD,

    TOKEN_END_OF_INPUT,

    TOKEN_ERROR
};

struct Token {
    ~Token();

    Token_Type type;
    int line_number;
    int character_index;
    int num_characters;

    union {
        Atom *name;
        int integer_value;
        double float_value;
        char *string_value;
    };
};

const int MAX_TOKEN_LENGTH = 512;
const int TOKEN_DISPOSAL_RING_SIZE = 8;
const int MAX_FILE_BUFFER_SIZE = 2000;

struct Lexer {
    Lexer(Lerp_Interp *interp);
    ~Lexer();

    void set_input_from_string(char *string);
    void set_input_from_file(void *file);  // The (void *) is actually a (FILE *)
                                           // but we don't want to require stdio... damn C++.
    Token *peek_next_token();
    Token *peek_token(int lookahead);
    void eat_token();

    int current_line_number;
    int current_character_index;

  protected:
    int peek_next_character();
    void eat_character();
    void unwind_one_character();

    Token *compose_new_token();


    Token *make_ident_or_keyword();
    Token *make_integer_or_float(int preload_charcater = -1);
    Token *make_query_variable();
    Token *make_implicit_variable();
    Token *make_entity_id();
    Token *make_string_constant();
    Token *make_one_character_token(int c);

    Token *make_token();


    char *parse_ident();
    bool parse_integer(int *result);
    bool parse_nonnegative_integer(int *result);

    void set_num_characters(Token *);
    bool spool_file_input(bool buffer_a_character = false, int character_to_buffer = -1);

    Lerp_Interp *interp;

    Token *token_disposal_ring[TOKEN_DISPOSAL_RING_SIZE];
    int token_disposal_ring_index;

    Token *current_token;
    Token *lookahead_token;

    char *input;
    char *input_cursor;

    void *current_input_file;
    char file_buffer[MAX_FILE_BUFFER_SIZE];

    char scratch_buffer[MAX_TOKEN_LENGTH];  // @Robustness: Kind of lame, do something better!

    Token *check_for_equals(int c, int augmented);
    void eat_input_due_to_block_comment();
};

