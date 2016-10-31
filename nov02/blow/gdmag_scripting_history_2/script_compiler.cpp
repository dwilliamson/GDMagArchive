#include "app_shell.h"
#include "scripting.h"
#include "script_compiler.h"

#include <ctype.h>


Script_Compiler::Script_Compiler() {
    log_file = NULL;
}

bool is_a_valid_first_character_of_a_name(int c) {
    if (isalpha(c)) return true;
    if (c == '_') return true;

    return false;
}

bool is_a_valid_successive_character_of_a_name(int c) {
    if (isalnum(c)) return true;
    if (c == '_') return true;

    return false;
}

char *skip_whitespace(char *s) {
    while (isspace(*s)) s++;
    return s;
}

void chop_up_to_whitespace(char *start, char **end_return) {
    char *cursor = start;

    while (1) {
        if (isspace(*cursor) || (*cursor == '\0')) {
            if (*cursor != '\0') {
                *cursor = '\0';
                cursor++;
            }

            if (end_return) *end_return = cursor;
            return;
        }

        cursor++;
    }
}

void chop_up_to_endquote(char *start, char **end_return) {
    char *cursor = start;

    while (1) {
        if ((*cursor == '\"') || (*cursor == '\0')) {
            if (*cursor != '\0') {
                *cursor = '\0';
                cursor++;
            }

            if (end_return) *end_return = cursor;
            return;
        }

        cursor++;
    }
}



void Script_Compiler::report_error(char *s) {
    if (log_file == NULL) {
        log_file = fopen("error_log_from_compiler.txt", "wt");
        if (log_file == NULL) return;
    }

    fprintf(log_file, "Error in line %d: \n    %s\n", line_number, s);
    fflush(log_file);
}

float Script_Compiler::parse_real(char *s) {
    float value;
    int success = sscanf(s, "%f", &value);
    if (success == 0) {
        report_error("Parse error trying to read a number.\n");
        return 0.0f;
    }

    return value;
}

void Script_Compiler::chop_word(char *s, char **end_return) {
    char *start = s;
    char *cursor = s;

    while (1) {
        bool valid;
        if (cursor == start) {
            valid = is_a_valid_first_character_of_a_name(*cursor);
        } else {
            valid = is_a_valid_successive_character_of_a_name(*cursor);
        }

        if (!valid) {
            if (*cursor && !isspace(*cursor)) {
                report_error("Parse error: invalid character in command name.");
            }

            if (*cursor) {
                *cursor = '\0';
                cursor++;
            }

            if (end_return) *end_return = cursor;
            break;
        }

        cursor++;
    }
}

void Script_Compiler::process_variable_declaration(char *s) {
    char *type_name = s;
    char *name;

    chop_word(type_name, &name);
    chop_word(name, NULL);

    if ((type_name == name) || (name[0] == '\0')) {
        report_error("Parse error in variable declaration.\n");
        return;
    }


    Variable_Type type_enum;
    if (strcmp(type_name, "vector2") == 0) {
        type_enum = VARTYPE_VECTOR2;
    } else if (strcmp(type_name, "real") == 0) {
        type_enum = VARTYPE_SCALAR;
    } else if (strcmp(type_name, "string") == 0) {
        type_enum = VARTYPE_STRING;
    } else {
        report_error("Invalid type in variable declaration.\n");
        return;
    }

    Variable_Binding *binding = global_variables->lookup(name);
    if (binding) {
        char buf[BUFSIZ];
        sprintf(buf, "Variable '%s' has already been declared.\n", name);
        report_error(buf);
        return;
    }

    global_variables->declare(name, type_enum);
}


