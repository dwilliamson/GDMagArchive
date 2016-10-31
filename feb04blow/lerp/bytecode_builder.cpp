#include "general.h"
#include "bytecode.h"
#include "bytecode_builder.h"
#include "parser.h"
#include "parser_private.h"

#include "concatenator.h"
#include "lexer.h"  // For token types stored in Ast
#include "interp.h"

#define pack_u1b code_buffer->add_u1b
#define pack_u2b code_buffer->add_u2b

extern void break_here();


struct Loop_Patch_Address {
    int pc_of_address_slot;
};

struct Loop_Patch_List {
    Loop_Patch_List *parent;
    List patch_addresses;
};

void get_unop_from_token(int operator_token_type, int *unop_return) {
    switch (operator_token_type) {
    case '-':
        *unop_return = LERP_UNOP_MINUS;
        break;
    case '!':
        *unop_return = LERP_UNOP_NOT;
        break;
    default:
        // Nothing!  Hopefully they put -1 in unop_return so they know what happened.
        break;
    }
}

void get_binop_from_token(int operator_token_type, int *operator_type_return, bool *assignment_return) {
    int operator_type;
    bool assignment = false;
    switch (operator_token_type) {
    case '+':
        operator_type = LERP_BINOP_PLUS;
        break;
    case '-':
        operator_type = LERP_BINOP_MINUS;
        break;
    case '*':
        operator_type = LERP_BINOP_TIMES;
        break;
    case '/':
        operator_type = LERP_BINOP_DIV;
        break;
    case '<':
        operator_type = LERP_BINOP_LESS;
        break;
    case '>':
        operator_type = LERP_BINOP_GREATER;
        break;
    case TOKEN_PLUSEQUALS:
        operator_type = LERP_BINOP_PLUS;
        assignment = true;
        break;
    case TOKEN_MINUSEQUALS:
        operator_type = LERP_BINOP_MINUS;
        assignment = true;
        break;
    case TOKEN_TIMESEQUALS:
        operator_type = LERP_BINOP_TIMES;
        assignment = true;
        break;
    case TOKEN_DIVEQUALS:
        operator_type = LERP_BINOP_DIV;
        assignment = true;
        break;
    case TOKEN_ISEQUAL:
        operator_type = LERP_BINOP_ISEQUAL;
        break;
    case TOKEN_ISNOTEQUAL:
        operator_type = LERP_BINOP_ISNOTEQUAL;
        break;
    case TOKEN_LESSEQUALS:
        operator_type = LERP_BINOP_LESSEQUAL;
        break;
    case TOKEN_GREATEREQUALS:
        operator_type = LERP_BINOP_GREATEREQUAL;
        break;
    case TOKEN_DOTPLUS:
        operator_type = LERP_BINOP_DB_ADD;
        break;
    case TOKEN_DOTMINUS:
        operator_type = LERP_BINOP_DB_SUBTRACT;
        break;
    default:
        break_here();
        operator_type = -1;
        break;
    }

    *operator_type_return = operator_type;
    *assignment_return = assignment;
}


Lerp_Bytecode_Builder::Lerp_Bytecode_Builder(Lerp_Interp *_interp) {
    arguments = NULL;
    code_buffer = new Concatenator;
    interp = _interp;
    current_scope = NULL;
    current_patch_list = NULL;
}

Lerp_Bytecode_Builder::~Lerp_Bytecode_Builder() {
    delete [] arguments;
    delete code_buffer;
}

int Lerp_Bytecode_Builder::allocate_output_register() {
    return registers_allocated++;
}

void Lerp_Bytecode_Builder::pack_register(int register_index) {
    assert(register_index >= 0);
    assert(register_index < 65536);

    code_buffer->add_u2b(register_index);
}

int Lerp_Bytecode_Builder::note_bytecode_location() {
    return code_buffer->length();
}

void Lerp_Bytecode_Builder::patchup_2b(int location, int value) {
    code_buffer->modify_2b(location, value);
}

int Lerp_Bytecode_Builder::emit_code_for_return(Ast_Unary_Expression *expression) {
    int arg_register = emit_code_for_expression(expression->subexpression);

    code_buffer->add_u1b(LERP_BYTECODE_RETURN);
    pack_register(arg_register);
    return 0;  // Nobody should be looking at this register return value...
}


// Search rightward through any '.' operations to find an expression sequence
// at the right-hand side.  (Umm, of course there could be 0 '.' operations).
// If we find one, it defines the variables that will be bound into the local scope.
Ast_Declarative_Assertion *find_assertion(Ast *expression) {
    while (expression->type == AST_BINARY_EXPRESSION) {
        Ast_Binary_Expression *binexp = (Ast_Binary_Expression *)expression;
        if (binexp->operator_token_type == TOKEN_OPERATOR_ARRAY_SUBSCRIPT) return reinterpret_cast <Ast_Declarative_Assertion *>(binexp->right);
        if (binexp->operator_token_type != '.') break;
        expression = binexp->right;
    }

    if (expression->type == AST_DECLARATIVE_ASSERTION) return (Ast_Declarative_Assertion *)expression;
    return NULL;
}

int Lerp_Bytecode_Builder::collect_explicit_variables(Ast *expression, Variable ***variables_return) {
    Ast_Declarative_Assertion *sequence = find_assertion(expression);
    if (sequence == NULL) return 0;

    if (sequence->flags & ASSERTION_FLAGS_HAS_SCALAR_RESULT) {
        interp->report_bytecode_error("Attempt to use '??' in a nonsensical place.\n");
    }

    *variables_return = sequence->variables;
    return sequence->num_variables;
}

