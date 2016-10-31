/*
  This implements a lexical analyzer for the language parsing system.
  It's not central to the idea of this month's code so I will not yet
  comment it heavily.
*/

enum Token_Type {
    // Mentally insert ASCII types here
    TOKEN_IDENT = 256,
    TOKEN_VARIABLE,
    TOKEN_INTEGER,
    TOKEN_ENTITY_ID,
    TOKEN_IMPLIED_BY,

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
        char *name;
        int integer_value;
    };
};

const int MAX_TOKEN_LENGTH = 512;
const int TOKEN_DISPOSAL_RING_SIZE = 8;
const int MAX_FILE_BUFFER_SIZE = 2000;

struct Lexer {
    Lexer();
    ~Lexer();

    void set_input_from_string(char *string);
    void set_input_from_file(void *file);  // The (void *) is actually a (FILE *)
                                           // but we don't want to require stdio... damn C++.
    Token *peek_next_token();
    void eat_token();

  protected:
    int peek_next_character();
    void eat_character();

    Token *make_ident();
    Token *make_integer();
    Token *make_variable();
    Token *make_entity_id();

    Token *make_one_character_token(int c);

    Token *make_token();


    char *parse_ident();
    bool parse_integer(int *result);
    bool parse_nonnegative_integer(int *result);

    void set_num_characters(Token *);
    bool spool_file_input();

    Token *token_disposal_ring[TOKEN_DISPOSAL_RING_SIZE];
    int token_disposal_ring_index;

    Token *current_token;

    char *input;
    char *input_cursor;

    void *current_input_file;
    char file_buffer[MAX_FILE_BUFFER_SIZE];

    int current_line_number;
    int current_character_index;

    char scratch_buffer[MAX_TOKEN_LENGTH];  // @Robustness: Kind of lame, do something better!


    bool parse_nonnegative_integer_helper(char *scratch_pos, int *result_ret);
};

