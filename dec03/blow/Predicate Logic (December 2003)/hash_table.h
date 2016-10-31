#pragma once

#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

const int MAX_OVERCROWDING_FACTOR = 4;

struct Typed_Hash_Info {
    int code;
    char *name;
    void *value;
};

struct String_Hash_Table {
    String_Hash_Table(int hash_size);
    ~String_Hash_Table();

    void add(char *name, void *value);
    bool find(char *name, void **value_return);
    bool remove(char *name, void **value_return);

    int items;
    int table_size;
    List *lists;

    void resize();
};

#endif // __HASH_TABLE_H
