struct Atom;
struct First_Class;

enum Lerp_Bytecode_Instruction {
    LERP_BYTECODE_COPY_REGISTER,
    LERP_BYTECODE_UNOP,
    LERP_BYTECODE_BINOP,
    LERP_BYTECODE_LOOKUP_RVALUE,
    LERP_BYTECODE_PUSH_NAMESPACE,
    LERP_BYTECODE_POP_NAMESPACES,
    LERP_BYTECODE_LOAD_CONSTANT,
    LERP_BYTECODE_MAKE_CALLING_RECORD,
    LERP_BYTECODE_POKE_INTO_CALLING_RECORD,
    LERP_BYTECODE_SET_THIS_POINTER_ON_CALLING_RECORD,
    LERP_BYTECODE_CALL_PROCEDURE,
    LERP_BYTECODE_RETURN,
    LERP_BYTECODE_GOTO_IF_FALSE,
    LERP_BYTECODE_GOTO,
    LERP_BYTECODE_RUN_QUERY,
    LERP_BYTECODE_RUN_QUERY_DOMAIN_SPECIFIED,
    LERP_BYTECODE_EACH_BEGIN,
    LERP_BYTECODE_EACH_NEXT,
    LERP_BYTECODE_TUPLE_MAKE,
    LERP_BYTECODE_TUPLE_PEEK,
    LERP_BYTECODE_TUPLE_POKE,
    LERP_BYTECODE_SEQUENCE_MAKE,
    LERP_BYTECODE_SEQUENCE_PREPEND,

    LERP_BYTECODE_ASSIGN,
    LERP_BYTECODE_ASSIGN_STRUCT_MEMBER,
    LERP_BYTECODE_ASSIGN_ARRAY_SUBSCRIPT,
    LERP_BYTECODE_INSTANTIATE
};

enum Lerp_Bytecode_Binop {
    LERP_BINOP_PLUS,
    LERP_BINOP_MINUS,
    LERP_BINOP_TIMES,
    LERP_BINOP_DIV,
    LERP_BINOP_ISEQUAL,
    LERP_BINOP_ISNOTEQUAL,
    LERP_BINOP_GREATER,
    LERP_BINOP_GREATEREQUAL,
    LERP_BINOP_LESS,
    LERP_BINOP_LESSEQUAL,
    LERP_BINOP_LOOKUP,
    LERP_BINOP_DB_ADD,
    LERP_BINOP_DB_SUBTRACT,

    LERP_BINOP_LIMIT
};

enum Lerp_Bytecode_Unop {
    LERP_UNOP_MINUS,
    LERP_UNOP_NOT,

    LERP_UNOP_LIMIT
};

// Bytecode is only a child of First_Class so that we can GC it... hmm...
struct Lerp_Bytecode : public First_Class {
    Lerp_Bytecode();
    ~Lerp_Bytecode();

    int num_arguments;
    Barrier <Value_Array *> *argument_info;  // 2 * number of arguments, type name first, argument name second; type name may be NULL.

    int num_registers;

    Atom *name;
    char *data;
    int length;

    Barrier <Procedure *> *procedure;
    List *debug_info;
    int num_constants;
    Barrier <First_Class *> *constants[1];

    static int allocation_size(int num_arguments);
};

inline int Lerp_Bytecode::allocation_size(int num_arguments) {
    int extra_bytes = (num_arguments - 1) * sizeof(First_Class *);  // This could be -4!
    int size = sizeof(Lerp_Bytecode) + extra_bytes;
    return size;
}

struct Statement_Debug_Info {
    int line_number;
    int program_counter;
};
