/*

  List: a singularly-linked list.  The point of this class is to be
    simple and easy to use; it is not necessarily supposed to be efficient.
    Since it allocates and deallocates memory explicitly (via 'new') when 
    you add and remove items, you don't want to use it in cases where
    you need high performance and many list operations are necessary.
   
 */

#ifndef __DS_H
#  define __DS_H


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

    // 'add_at_head' prepends an item to the start of the list.
    void add_at_head(void *item);

    // 'add_unique' adds an item to the list if it is not already
    // present.  It returns true if it actually added the item, or
    // false if it found a duplicate.
    bool add_unique(void *item);

    // 'insert_behind' searches for the value 'ipoint' in the list;
    // if it finds it, it adds 'item' into the list right behind 'ipoint',
    // and returns true.  Otherwise, it returns false and does not
    // insert anything.
    bool insert_behind(void *ipoint, void *item);

    // 'remove' searches for an item in the list.  If the item is
    // found, it is taken out of the list, and the function returns
    // true.  Otherwise, nothing is removed, and the function returns false.
    bool remove(void *item);

    // 'find' searches for an item in the list.  If found, it will store
    // the index of the item in 'pos_ret' (if pos_ret is not NULL).
    // Then the function returns true.  If the item is not found,
    // the function returns false, and pos_ret is not used.
    bool find(void *item, int *pos_ret = NULL);

    // Calling 'remove_head', 'remove_tail', 'peek_at_head', 
    // or 'peek_at_tail' on the empty list is an error.

    // 'remove_head' takes the first item out of the list and returns it.
    void *remove_head();
    // 'remove_tail' takes the last item out of the list and returns it.
    void *remove_tail();

    // 'peek_at_head' returns the first item without removing it.
    void *peek_at_head();
    // 'peek_at_tail' returns the last item without removing it.
    void *peek_at_tail();

    // 'nth' returns to you the item in position n of the list.
    // If there are less than n+1 items in the list, that is an error.
    void *nth(unsigned int n);

    // 'remove_nth' takes the item in position n out of the list and
    // returns it.  If there aren't enough items in the list to satisfy
    // the request, it returns NULL.  If the argument 'ok' is not NULL,
    // then the function uses it to tell you whether there was enough
    // room in the list or not: (*ok) is true if an item was removed
    // and returned, and false if not.
    void *remove_nth(unsigned int n, bool *ok = NULL);

    // 'total_items' tells you how many items are in the list.
    unsigned int total_items();


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

#endif // !__DS_H