//
// @Refactor: emit_code_for_integer_range_each contains heavy cut-and-paste
// from emit_code_for_each.... this should be refactored of course.
//
int Lerp_Bytecode_Builder::emit_code_for_integer_range_each(Ast_Each *ast_each) {
    Ast *body = ast_each->body;

    assert(ast_each->list_expression->type == AST_BINARY_EXPRESSION);
    Ast_Binary_Expression *binexp = (Ast_Binary_Expression *)ast_each->list_expression;
    assert(binexp->operator_token_type == TOKEN_DOUBLE_DOT);

    // @Robustness: Typecheck these arguments early on, to see if they are
    // obviously not integers, or the left one is greater than the right, etc.
    int left_register = emit_code_for_expression(binexp->left);
    int right_register = emit_code_for_expression(binexp->right);


    int accumulator_register = -1;
    int binop_operator;
    bool binop_assignment;
    if (ast_each->flags & EACH_BINOP) {
        assert(ast_each->starter);
        accumulator_register = emit_code_for_expression(ast_each->starter);
        get_binop_from_token(ast_each->operator_token_type, &binop_operator, &binop_assignment);
        if (binop_assignment) {
            interp->report_bytecode_error("Assignments (e.g. operator '+=') not yet supported in 'each'.\n");
            return 0;
        }
    }


    int comparison_register = allocate_output_register();
    int register_for_1 = emit_code_for_constant(interp->parser->make_integer(1));

    int iterator_register = 0;

    pack_u1b(LERP_BYTECODE_COPY_REGISTER);
    pack_register(iterator_register);
    pack_register(left_register);



    // Start the loop

    int start_of_loop = note_bytecode_location();

    //
    // compare (left_register <= right_register)
    //    

    pack_u1b(LERP_BYTECODE_BINOP);
    pack_u1b(LERP_BINOP_LESSEQUAL);
    pack_register(comparison_register);
    pack_register(iterator_register);
    pack_register(right_register);
    

    // 
    // If not, break out of loop
    //
    code_buffer->add_u1b(LERP_BYTECODE_GOTO_IF_FALSE);
    int patch_position = note_bytecode_location();
    pack_register(0);
    pack_register(comparison_register);


    push_patch_list();

    int body_result_register = -1;
    if (body) {
        if (body->type == AST_BLOCK) {
            emit_code_for_block((Ast_Block *)ast_each->body);
        } else {
            body_result_register = emit_code_for_expression((Ast_Expression *)ast_each->body);
        }
    } else {
        // @Refactor: Ugh this is horrible... well, it seems $0 is always in
        // register 0, so let's just use that... Soon I am planning to make $_ and $0
        // different things.  At that time, we will need to reference $_ here, instead
        // of $0 (or else have the iterator value be placed in a regsiter somewhere
        // that we can get at it).
        body_result_register = 0;
    }

    //
    // If this is part of a binop, feed the result into the accumulator...
    //
    if (ast_each->flags & EACH_BINOP) {
        assert(body_result_register != -1);
        pack_u1b(LERP_BYTECODE_BINOP);
        pack_u1b(binop_operator);
        pack_u2b(accumulator_register);
        pack_u2b(accumulator_register);
        pack_u2b(body_result_register);

        assert(binop_operator != -1);
    }

    //
    // left := left + 1
    //
    pack_u1b(LERP_BYTECODE_BINOP);
    pack_u1b(LERP_BINOP_PLUS);
    pack_register(iterator_register);
    pack_register(iterator_register);
    pack_register(register_for_1);

    //
    // Time to loop back around.
    //
    code_buffer->add_u1b(LERP_BYTECODE_GOTO);
    code_buffer->add_u2b(start_of_loop);

    int loop_exit = note_bytecode_location();
    patchup_2b(patch_position, loop_exit);

    pop_patch_list(loop_exit);

    if (ast_each->flags & EACH_BINOP) {
        return accumulator_register;
    }

    return 0;
}

