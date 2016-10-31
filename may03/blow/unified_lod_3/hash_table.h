#pragma once

#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

const int MAX_OVERCROWDING_FACTOR = 4;

struct List;

struct Hashable {
    int hash_code;
    
    virtual int compare(Hashable *other) = 0;
    virtual int get_hash_code() = 0;
};

struct Hash_Table {
    Hash_Table(int hash_size);
    ~Hash_Table();

    void add(Hashable *hashable);
    Hashable *find(Hashable *hashable);
    Hashable *remove(Hashable *hashable);

    int items;
    int table_size;
    List *lists;

  protected:
    void resize();
};

struct Hash_Table_Iterator {
    void init(Hash_Table *table);
    void next();

    Hash_Table *table;
    int current_bucket_index;
    int current_list_item_index;

    Hashable *hashable;
    bool done;
};


#endif // __HASH_TABLE_H