void Script_Compiler::process_command_helper(char **argument_strings, int num_arguments) {

    // If the command starts with a literal, it's probably a synthetically
    // generated thing whose goal is to set the return value, so make
    // that explicit here.

    char *command_name = argument_strings[0];
    int argument_start_index = 1;

    if ((isdigit(command_name[0]) || (command_name[0] == '.') || (command_name[0] == '\"') || (command_name[0] == '-'))) {
        command_name = "instantiate_literal";
        argument_start_index = 0;
    }

    // Okay, now proceed to parse the command.
    Script_Command command;
    command.command_name = strdup(command_name);

    command.argc = num_arguments - argument_start_index;
    command.arguments = new Command_Argument_Spec[command.argc];
    command.line_number = line_number;
    assert(command.line_number > 0);

    // Process the argument strings into semantic thingies.
    int i;
    for (i = 0; i < command.argc; i++) {
        char *argument_string = argument_strings[i + argument_start_index];
        Command_Argument_Spec *spec = &command.arguments[i];

        if (isdigit(argument_string[0]) || (argument_string[0] == '.') || (argument_string[0] == '-')) {
            spec->type = Command_Argument_Spec::LITERAL_REAL;
            spec->real_value = parse_real(argument_string);
        } else if (argument_string[0] == '\"') {
            argument_string++;

            spec->type = Command_Argument_Spec::LITERAL_STRING;
            spec->string_data = app_shell->strdup(argument_string);
        } else if (argument_string[0] == '&') {
            if (strlen(argument_string) > 1) {
                argument_string++;
            } else {
                report_error("Parse error after '&'.");
                argument_string = "_error";
            }

            spec->type = Command_Argument_Spec::LVALUE;
            spec->variable_name = app_shell->strdup(argument_string);
        } else {
            spec->type = Command_Argument_Spec::VARIABLE;
            spec->variable_name = app_shell->strdup(argument_string);
        }
    }

    assert(num_commands < MAX_COMMANDS);
    commands[num_commands++] = command;
}

void Script_Compiler::process_command(char *s) {
    if (num_commands == MAX_COMMANDS) {
        report_error("Maximum number of commands exceeded.\n");
        return;
    }

    char *argument_strings[MAX_ARGUMENTS_PER_COMMAND];

    int num_arguments = 0;
    while (1) {
        if (num_arguments == MAX_ARGUMENTS_PER_COMMAND) {
            report_error("Limit exceeded on maximum number of arguments per command.\n");
            break;
        }

        char *next_argument = skip_whitespace(s);
        if (*next_argument == '\0') break;

        char *remainder;
        if (*next_argument == '\"') {
            // It's a string!
            chop_up_to_endquote(next_argument + 1, &remainder);
        } else if (*next_argument == '=') {
            remainder = next_argument + 1;
            next_argument = "=";
        } else {
            // It's an identifier or an integer
            chop_up_to_whitespace(next_argument, &remainder);
        }

        s = remainder;

        argument_strings[num_arguments++] = next_argument;
    }

    if (num_arguments == 0) {
        report_error("Parse error.\n");
        return;
    }


    // Okay.  Now, if the second argument in the list is "=", then
    // that's syntactic sugar to generate two commands in a row,
    // one of which does everything to the right-hand side of "=",
    // and the other which assigns the left-hand side of "="
    // to whatever the return value is.

    if ((num_arguments > 1) && (strcmp(argument_strings[1], "=") == 0)) {
        if (num_arguments == 2) {
            report_error("Empty right-hand side after '='.");
            return;
        }

        char *tmp_argument_strings[2];
        tmp_argument_strings[0] = "assign_return_value";
        char buf[BUFSIZ]; // XXX static buffer string

        strcpy(buf + 1, argument_strings[0]);
        buf[0] = '&';


        // Expand the syntactic sugar.
        process_command_helper(argument_strings + 2, num_arguments - 2);

        tmp_argument_strings[1] = buf;
        process_command_helper(tmp_argument_strings, 2);
    } else {
        // Do a regular old command
        process_command_helper(argument_strings, num_arguments);
    }
} 


void Script_Compiler::kill_newline(char *s) {
    int len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n') s[len - 1] = '\0';
}

void Script_Compiler::process_line(char *s) {
    kill_newline(s);

    while (isspace(*s)) s++;

    if (s[0] == ':') {
        process_variable_declaration(s + 1);
        return;
    }

    if (*s == '\0') return;
    if (*s == '#') return;

    process_command(s);
}

Compiled_Script *Script_Compiler::compile_script(FILE *f) {
    global_variables = new Variable_Binding_List();
    global_variables->init();

    num_commands = 0;


    char buf[BUFSIZ]; 

    line_number = 0;
    while (1) {
        line_number++;
        char *s = fgets(buf, BUFSIZ, f);
        if (s == NULL) break;
        process_line(s);
    }

    
    Script_Command_Sequence *command_sequence = new Script_Command_Sequence;
    command_sequence->num_commands = num_commands;
    command_sequence->commands = new Script_Command[num_commands];
    int i;
    for (i = 0; i < num_commands; i++) {
        command_sequence->commands[i] = commands[i];
    }

    num_commands = 0;



    Compiled_Script *script = new Compiled_Script;
    script->global_variables = global_variables;
    global_variables = NULL;

    script->commands = command_sequence;
    return script;
}