int Lerp_Bytecode_Builder::emit_code_for_each(Ast_Each *ast_each) {
    Ast *body = ast_each->body;

    if (body && (body->type == AST_BLOCK)) {
        // If the body is a block, it doesn't return a value.
        // If this 'each' is chaining an expression, it requires a value.
        // These two things in combination would be an error!

        if (ast_each->flags & EACH_BINOP) {
            interp->report_bytecode_error("Attempt to chain an 'each' whose body does not have a value (can't be a block enclosed in {}!).\n");
            return 0;
        }
    }

    if (ast_each->list_expression->type == AST_BINARY_EXPRESSION) {
        Ast_Binary_Expression *binexp = (Ast_Binary_Expression *)ast_each->list_expression;
        if (binexp->operator_token_type == TOKEN_DOUBLE_DOT) {
            return emit_code_for_integer_range_each(ast_each);
        }
    }

    int iterator_status_register = allocate_output_register();
    int scratch_register = allocate_output_register();

    int list_value_register = emit_code_for_expression(ast_each->list_expression);

    int accumulator_register = -1;
    int binop_operator;
    bool binop_assignment;
    if (ast_each->flags & EACH_BINOP) {
        assert(ast_each->starter);
        accumulator_register = emit_code_for_expression(ast_each->starter);
        get_binop_from_token(ast_each->operator_token_type, &binop_operator, &binop_assignment);
        if (binop_assignment) {
            interp->report_bytecode_error("Assignments (e.g. operator '+=') not yet supported in 'each'.\n");
            return 0;
        }
    }

    code_buffer->add_u1b(LERP_BYTECODE_EACH_BEGIN);
    pack_register(iterator_status_register);
    pack_register(scratch_register);
    pack_register(list_value_register);

    int start_of_loop = note_bytecode_location();

    code_buffer->add_u1b(LERP_BYTECODE_GOTO_IF_FALSE);
    int patch_position = note_bytecode_location();
    pack_register(0);
    pack_register(iterator_status_register);

    //
    // If this is an expression that defines explicit variables,
    // generate code to initialize each of those variables at the 
    // head of the loop.
    //
    Variable **explicit_variables;
    int num_variables = collect_explicit_variables(ast_each->list_expression, &explicit_variables);

    if (num_variables) push_scope();
    push_patch_list();

    int i;
    for (i = 0; i < num_variables; i++) {
        Variable *variable = explicit_variables[i];

        // Two steps: First: extract the value from the query tuple into a register;
        // Second: put that register into a global variable.  In the future we 
        // will actually want these to be local to the block scope, so:
        // XXX @Refactor: Make these locals, not globals.

        // Get the register for this variable...
        Atom *name = interp->parser->make_atom(variable->name);
        int var_register = allocate_output_register();
        add_variable_to_scope(name, var_register);
        
        // First step, extract value from tuple into the appropriate register.
        code_buffer->add_u1b(LERP_BYTECODE_TUPLE_PEEK);
        pack_register(var_register);
        pack_register(0);  // The tuple is in register 0 ($_)
        pack_register(i);  // @SolutionTuple
    }


    int body_result_register = -1;
    if (body) {
        if (body->type == AST_BLOCK) {
            emit_code_for_block((Ast_Block *)ast_each->body);
        } else {
            body_result_register = emit_code_for_expression((Ast_Expression *)ast_each->body);
        }
    } else {
        // @Refactor: Ugh this is horrible... well, it seems $0 is always in
        // register 0, so let's just use that... Soon I am planning to make $_ and $0
        // different things.  At that time, we will need to reference $_ here, instead
        // of $0 (or else have the iterator value be placed in a regsiter somewhere
        // that we can get at it).
        body_result_register = 0;
    }

    if (num_variables) pop_scope();

    //
    // If this is part of a binop, feed the result into the accumulator...
    //
    if (ast_each->flags & EACH_BINOP) {
        assert(body_result_register != -1);
        pack_u1b(LERP_BYTECODE_BINOP);
        pack_u1b(binop_operator);
        pack_u2b(accumulator_register);
        pack_u2b(accumulator_register);
        pack_u2b(body_result_register);

        assert(binop_operator != -1);
    }

    // 
    // Iterate to the next value in the list...
    //
    code_buffer->add_u1b(LERP_BYTECODE_EACH_NEXT);
    pack_register(iterator_status_register);
    pack_register(scratch_register);

    //
    // Time to loop back around.
    //
    code_buffer->add_u1b(LERP_BYTECODE_GOTO);
    code_buffer->add_u2b(start_of_loop);

    int loop_exit = note_bytecode_location();
    patchup_2b(patch_position, loop_exit);

    pop_patch_list(loop_exit);

    if (ast_each->flags & EACH_BINOP) {
        return accumulator_register;
    }

    return iterator_status_register;
}

int Lerp_Bytecode_Builder::emit_code_for_run_query(Ast_Declarative_Assertion *expression, Ast_Expression *owner) {
    int query_register = emit_code_for_expression((Ast_Expression *)expression);
    int result_register = allocate_output_register();

    int owner_register = -1;
    if (owner) owner_register = emit_code_for_expression(owner);

    pack_u1b(LERP_BYTECODE_RUN_QUERY);
    pack_u2b(expression->num_variables);
    pack_u2b(result_register);
    pack_u2b(query_register);

    if (owner) {
        pack_u1b(1);
        pack_u2b(owner_register);
    } else {
        pack_u1b(0);
    }

    return result_register;
}

int Lerp_Bytecode_Builder::emit_code_for_type_instantiation(Ast_Type_Instantiation *inst) {
    //
    // @Robustness: Check for multi-definitions in variable declaration!
    //
    int var_register = allocate_output_register();
    add_variable_to_scope(inst->variable_name, var_register);

    if (inst->initializer == NULL) {
        // Emit the instantiate instruction...
        int constant_for_type_name = add_to_constant_store(inst->type_name);
        code_buffer->add_u1b(LERP_BYTECODE_INSTANTIATE);
        pack_register(var_register);
        pack_register(constant_for_type_name);
    } else {
        // For now we are sort of just going to ignore the type, and assign
        // the value of the initializer to the expression...

        // @Robustness: At some time in the future we probably want to do a
        // typecheck here (for builds where typechecking is turned on)

        int rvalue_register = emit_code_for_expression(inst->initializer);

        //
        // Copy rvalue_register into var_register (we don't have a MOV instruction,
        // this might be a good thing to add!  @Speed!)
        //
        pack_u1b(LERP_BYTECODE_COPY_REGISTER);
        pack_register(var_register);
        pack_register(rvalue_register);
    }
    
    return var_register;
}

int Lerp_Bytecode_Builder::emit_code_for_array_subscript(Ast_Binary_Expression *sub) {
    //
    // Evaluating an array subscript as an rvalue...
    //
    int left_register = emit_code_for_expression(sub->left);
    // Note: Since we are emitting code for a sequence, but we only really need one
    // expression, we are gonna have some extra opcodes here... should find a way to
    // short-circuit this.
    int right_register = emit_code_for_optimized_sequence_formation(reinterpret_cast <Ast_Declarative_Assertion *>(sub->right));
    int result_register = allocate_output_register();

    pack_u1b(LERP_BYTECODE_RUN_QUERY_DOMAIN_SPECIFIED);
    pack_u2b(result_register);
    pack_u2b(left_register);
    pack_u2b(right_register);
    
    return result_register;
}


int Lerp_Bytecode_Builder::emit_code_for_while(Ast_While *ast_while) {
    int loop_head = note_bytecode_location();

    // Evaluate the expression that determines if we keep looping or not.
    // The result is stored in "condition_register".
    int condition_register = emit_code_for_expression(ast_while->conditional);

    //
    // Emit the branch instruction that can take us out of the loop.
    //
    pack_u1b(LERP_BYTECODE_GOTO_IF_FALSE);

    int goto_source_pos = note_bytecode_location();
    pack_u2b(0);  // This is a placeholder for the branch destination address, we will fill it in later...
    pack_u2b(condition_register);   // This is the value that the instruction tests for true or false.

    push_patch_list();

    // Done emitting branch instruction...

    // Emit the body of the loop...
    emit_code_for_block(ast_while->block);

    //
    // At the end of the loop is a goto statement that goes back around to the test...
    //
    pack_u1b(LERP_BYTECODE_GOTO);
    pack_u2b(loop_head);


    //
    // Now we are at the spot in the bytecode that we must jump to, in order to exit the loop.
    // Go back and patch up that branch instruction with the current address...
    //
    int goto_end_pos = note_bytecode_location();
    patchup_2b(goto_source_pos, goto_end_pos);

    pop_patch_list(goto_end_pos);

    return 0;  // 'while' just returns register 0, which is $_ for whoever set it last.
}

