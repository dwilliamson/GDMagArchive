#include "../framework.h"

void _auto_array_allocate(void **data_ret, int *items_ret,
                          int start_items, int item_size) {
    int bytes = start_items * item_size;
    void *data = (void *)new char[bytes];
    *data_ret = data;
    *items_ret = start_items;
}

void _auto_array_deallocate(void **data_ret) {
    delete [] (char *)(*data_ret);
}

void _auto_array_expand(void **data_ret, int *items_ret,
                        int item_size) {
    int old_items = *items_ret;
    int new_items = 2 * old_items;
    int old_bytes = old_items * item_size;
    int new_bytes = new_items * item_size;

    void *old_data = *data_ret;
    void *new_data = (void *)new char[new_bytes];

    memcpy(new_data, old_data, old_bytes);
    delete [] (char *)old_data;

    *data_ret = new_data;
    *items_ret = new_items;
}

