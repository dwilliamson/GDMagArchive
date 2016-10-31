struct Lvalue;
struct Atom;
struct Database;

enum {
    NS_FLAG_HEAVYWEIGHT = 0x1  // This means, don't pass by value  (@Incomplete: not yet used)
};

// XXXXX Schemas must now be copied if their instantiation count is nonzero when they are modified!
struct Schema : public First_Class {
    Schema();
    Barrier <Value_Array *> *named_slots;  // May be NULL
    Atom *type_name;

    LEXPORT void initialize_slot(int index, Atom *name, First_Class *value);

    int instantiation_count;
    unsigned long type_tag;   // Only initialized if we've been instantiated.
    unsigned long flags;

    List db_constraints;  // Each of these is a Decl_Constraint
};

inline Schema::Schema() {
    type = ARG_SCHEMA;
    type_name = NULL;
    instantiation_count = 0;
    type_tag = 0;
    flags = 0;
    named_slots = NULL;
}

inline First_Class *read_nth_member(First_Class *fc, int index) {
    Barrier <First_Class *> **slots = (Barrier <First_Class *> **)fc;

    // The +1 is to skip the type tag...
    First_Class *result = slots[index + 1]->read();
    return result;
}

inline Barrier <First_Class *> **address_of_nth_member(First_Class *fc, int index) {
    Barrier <First_Class *> **slots = (Barrier <First_Class *> **)fc;
    // The +1 is to skip the type tag...
    return slots + (index + 1);
}