int Lerp_Bytecode_Builder::emit_code_for_if(Ast_If *ast_if) {
    int condition_register = emit_code_for_expression(ast_if->conditional);
    code_buffer->add_u1b(LERP_BYTECODE_GOTO_IF_FALSE);

    int goto_source_if_part = note_bytecode_location();
    pack_register(0);  // This is a dummy, we will fill it in later...

    pack_register(condition_register);
    
    emit_code_for_block(ast_if->then_block);
    
    int goto_source_else_part = -1;
    if (ast_if->else_block) {
        code_buffer->add_u1b(LERP_BYTECODE_GOTO);
        goto_source_else_part = note_bytecode_location();
        code_buffer->add_u2b(0);  // This is a dummy, we will fill it in later...
    }

    int then_end_pos = note_bytecode_location();
    patchup_2b(goto_source_if_part, then_end_pos);

    if (ast_if->else_block) {
        emit_code_for_block(ast_if->else_block);
        int else_end_pos = note_bytecode_location();
        patchup_2b(goto_source_else_part, else_end_pos);
    }

    return 0;  // Nobody should be looking at this register return value...
}

int Lerp_Bytecode_Builder::emit_code_for_break(Ast_Break *expression) {
    if (current_patch_list == NULL) {
        interp->report_bytecode_error("Illegal 'break'.\n");
        return 0;
    }

    Loop_Patch_Address *address = AST_NEW(Loop_Patch_Address);
    code_buffer->add_u1b(LERP_BYTECODE_GOTO);
    int patch_address = note_bytecode_location();
    code_buffer->add_u2b(0);
    current_patch_list->patch_addresses.add(address);

    address->pc_of_address_slot = patch_address;

    return 0;
}

int Lerp_Bytecode_Builder::emit_code_for_unary(Ast_Unary_Expression *expression) {
    if (expression->operator_token_type == TOKEN_KEYWORD_RETURN) {
        // This is an exception... RETURNs are stored in a unop structure for now
        return emit_code_for_return(expression);
    }

    int result_register;
    int arg_register = emit_code_for_expression(expression->subexpression);

    if (expression->operator_token_type == '+') {
        // Do nothing!
        return arg_register;
    }

    code_buffer->add_u1b(LERP_BYTECODE_UNOP);

    int operator_type = -1;
    get_unop_from_token(expression->operator_token_type, &operator_type);
    if (operator_type == -1) {
        interp->report_error("Weird error in unary operator.\n");
        return 0;
    }

    result_register = allocate_output_register();
    code_buffer->add_u1b(operator_type);
    assert(operator_type < LERP_UNOP_LIMIT);

    pack_register(result_register);
    pack_register(arg_register);

    return result_register;
}

// As it currently stands, ASSIGN would just fall into place as a BINOP
// and things would be a little simpler, but I built it out like this
// because I assume this would change in the future.  (if it doesn't,
// this will be a nice simplification).
int Lerp_Bytecode_Builder::emit_code_for_assignment(Ast_Expression *lvalue_expression, int rvalue_register) {
    // Based on the form of the lvalue_expression, we either want to do a struct member assignment
    // or a regular-old-assignment...

    if (lvalue_expression->type == ARG_ATOM) {
        int local_register = find_local_variable((Atom *)lvalue_expression);
        if (local_register != -1) {  // It's a local variable in lexical scope...
            //
            // Copy rvalue_register into local_register (we don't have a MOV instruction,
            // this might be a good thing to add!  @Speed!)
            //
            pack_u1b(LERP_BYTECODE_COPY_REGISTER);
            pack_register(local_register);
            pack_register(rvalue_register);
        } else {
            int constant_index = add_to_constant_store(lvalue_expression);

            code_buffer->add_u1b(LERP_BYTECODE_ASSIGN);
            pack_register(constant_index);
            pack_register(rvalue_register);
        }
    } else if (lvalue_expression->type == AST_BINARY_EXPRESSION) {
        Ast_Binary_Expression *binexp = (Ast_Binary_Expression *)lvalue_expression;
        if (binexp->operator_token_type == '.') {
            if (binexp->right->type == ARG_ATOM) {
                int owner_index = emit_code_for_expression(binexp->left);
            
                int constant_index = add_to_constant_store(binexp->right);
                code_buffer->add_u1b(LERP_BYTECODE_ASSIGN_STRUCT_MEMBER);
                pack_register(owner_index);
                pack_register(constant_index);
                pack_register(rvalue_register);
            } else {
                interp->report_bytecode_error("There has been an error in assignment lvalue type... binop must be '.'!!\n");
            }
        } else if (binexp->operator_token_type == TOKEN_OPERATOR_ARRAY_SUBSCRIPT) {
            int left_register = emit_code_for_expression(binexp->left);
            int right_register = emit_code_for_optimized_sequence_formation(reinterpret_cast <Ast_Declarative_Assertion *>(binexp->right));

            pack_u1b(LERP_BYTECODE_ASSIGN_ARRAY_SUBSCRIPT);
            pack_register(left_register);
            pack_register(right_register);
            pack_register(rvalue_register);
        }
    } else {
        interp->report_bytecode_error("There has been an error in assignment lvalue type!!\n");
    }

    return rvalue_register;
}

