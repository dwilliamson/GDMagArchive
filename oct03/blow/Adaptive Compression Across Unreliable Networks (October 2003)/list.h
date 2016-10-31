/*

  List: a singularly-linked list.  The point of this class is to be
    simple and easy to use; it is not necessarily supposed to be efficient.
    Since it allocates and deallocates memory explicitly (via 'new') when 
    you add and remove items, you don't want to use it in cases where
    you need high performance and many list operations are necessary.
   
 */

#ifndef __LIST_H
#  define __LIST_H


// The Listnode is a helper class for List.
struct Listnode {
    struct Listnode *next;
    struct Listnode *prev;
    void *data;
};

struct List {
    List();
    ~List();

    // 'init' initializes the state of the List.  (But it is not
    // an acceptable way to clear a list that has been used!)  You
    // can use 'init' to initialize a List that's in a memory 
    // situation where its constructor will not have been called.
    void init();

    // 'clean' frees the Listnodes that were allocated by a list
    // that was in use.  However it will not attempt to delete the
    // data items pointed to by any of the Listnodes (because it does
    // not know how you allocated those!)  So be sure to manually
    // free any allocated data that you stored in the list, prior
    // to calling clean().
    void clean();

    // 'add' appends an item to the tail of the list.
    void add(void *item);

    // 'remove' searches for an item in the list.  If the item is
    // found, it is taken out of the list, and the function returns
    // true.  Otherwise, nothing is removed, and the function returns false.
    bool remove(void *item);

    // 'remove_head' takes the first item out of the list and returns it.
    void *remove_head();

    // 'peek_at_head' returns the first item without removing it.
    void *peek_at_head();

    // Data items that aren't necessarily private because you might
    // want to monkey with them.  But unless you are doing something
    // special you shouldn't manipulate these directly.
    int items;

    Listnode *head;
    Listnode *tail;
};


// Macros that help you iterate over a List in nice shorthand.

#define Foreach(_l, _x){    		\
    Listnode *__m, *__n = (_l)->head;		\
    void **foozle;				\
    for (; __n != NULL; __n = __m) {		\
        __m = __n->next;			\
        foozle = (void **)&_x;			\
        *foozle = __n->data;

#define Endeach	}}

void foreach_remove(List *l, Listnode *n);

#define Foreach_Remove_Current_Element(l) foreach_remove(l, __n)

#endif // !__LIST_H
