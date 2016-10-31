#include "network_agent.h"
#include <stdio.h>  // Because C is lame in that 'FILE' is a macro so we can't forward-declare it.


// The Server transmits a lot of data messages to the Client.
// When the Client asks for new data models to be synthesized,
// the Server happily complies with this request, as it helps
// reduce the server's bandwidth usage.
struct Server : public Network_Agent {
    Server();
    ~Server();

    FILE *file_to_transmit;
    bool done;

    int num_bytes_transmitted;
    int num_data_bytes_precompressed;
    int current_model_index;

    void update();

  protected:
    void handle_new_model_request(Network_Message *message);
};