int Lerp_Bytecode_Builder::emit_code_for_lookup(Ast_Binary_Expression *expression) {
    Ast_Expression *left = expression->left;
    Ast_Expression *right = expression->right;

    if (right->type == AST_DECLARATIVE_ASSERTION) {
        return emit_code_for_run_query((Ast_Declarative_Assertion *)right, left);
    } else if (right->type != ARG_ATOM) {
        interp->report_bytecode_error("Type error on RHS of lookup -- must be IDENT\n");
        return 0;  // @Robustness: Hmmmm...
    }

    int left_register = emit_code_for_expression(left);
    int result_register = allocate_output_register();

    int constant_index = add_to_constant_store(right);
    
    code_buffer->add_u1b(LERP_BYTECODE_LOOKUP_RVALUE);
    code_buffer->add_u1b(3);
    pack_register(result_register);
    pack_register(constant_index);
    pack_register(left_register);

    return result_register;
}


void check_for_break(Ast_Expression *expression) {
    if (expression->type == ARG_ATOM) {
        Atom *atom = (Atom *)expression;
    }
}

int Lerp_Bytecode_Builder::emit_code_for_binary(Ast_Binary_Expression *expression) {
    if (expression->operator_token_type == TOKEN_DOUBLE_DOT) {
        interp->report_bytecode_error("The operator '..' can only be used in limited contexts (such as the list argument in an 'each' iteration.)");
        return 0;
    }

    if (expression->operator_token_type == '.') {
        return emit_code_for_lookup(expression);
    }


    if (expression->operator_token_type == TOKEN_OPERATOR_ARRAY_SUBSCRIPT) {
        return emit_code_for_array_subscript(expression);
    }

    int right_register = emit_code_for_expression(expression->right);

    if (expression->operator_token_type == '=') {
        return emit_code_for_assignment(expression->left, right_register);
    }

    check_for_break(expression->left);
    int left_register = emit_code_for_expression(expression->left);

    code_buffer->add_u1b(LERP_BYTECODE_BINOP);

    int result_register;
    result_register = allocate_output_register();
    
    int operator_type;
    bool assignment = false;
    get_binop_from_token(expression->operator_token_type, &operator_type, &assignment);

    code_buffer->add_u1b(operator_type);
    assert(operator_type != -1);

    pack_register(result_register);
    pack_register(left_register);
    pack_register(right_register);

    if (assignment) {
        result_register = emit_code_for_assignment(expression->left, result_register);
    }

    return result_register;
}

int Lerp_Bytecode_Builder::emit_code_for_procedure_call(Ast_Procedure_Call_Expression *expression) {
    int result_register = allocate_output_register();

    int owner_register = -1;
    int proc_register;
    if (expression->owner_expression) {
        assert(expression->procedure_expression->type == ARG_ATOM);

        owner_register = emit_code_for_expression(expression->owner_expression);
        proc_register = emit_code_for_lookup((Atom *)expression->procedure_expression, owner_register);
    } else {
        proc_register = emit_code_for_expression(expression->procedure_expression);
    }

    int num_arguments = expression->argument_list.items;

    //
    // Construct a calling record.
    //
    int record_register = allocate_output_register();
    code_buffer->add_u1b(LERP_BYTECODE_MAKE_CALLING_RECORD);
    pack_register(record_register);
    pack_register(proc_register);
    assert(num_arguments < 65536);
    code_buffer->add_u2b(num_arguments);
    
    //
    // Fill the record...
    //
    int poke_index = 0;
    Ast_Expression *arg;
    Foreach(&expression->argument_list, arg) {
        int arg_register = emit_code_for_expression(arg);
        
        code_buffer->add_u1b(LERP_BYTECODE_POKE_INTO_CALLING_RECORD);
        pack_register(record_register);
        code_buffer->add_u2b(poke_index + 1);  // Because register 0 is reserved.  // @RegisterConvention
        pack_register(arg_register);

        poke_index++;
    } Endeach;

    //
    // If there's an owner expression, we need to evaluate that, and then
    // set the 'this' pointer on the calling record.
    //

    if (expression->owner_expression) {
        assert(owner_register != -1);
        code_buffer->add_u1b(LERP_BYTECODE_SET_THIS_POINTER_ON_CALLING_RECORD);
        pack_register(record_register);
        pack_register(owner_register);
    }

    code_buffer->add_u1b(LERP_BYTECODE_CALL_PROCEDURE);
    pack_register(result_register);
    pack_register(proc_register);  // Note this is potentially redundant, we mentioned it earlier when we made the record
    pack_register(record_register);

    return result_register;
}

int Lerp_Bytecode_Builder::emit_code_for_declarative_expression(Ast_Declarative_Expression *expression) {
    assert(expression->type != AST_DECLARATIVE_EXPRESSION_PROCEDURAL);  // Not yet implemented!
    assert(expression->type == AST_DECLARATIVE_EXPRESSION);

    // Create the tuple...
    //
    int tuple_register = allocate_output_register();
    pack_u1b(LERP_BYTECODE_TUPLE_MAKE);
    pack_u2b(tuple_register);
    pack_u2b(expression->arguments.items);
    pack_u2b(expression->flags);  // I sure hope the flags fit within 2 bytes!

    // Let's make sure it fits into two bytes.
    assert((expression->flags & 0xffff) == expression->flags);

    //
    // For each item, generate code for the sucker and put it into the tuple.
    // @Speed, @Memory: We could check the tuple at preprocess time, and in cases
    // where the whole thing consists of constants, we could just stash the whole
    // tuple into constant storage and not bother with any of this eval stuff.
    //

    Ast_Expression *arg;
    int index = 0;
    Foreach(&expression->arguments, arg) {
        int value_register = emit_code_for_expression(arg);

        pack_u1b(LERP_BYTECODE_TUPLE_POKE);
        pack_u2b(tuple_register);
        pack_u2b(value_register);
        pack_u2b(index);
        index++;
    } Endeach;

    return tuple_register;
}

