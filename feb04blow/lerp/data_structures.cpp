#include "lerp_os_specific.h"
#include <stdlib.h>
#include <assert.h>
#include "memcpy.h"
#include "data_structures.h"

List::List(void) {
    head = NULL;
    tail = NULL;
    items = 0;
}

List::~List(void) {
    Listnode *n, *m;

    for (n = head; n != NULL; n = m) {
        m = n->next;
        delete n;
    }
}

void List::clean() {
    Listnode *m, *n = head;

    m = n;
    while (n != NULL) {
        n = n->next;
        delete m;
        m = n;
    }

    head = NULL;
    tail = NULL;
    items = 0;
}

void List::add(void *item) {
    Listnode *n;

    n = new Listnode();
    n->next = NULL;
    n->data = item;
    n->prev = tail;

    if (tail == NULL) {
        head = tail = n;
    } else {
        tail->next = n;
        tail = n;
    }

    items++;
}

void *List::peek_at_tail() {
    assert(tail);
    return tail->data;
}

void List::add_at_head(void *item) {
    Listnode *n;

    n = new Listnode();
    n->prev = NULL;
    n->data = item;
    n->next = head;

    if (head == NULL) {
        head = tail = n;
    } else {
        head->prev = n;
        head = n;
    }

    items++;
}

bool List::add_unique(void *item) {
    void *other;
    Foreach(this, other) {
        if (other == item) return false;
    } Endeach;

    add(item);
    return true;
}


void *List::nth(unsigned int i) {
    assert((int)i < items);

    Listnode *n = head;
    while (i--) {
        n = n->next;
    }

    assert(n != NULL);

    return n->data;
}


void *List::remove_tail() {
    assert(tail != NULL);
    assert(items > 0);

    Listnode *n = tail;
    void *retval = n->data;

    if (tail->prev == NULL) {
        head = tail = NULL;
    } else {
        tail = n->prev;
        tail->next = NULL;
    }

    delete n;
    items--;
    return retval;
}

void foreach_remove(List *l, Listnode *n) {
    if (n->prev) {
        n->prev->next = n->next;
    } else {
        l->head = n->next;
    }

    if (n->next) {
        n->next->prev = n->prev;
    } else {
        l->tail = n->prev;
    }

    l->items--;
    delete n;
}




void _auto_array_allocate(void **data_ret, int *items_ret,
                          int start_items, int item_size) {

    int bytes = start_items * item_size;
    void *data = (void *)new char[bytes]; // malloc(bytes);
    *data_ret = data;
    *items_ret = start_items;
}

void _auto_array_deallocate(void **data_ret) {
    delete [] (char *)(*data_ret);
    // delete [] 
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


void *List::remove_head() {
    assert(head != NULL);
    assert(items > 0);

    Listnode *n = head;
    void *retval = n->data;

    if (head->next == NULL) {
        head = tail = NULL;
    } else {
        head = n->next;
        head->prev = NULL;
    }

    delete n;
    items--;
    return retval;
}

//
// List::remove
//
// Remove only the first occurrence of item in self.
//

bool List::remove(void *item) {
    Listnode *n;

    for (n = head; n != NULL; n = n->next) {
        if (n->data == item) {
            if (n->prev == NULL) {
                head = n->next;
            } else {
                n->prev->next = n->next;
            }

            if (n->next == NULL) {
                tail = n->prev;
            } else {
                n->next->prev = n->prev;
            }

            delete n;
            items--;
            return true;
        }
    }

    return false;
}

