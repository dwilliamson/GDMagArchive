#include "general.h"
#include "hash_table.h"

//
// What we have here is a hash table.  No big deal, I won't
// comment it.
//

char *copy_string(char *s) {
    int len = strlen(s);
    char *result = new char[len + 1];
    strcpy(result, s);
    return result;
}




// Derived from hashpjw.
inline unsigned int hash_string(char *s) {
    int result = 0;

    const int BITS_IN_RESULT= sizeof(result) * 8;
    const int ONE_EIGHTH = BITS_IN_RESULT/ 8;
    const int THREE_QUARTERS = (BITS_IN_RESULT * 3) / 4;
    const int HIGH_BITS = ~((unsigned int)(~0) >> ONE_EIGHTH);

    while (*s) {
        result = (result << ONE_EIGHTH) + *s;
        int high = result & HIGH_BITS;
        result = (result ^ (high >> THREE_QUARTERS)) & ~HIGH_BITS;
        s++;
    }

    return (unsigned int)result;
}

String_Hash_Table::String_Hash_Table(int _table_size) {
    table_size = _table_size;
    lists = new List[table_size];
    items = 0;
}

 
String_Hash_Table::~String_Hash_Table() {
    int i;
    for (i = 0; i < table_size; i++) {
        List *list = &lists[i];
        Typed_Hash_Info *info;
        Foreach(list, info) {
            delete info;
        } Endeach;
    }

    delete [] lists;
}

 
void String_Hash_Table::add(char *name, void *value) {
    int code = hash_string(name) % table_size;
    List *list = &lists[code];

    Typed_Hash_Info *info = new Typed_Hash_Info;
    info->code = code;
    info->name = copy_string(name);
    info->value = value;

    lists[code].add(info);
    items++;

    if (items >= table_size * MAX_OVERCROWDING_FACTOR) resize();
}

 
bool String_Hash_Table::remove(char *name, void **value_return) {
    // Removal is not implemented in this project.
    assert(0);
    return false;
}

 
bool String_Hash_Table::find(char *name, void **value_return) {
    int code = hash_string(name) % table_size;
    List *l = &lists[code];

    Typed_Hash_Info *info;
    Foreach(l, info) {
        if (info->code == code) {
            if (strcmp(name, info->name) == 0) {
                *value_return = info->value;
                return true;
            }
        }
    } Endeach;

    return false;
}


void String_Hash_Table::resize() {
    int new_table_size = table_size * MAX_OVERCROWDING_FACTOR;
    List *new_lists = new List[new_table_size];
    if (new_lists == NULL) return;

    List *old_lists = lists;
    int old_table_size = table_size;

    table_size = new_table_size;
    lists = new_lists;

    int i;
    for (i = 0; i < old_table_size; i++) {
        List *list = &old_lists[i];

        Typed_Hash_Info *info;
        Foreach(list, info) {
            int code = hash_string(info->name);
            info->code = code;
            lists[code].add(info);
        } Endeach;
    }

    delete [] old_lists;
}