int Lerp_Bytecode_Builder::emit_code_for_sequence_piece(Ast_Declarative_Expression *expression, int sequence_register) {
    if (!expression) return 0;
    emit_code_for_sequence_piece(expression->next, sequence_register);
    int tuple_register = emit_code_for_declarative_expression(expression);

    //
    // Prepend the tuple to the sequence.
    //

    pack_u1b(LERP_BYTECODE_SEQUENCE_PREPEND);
    pack_u2b(sequence_register);
    pack_u2b(tuple_register);

    return sequence_register;
}


bool Lerp_Bytecode_Builder::type_is_constable(int type) {
    switch (type) {
    case ARG_VARIABLE:
    case ARG_INTEGER:
    case ARG_FLOAT:
    case ARG_STRING:
    case ARG_DATABASE:
    case ARG_BLOB_UNMANAGED:
        return true;
    } 
    return false;
}

bool Lerp_Bytecode_Builder::expression_is_constable(Ast_Declarative_Expression *expression) {
    if (expression == NULL) return true;  // So we don't have to check everything for NULL in the callers...
    if (expression->type == AST_DECLARATIVE_EXPRESSION_PROCEDURAL) return true;

    // Otherwise see if all its data arguments are const...

    Ast *arg;
    Foreach(&expression->arguments, arg) {
        if (!type_is_constable(arg->type)) return false;
    } Endeach;

    return true;
}

bool Lerp_Bytecode_Builder::sequence_is_constable(Ast_Declarative_Assertion *sequence) {
    if (!expression_is_constable(sequence->expression)) return false;
    
    Ast_Declarative_Expression *expression = sequence->conditionals;
    while (expression) {
        if (!expression_is_constable(expression)) return false;
        expression = expression->next;
    }

    return true;
}

static void add_expression(Decl_Assertion *assertion, Decl_Expression *expression) {
    // @WriteBarrier?!?

    Barrier <Decl_Expression *> **cursor = &assertion->conditionals;
    while (*cursor) {
        Decl_Expression *exp = (*cursor)->read();
        cursor = &exp->next;
    }

    *cursor = ToBarrier(expression);
}

Decl_Expression *Lerp_Bytecode_Builder::convert_ast_to_decl(Ast_Declarative_Expression *ast_expression) {
    if (ast_expression == NULL) return NULL;
    if (ast_expression->type == AST_DECLARATIVE_EXPRESSION_PROCEDURAL) {
        // XXXXX @Incomplete

        Decl_Expression_Procedural *result = GC_NEW(Decl_Expression_Procedural);
        result->proc = NULL;  // XXXXXX
        

        //return result;
        return NULL;
    }

    Decl_Expression *decl_expression = interp->memory_manager->create_decl_expression(ast_expression->arguments.items);
    decl_expression->flags = ast_expression->flags;

    int index = 0;
    First_Class *arg;
    Foreach(&ast_expression->arguments, arg) {
        decl_expression->arguments[index++] = ToBarrier(arg);
    } Endeach;

    return decl_expression;
}
        

void Lerp_Bytecode_Builder::finish_constant_sequence_prototype(Ast_Declarative_Assertion *sequence, Decl_Assertion *proto) {
    proto->flags |= ASSERTION_FLAGS_CONST;
    proto->expression = ToBarrier(convert_ast_to_decl(sequence->expression));

    Ast_Declarative_Expression *ast_expression = sequence->conditionals;
    while (ast_expression) {
        Decl_Expression *decl_expression = convert_ast_to_decl(ast_expression);
        add_expression(proto, decl_expression);
        ast_expression = ast_expression->next;
    }
}

int Lerp_Bytecode_Builder::emit_code_for_sequence_formation(Ast_Declarative_Assertion *sequence) {
    // 
    // Create the sequence...
    //
    int sequence_register = allocate_output_register();

    // Make a prototype sequence that has the variable list already there
    // so we don't have to postprocess it at runtime (hmmmm... this is a little
    // ugly, maybe we want to make it nicer.  @Refactor).
    Decl_Assertion *proto = GC_NEW(Decl_Assertion);
    proto->num_variables = sequence->num_variables;
    proto->variables = sequence->variables;  // XXX @Memory: Potentially unsafe thing here
    proto->flags = sequence->flags;
    proto->scalar_result_index = sequence->scalar_result_index;

    int prototype_constant = add_to_constant_store(proto);

    if (sequence_is_constable(sequence)) {
        finish_constant_sequence_prototype(sequence, proto);
        code_buffer->add_u1b(LERP_BYTECODE_LOAD_CONSTANT);
        pack_register(sequence_register);
        pack_register(prototype_constant);
        return sequence_register;
    }

    pack_u1b(LERP_BYTECODE_SEQUENCE_MAKE);
    pack_u2b(sequence_register);
    pack_u2b(prototype_constant);

    if (sequence->expression == NULL) {
        proto->expression = NULL;
    } else {
        assert(0);  // XXXX proto->expression needs to be set based on what's in the Ast_Declarative_Assertion
    }

    emit_code_for_sequence_piece(sequence->conditionals, sequence_register);
    return sequence_register;
}

int Lerp_Bytecode_Builder::emit_code_for_optimized_sequence_formation(Ast_Declarative_Assertion *sequence) {
//    if (sequence->expression == NULL) {
    if ((sequence->expression == NULL) && (sequence->num_variables == 0)) {
        Ast_Declarative_Expression *exp = sequence->conditionals;
        if (exp && (exp->next == NULL) && (exp->arguments.items == 1)) {
            // @Incomplete: We are dissing all the flags here, and the scalar_result_index, etc.
            // We might need the flags though?!?
            return emit_code_for_expression((Ast_Expression *)exp->arguments.nth(0));
        }
    }

    return emit_code_for_sequence_formation(sequence);
}

