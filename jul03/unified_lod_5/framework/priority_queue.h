struct Random_Generator;

struct Priority_Queue_Node {
    float priority;
    void *data;
    Priority_Queue_Node *next[1];
};

/*
  If you want reasonably random behavior, you should be sure to seed
  the Random_Generator member of Priority_Queue before you start
  adding items.
*/

struct Priority_Queue {
    Priority_Queue(int num_expected_items, double search_log_factor = 2.7);
    ~Priority_Queue();

    void add(float priority, void *data);
    void *remove_head(float *priority_return, bool *found_return);
    void reset();

    int num_items;
    Priority_Queue_Node *sentinel;

    Random_Generator *random_generator;

    int max_node_height;
    double search_log_factor;

  protected:
    void init_sentinel();

    Priority_Queue_Node *make_node(int *node_height_result);
    Priority_Queue_Node *make_node(int node_height);
};
