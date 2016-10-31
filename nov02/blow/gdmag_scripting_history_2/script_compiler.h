// Okay, it is relly lame-ass that there's a maximum
// number of commands you can have, and stuff.  But I
// am trying to keep my code as small as possible for
// these examples, which means, like, no data structures,
// and stuff.  Any "real" scripting language that has a
// limit like this should be shot.

const int MAX_COMMANDS = 400;

// Also, note that the code writes error messages into
// static buffers all over the place.  This is very bad
// practice and thus you could surely crash the system
// with buffer overruns if you try.  This is one of the
// first things I would change if making a production
// system (before even removing the command limit!)
// The reason is, the longer this kind of problem stays
// around, the more complexly it becomes couched in the
// rest of the code, and it becomes harder to eradicate.

// But hey, I wrote this "scripting language" in a day,
// I'm allowed to take shortcuts.

struct Script_Compiler {
    Script_Compiler();

    Compiled_Script *compile_script(FILE *f);
    Variable_Binding_List *global_variables;

    int num_commands;
    Script_Command commands[MAX_COMMANDS];
    
    int line_number;

    void report_error(char *error_message);

  protected:
    void process_line(char *s);

    void process_variable_declaration(char *s);
    void process_command(char *s);
    void process_command_helper(char **argument_strings, int num_arguments);

    float parse_real(char *s);

    void kill_newline(char *s);
    void chop_word(char *s, char **end_return);

    FILE *log_file;
};

