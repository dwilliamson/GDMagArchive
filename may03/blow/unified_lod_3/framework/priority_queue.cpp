#include "../framework.h"
#include "priority_queue.h"
#include <math.h>

Priority_Queue::Priority_Queue(int num_expected_items, double _search_log_factor) {
    assert(num_expected_items > 0);

    search_log_factor = _search_log_factor;

    max_node_height = (int)(log(num_expected_items) / log(search_log_factor)) + 1;
    assert(max_node_height > 0);

    float population = 100;

    int i;
    for (i = 0; i < max_node_height; i++) {
        int size_in_bytes = sizeof(Priority_Queue_Node) + i * sizeof(Priority_Queue_Node *);

        population /= search_log_factor;
        if (population < 20) population = 20; // @Cleanup arbitrary constant.
    }

    init_sentinel();
    num_items = 0;
}

Priority_Queue::~Priority_Queue() {
}

void Priority_Queue::init_sentinel() {
    // Set up the sentinel node.

    sentinel = make_node(max_node_height);
    sentinel->priority = -1;
    sentinel->data = NULL;

    int i;
    for (i = 0; i < max_node_height; i++) sentinel->next[i] = NULL;
}

void Priority_Queue::reset() {
    num_items = 0;

    init_sentinel();
}

Priority_Queue_Node *Priority_Queue::make_node(int height) {
    assert(height >= 1);
    assert(height <= max_node_height);

    Priority_Queue_Node *node = (Priority_Queue_Node *)(new char[sizeof(Priority_Queue_Node) + (height - 1) * sizeof(Priority_Queue_Node *)]);

    return node;
}

Priority_Queue_Node *Priority_Queue::make_node(int *height_result) {
    int height = 1;

    const int RAND_QUANTUM = 1 << 24;
    double factor = 1;

    // @Warning: 'die_roll' uses rand() which is a no-no for
    // serious software development.
    int die_roll = rand() % RAND_QUANTUM;

    // @Improvement we can probably compute the below thingy
    // in closed form... why don't we do that?
    while (height < max_node_height) {
        factor *= (1.0 / search_log_factor);
        int number_to_beat = (int)(factor * RAND_QUANTUM);
        
        if (die_roll >= number_to_beat) break;

        height++;
    }

    assert(height <= max_node_height);

    *height_result = height;
    return make_node(height);
}

void Priority_Queue::add(float priority, void *data) {
    int node_height;
    Priority_Queue_Node *node = make_node(&node_height);

    node->priority = priority;
    node->data = data;

    Priority_Queue_Node *cursor = sentinel;
    int i;
    for (i = max_node_height - 1; i >= 0; i--) {
        while (1) {
            Priority_Queue_Node *next = cursor->next[i];
            if (next == NULL) break;
            if (next->priority > node->priority) break;
            cursor = next;
        }
        
        if (i < node_height) {
            node->next[i] = cursor->next[i];
            cursor->next[i] = node;
        }
    }

    num_items++;
}

void *Priority_Queue::remove_head(float *priority_result,
                                  bool *success_result) {
    Priority_Queue_Node *first_node = sentinel->next[0];
    if (!first_node) {
        *success_result = false;
        assert(num_items == 0);
        return NULL;
    }

    // Unlink the head from the list.

    assert(num_items > 0);

    int i;
    for (i = 0; i < max_node_height; i++) {
        if (sentinel->next[i] == first_node) {
            sentinel->next[i] = first_node->next[i];
        }
    }

    *priority_result = first_node->priority;
    *success_result = true;
    void *return_value = first_node->data;

    delete [] ((char *)first_node);
    num_items--;

    return return_value;
}