int Lerp_Bytecode_Builder::emit_code_for_implicit_variable(First_Class *expression) {
    Ast_Implicit_Variable *var = (Ast_Implicit_Variable *)expression;
    if (var->value == 0) return 0;  // This is $_ or $0... we always have one of those, it's register 0.

    assert(0);
    int result_register = allocate_output_register();
    return result_register;
}

// @Improvement: Flesh out constants_match some more.
bool constants_match(First_Class *a, First_Class *b) {
    if (a->type != b->type) return false;
    switch (a->type) {
    case ARG_ATOM:
        if (a == b) return true;
        return false;
    case ARG_INTEGER:
        if (((Integer *)a)->value == ((Integer *)b)->value) return true;
        return false;
    case ARG_FLOAT:
        if (((Float *)a)->value == ((Float *)b)->value) return true;
        return false;
    }

    return false;
}

int Lerp_Bytecode_Builder::add_to_constant_store(First_Class *expression, bool *added) {
    if (added) *added = false;

    int i;
    for (i = 0; i < constants.items; i++) {
        if (constants_match(constants[i], expression)) return i;
    }

    int result = constants.items;
    constants.add(expression);
    register_for_each_constant.add(-1);

    if (added) *added = true;
    return result;
}

int Lerp_Bytecode_Builder::emit_code_for_constant(First_Class *expression) {
    bool added;
    int constant_index = add_to_constant_store(expression, &added);

    // @Speed: We ought to try to re-use these registers...
    // Or maybe the optimization post-pass will deal with it.
    // Actually on second thought that is a better idea.

    int result_register = allocate_output_register();
    register_for_each_constant[constant_index] = result_register;

    code_buffer->add_u1b(LERP_BYTECODE_LOAD_CONSTANT);
    pack_register(result_register);
    pack_register(constant_index);

    return result_register;
}


int Lerp_Bytecode_Builder::find_local_variable(Atom *name) {
    assert(block_stack.items > 0);
    
    Ast_Lexical_Scope *scope = current_scope;
    while (scope) {
        Ast_Scope_Record *record;
        Foreach(&scope->variables, record) {
            if (record->atom == name) {
                assert(record->register_number >= 0);
                return record->register_number;
            }
        } Endeach;

        scope = scope->parent;
    }

    return -1;
}

int Lerp_Bytecode_Builder::emit_code_for_lookup(Atom *expression) {
    // Check to see if it's a local variable.  If it is, we just return
    // that register (easy huh!)
    int local_register = find_local_variable(expression);
    if (local_register != -1) return local_register;

    int constant_index = add_to_constant_store(expression);

    int result_register = allocate_output_register();

    code_buffer->add_u1b(LERP_BYTECODE_LOOKUP_RVALUE);
    code_buffer->add_u1b(0);
    pack_register(result_register);
    pack_register(constant_index);

    return result_register;
}

int Lerp_Bytecode_Builder::emit_code_for_lookup(Atom *expression, int owner) {
    int constant_index = add_to_constant_store(expression);

    int result_register = allocate_output_register();

    code_buffer->add_u1b(LERP_BYTECODE_LOOKUP_RVALUE);
    code_buffer->add_u1b(2);
    pack_register(result_register);
    pack_register(constant_index);
    pack_register(owner);

    return result_register;
}

int Lerp_Bytecode_Builder::emit_code_for_expression(Ast_Expression *expression) {
    int result;
    switch (expression->type) {
    case AST_UNARY_EXPRESSION:
        result = emit_code_for_unary((Ast_Unary_Expression *)expression);
        break;
    case AST_BINARY_EXPRESSION:
        result = emit_code_for_binary((Ast_Binary_Expression *)expression);
        break;
    case AST_PROCEDURE_CALL_EXPRESSION:
        result = emit_code_for_procedure_call((Ast_Procedure_Call_Expression *)expression);
        break;
    case AST_IMPLICIT_VARIABLE:
        result = emit_code_for_implicit_variable((First_Class *)expression);
        break;
    case AST_IF:
        result = emit_code_for_if((Ast_If *)expression);
        break;
    case AST_WHILE:
        result = emit_code_for_while((Ast_While *)expression);
        break;
    case AST_BREAK:
        result = emit_code_for_break((Ast_Break *)expression);
        break;
    case ARG_INTEGER:
    case ARG_FLOAT:
    case ARG_STRING:
//    case ARG_PRIMITIVE_FUNCTION:
    case ARG_IMPERATIVE_PROCEDURE:
    case ARG_VARIABLE:
    case ARG_UNINITIALIZED:
        result = emit_code_for_constant((First_Class *)expression);
        break;
    case ARG_ATOM:
        result = emit_code_for_lookup((Atom *)expression);
        break;
    case AST_DECLARATIVE_ASSERTION:
        result = emit_code_for_sequence_formation((Ast_Declarative_Assertion *)expression);
        break;
    case AST_DECLARATIVE_EXPRESSION:
        result = emit_code_for_declarative_expression((Ast_Declarative_Expression *)expression);
        break;
/*
    case ARG_DECL_ASSERTION:
        result = emit_code_for_run_query((Decl_Assertion *)expression);
        break;
*/
    case AST_EACH_HARDENED:
        result = emit_code_for_each((Ast_Each *)expression);
        break;
    case AST_TYPE_INSTANTIATION:
        result = emit_code_for_type_instantiation((Ast_Type_Instantiation *)expression);
        break;
    case AST_BLOCK:
        emit_code_for_block((Ast_Block *)expression);
        result = 0;  // @Robustness:  I sure hope there is something good in 0!
        break;
    default:
        assert(0);
        result = 0;
        break;
    }

    return result;
}

void Lerp_Bytecode_Builder::output_debug_info_for_statement(Ast_Statement *statement) {
    Statement_Debug_Info *info = new Statement_Debug_Info; // @Memory: Unmanaged memory  XXX
    info->line_number = statement->starting_line_number;
    info->program_counter = code_buffer->length();
    statement_debug_info_list.add(info);
}

