#pragma once

#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

const int MAX_OVERCROWDING_FACTOR = 4;

struct String_Hash_Info {
    int code;
    Text_Utf8 *name;
    void *value;
};

struct String_Hash_Table {
    String_Hash_Table(int hash_size);
    ~String_Hash_Table();

    void add(Text_Utf8 *name, void *value);
    bool find(Text_Utf8 *name, void **value_return);
    bool remove(Text_Utf8 *name, void **value_return);

    int items;
    int table_size;
    List *lists;

    void resize();
};

struct Integer_Hash_Info {
    int code;
    unsigned long key;
    void *value;
};

struct Integer_Hash_Table {
    Integer_Hash_Table(int hash_size);
    ~Integer_Hash_Table();

    void add(unsigned long key, void *value);
    bool find(unsigned long key, void **value_return);

    int items;
    int table_size;
    List *lists;

    void resize();
};

#endif // __HASH_TABLE_H
