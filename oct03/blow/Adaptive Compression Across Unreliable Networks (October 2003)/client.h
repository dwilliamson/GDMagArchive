#include "network_agent.h"

// The Client receives the data being transmitted by the Server,
// and occasionally sends messages back to the Server asking
// for updated data models, which are used to increase the
// effectiveness of the compression.
struct Client : public Network_Agent {
    Client();
    ~Client();

    void update();

    int last_requested_model_index;
    int num_messages_received;
    int highest_message_id_received;

  protected:
    void send_new_model_request(int old_index, int new_index, int block_index);
    int get_unused_model_index();
};