void Lerp_Bytecode_Builder::add_variable_to_scope(Atom *name, int register_number) {
    assert(current_scope);

    Ast_Scope_Record *record = AST_NEW(Ast_Scope_Record);
    record->atom = name;
    record->register_number = register_number;

    //
    // @Robustness: Check for duplicate variables (redundant declarations!)
    //
    current_scope->variables.add(record);
}

void Lerp_Bytecode_Builder::push_scope() {
    Ast_Lexical_Scope *scope = new Ast_Lexical_Scope();
    scope->parent = current_scope;
    current_scope = scope;
}

void Lerp_Bytecode_Builder::pop_scope() {
    assert(current_scope);
    Ast_Lexical_Scope *scope = current_scope;
    current_scope = current_scope->parent;
    delete scope;
}

void Lerp_Bytecode_Builder::push_patch_list() {
    Loop_Patch_List *list = AST_NEW(Loop_Patch_List);
    list->parent = current_patch_list;
    current_patch_list = list;
}

void Lerp_Bytecode_Builder::pop_patch_list(int patch_destination) {
    Loop_Patch_List *list = current_patch_list;
    current_patch_list = list->parent;

    Loop_Patch_Address *dest;
    Foreach(&list->patch_addresses, dest) {
        patchup_2b(dest->pc_of_address_slot, patch_destination);
    } Endeach;

    list->patch_addresses.clean();
}

void Lerp_Bytecode_Builder::emit_code_for_block(Ast_Block *block, int num_arguments, Value_Array *argument_info) {
    if (block == NULL) return;

    //
    // @Refactor: push_scope() just pushes near-by lexical scopes.
    // However we also do this thing where we push global scopes.
    // This is perhaps a little bit confusing. At the very least we
    // ought to rename the functions in this file to be less ambiguous.
    // But even better, some way of making sure that the actual 
    // language semantics are also not confusing, would be good.
    //
    push_scope();

    // We must allocate procedure arguments first, because that's how we
    // keep the register convention.  Note that the nonlocal scope thingy
    // below, actually might fill some registers (since it moves constant
    // values into registers to look up scope names) so it has to happen
    // after this.
    int i;
    for (i = 0; i < num_arguments; i++) {
        int register_number = allocate_output_register();
        add_variable_to_scope((Atom *)argument_info->values[i*2+1]->read(), register_number);
    }




    if (block->scopes.items) {
        // We have global scopes to push before executing this block...
        assert(block->scopes.items < 65536);  // We are packing the count in 16 bits in the POP instruction.
        Atom *name;
        Foreach(&block->scopes, name) {
            // Emit a
            int name_register = emit_code_for_constant(name);
            assert(name->type == ARG_ATOM);
            pack_u1b(LERP_BYTECODE_PUSH_NAMESPACE);
            pack_u2b(name_register);    // @Speed: Should probably just put the constant index here
        } Endeach;
    }

    block_stack.add(block);

    // Generate code for the statements.

    Ast_Statement *statement;
    Foreach(&block->statements, statement) {
        if (interp->generate_debug_info) output_debug_info_for_statement(statement);
        Ast_Expression *expression = statement->expression;
        emit_code_for_expression(expression);
    } Endeach;

    void *removed = block_stack.remove_tail();
    assert(removed == (void *)block);

    pop_scope();

    if (block->scopes.items) {
        pack_u1b(LERP_BYTECODE_POP_NAMESPACES);
        pack_u2b(block->scopes.items);
    }
}

Lerp_Bytecode *Lerp_Bytecode_Builder::build(Ast_Proc_Declaration *declaration) {
    code_buffer->reset();

    int num_arguments = declaration->argument_list.items / 2;
    assert(num_arguments * 2 == declaration->argument_list.items);  // Make sure it wasn't an odd number.

    Value_Array *argument_info = interp->memory_manager->create_value_array(num_arguments * 2);

    Atom *argument_name;
    Atom *type_name;
    int cursor = 0;

    Listnode *node = declaration->argument_list.head;
    while (node) {
        assert(node->next);
        argument_name = (Atom *)node->data;
        type_name = (Atom *)node->next->data;
        node = node->next->next;

        // Note that we are switching the order ... type name is first here, but argument
        // name was first in the Ast.  (Maybe we should straighten this out at some point.)
        argument_info->values[cursor*2+0] = ToBarrierF(type_name);
        argument_info->values[cursor*2+1] = ToBarrierF(argument_name);
        cursor++;
    }

    // @Register_Convention
    registers_allocated = 1;  // @Register_Convention: Registers 1-n get allocated later.... in emit_code_for_block  // The registers 1-n are incoming arguments; register 0 is reserved for $_

    assert(statement_debug_info_list.items == 0);

    Ast_Block *block = declaration->body;
    emit_code_for_block(block, num_arguments, argument_info);

    int num_constants = constants.items;
    Lerp_Bytecode *result = interp->memory_manager->create_bytecode(num_constants);
    result->procedure = NULL;

    result->num_arguments = num_arguments;  // @Refactor: This is redundant with the Value_Array.
    result->argument_info = ToBarrier(argument_info);

    if (interp->generate_debug_info) {
        result->debug_info = new List;
        *result->debug_info = statement_debug_info_list;
        statement_debug_info_list.head = NULL;
        statement_debug_info_list.tail = NULL;
        statement_debug_info_list.items = 0;
    }

    // 
    // Code is generated; do other finalizing stuff.
    //


    char *data;
    unsigned int len;
    data = code_buffer->get_nozeroterm_result(&len);

    result->data = data;
    result->length = len;
    result->name = declaration->proc_name;


//    printf("**** bytecode is %d bytes long\n", len);

    result->num_registers = registers_allocated;

    //
    // Init the constants...
    //

    int constant_index = 0;
    First_Class *constant;
    Array_Foreach(&constants, constant) {
        result->constants[constant_index] = ToBarrier(constant);
        constant_index++;
    } Endeach;

    return result;
}
