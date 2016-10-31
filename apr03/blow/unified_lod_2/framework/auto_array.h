#pragma once

#ifndef _AUTO_ARRAY_H
#  define _AUTO_ARRAY_H

// Auto_Array:
//
// I am deviating from my "No Templates!!!" rule here because
// as nearly as I can see there is no other good way to do a
// simple auto-resizing array.  To prevent code bloat, all
// methods in the Auto_Array struct are extremely small and
// always inlined.  All operations that don't have to be super fast
// (like memory allocation, resizing, and copying) are callouts
// to non-struct functions (that do not polymorph, so they will
// not cause code bloat).

const int AUTO_ARRAY_START_SIZE = 32;
void _auto_array_allocate(void **data_ret, int *items_ret,
                          int start_items, int item_size);

void _auto_array_expand(void **data_ret, int *allocated_items_ret,
                        int item_size);
void _auto_array_deallocate(void **data_ret);
//void _auto_array_insert_space_at(void **data_ret);


template <class T> struct Auto_Array {
  public:
    Auto_Array(int start_size = AUTO_ARRAY_START_SIZE);
    ~Auto_Array();

    int live_items;
    int allocated_items;

    T *data;

    void add(T item);
    void insert_at(int index, T item);
    int count();
    void reset();
    bool remove(T item);   // Will reorder array if you use this!
    T remove_nth(int index);

    T &operator [](const int i) const;
};




template <class T> 
inline Auto_Array <T>::Auto_Array(int start_size) {
    live_items = 0;
    _auto_array_allocate((void **)&data, &allocated_items,
                         start_size, sizeof(T));
    allocated_items = start_size;
}

template <class T> 
inline Auto_Array <T>::~Auto_Array() {
    _auto_array_deallocate((void **)&data);
}

template <class T>
inline int Auto_Array <T>::count() {
    return live_items;
}

template <class T>
inline void Auto_Array <T>::reset() {
    live_items = 0;
}

template <class T> 
inline T &Auto_Array <T>::operator [] (const int i) const {
    assert(i >= 0);
    assert(i < live_items);

    return data[i];
}

template <class T>
inline void Auto_Array <T>::add(T item) {
    if (live_items == allocated_items) {
        _auto_array_expand((void **)&data, &allocated_items, sizeof(T));
    }

    data[live_items++] = item;
}

template <class T>
inline bool Auto_Array <T>::remove(T item) {
    int i;
    for (i = 0; i < live_items; i++) {
        if (data[i] == item) {
            data[i] = data[live_items - 1];
            live_items--;
            return true;
        }
    }

    return false;
}

template <class T>
inline T Auto_Array <T>::remove_nth(int index) {
    T result = data[index];
    data[index] = data[live_items - 1];
    live_items--;

    return result;
}

template <class T>
inline void Auto_Array <T>::insert_at(int index, T item) {
    assert(index <= live_items);

    if (live_items == allocated_items) {
        _auto_array_expand((void **)&data, &allocated_items, sizeof(T));
    }

    int i;
    for (i = live_items; i > index; i--) data[i] = data[i - 1];
    data[index] = item;
    live_items++;
}

#define Array_Foreach(_l, _x){    		\
    int __i ; \
    for (__i = 0; __i < (_l)->live_items; __i++) { \
        (_x) = (_l)->data[__i];

#endif // _AUTO_ARRAY_H


