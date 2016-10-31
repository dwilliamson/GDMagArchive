#include "../framework.h"

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


void *List::nth(unsigned int i) {
    assert (i < items);

    Listnode *n = head;
    while (i--) {
        n = n->next;
    }

    assert(n != NULL);

    return n->data;
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

