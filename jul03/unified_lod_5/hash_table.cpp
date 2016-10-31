#include "framework.h"
#include "hash_table.h"

Hash_Table::Hash_Table(int _table_size) {
    table_size = _table_size;
    lists = new List[table_size];
    items = 0;
}

Hash_Table::~Hash_Table() {
    int i;
    for (i = 0; i < table_size; i++) {
        List *list = &lists[i];
        Hashable *hashable;
        Foreach(list, hashable) {
            delete hashable;
        } Endeach;
    }

    delete [] lists;
}

void Hash_Table::add(Hashable *hashable) {
    int code = hashable->get_hash_code();
    assert(code >= 0);
    hashable->hash_code = code;

    int table_index = code % table_size;
    lists[table_index].add(hashable);
    items++;

    if (items >= table_size * MAX_OVERCROWDING_FACTOR) resize();
}

void Hash_Table::resize() {
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
        Hashable *hashable;
        Foreach(list, hashable) {
            int code = hashable->get_hash_code();
            int table_index = code % table_size;
            lists[table_index].add(hashable);
        } Endeach;
    }

    delete [] old_lists;
}

Hashable *Hash_Table::remove(Hashable *hashable) {
    // @Speed: If we decide to be able to rely on the fact
    // that the 'hash_code' member of Hashable doesn't get
    // stomped since the time we added it to the table, then
    // we can just read teh hash code out of the struct member
    // here, instead of calling a function to recompute it.
    int code = hashable->get_hash_code();
    int table_index = code % table_size;
    List *l = &lists[table_index];

    Hashable *other;
    Foreach(l, other) {
        if (hashable->compare(other) == 0) {
            Foreach_Remove_Current_Element(l);
            return other;
        }
    } Endeach;

    return NULL;
}

Hashable *Hash_Table::find(Hashable *to_find) {
    int code = to_find->get_hash_code();
    int table_index = code % table_size;

    List *l = &lists[table_index];

    Hashable *other;
    Foreach(l, other) {
        if (to_find->compare(other) == 0) {
            return other;
        }
    } Endeach;

    return NULL;
}


void Hash_Table_Iterator::init(Hash_Table *table) {
    done = false;
    current_list_item_index = -1;
    current_bucket_index = 0;
    next();
}

void Hash_Table_Iterator::next() {
    List *list = &table->lists[current_bucket_index];

    current_list_item_index++;
    if (current_list_item_index >= list->items) {
        current_list_item_index = 0;
        while (++current_bucket_index < table->table_size) {
            list = &table->lists[current_bucket_index];
            if (list->items) break;
        }
    }

    if (current_bucket_index >= table->table_size) {
        done = true;
        hashable = NULL;
        return;
    }

    hashable = (Hashable *)list->nth(current_list_item_index);
}

