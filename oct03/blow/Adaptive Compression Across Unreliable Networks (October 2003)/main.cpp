#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "arithmetic_coder.h"
#include "server.h"
#include "client.h"
#include "message_stats.h"


// transmit_or_lose will either discard a message, or pass it from the
// source to the destination.
void transmit_or_lose(Network_Agent *source, Network_Agent *dest, float loss_probability) {
    if (!source->outgoing_message) return;

    Network_Message *message = source->outgoing_message;
    source->outgoing_message = NULL;


    assert(RAND_MAX <= 0xffff);
    unsigned long roll = rand();  //  * rand();
    unsigned long roll_max = RAND_MAX;  //  * RAND_MAX;

    if (roll < loss_probability * roll_max) {
        delete message;
    } else {
        dest->incoming_message = message;
    }
}


void main(int argc, char *argv[]) {
    // Construct a client and server.
    Client *client = new Client();
    Server *server = new Server();

    while (!server->done) {
        // Transmit data from the server to the client.
        server->update();
        transmit_or_lose(server, client, PACKET_LOSS_PROBABILITY);

        // Transmit data from the client to the server.
        // For now all client-to-server data is assumed to be sent
        // reliably.  There is not much of such traffic here 
        // (just occasional short requests to build a new probability
        // model).  In a real game, it'd be no big deal to send
        // these reliably.  You can also just send them unreliably,
        // but then packet loss will further degrade the effectiveness
        // of the adaptive modeler.
        client->update();
        transmit_or_lose(client, server, 0.0f);
    }

    printf("Server compressed %d bytes down to %d bytes.\n",
           server->num_data_bytes_precompressed, server->num_bytes_transmitted);

    if (client->highest_message_id_received > -1) {
        int packets_lost = client->highest_message_id_received + 1 - client->num_messages_received;
        float packet_loss_ratio = packets_lost / (float)(client->highest_message_id_received + 1);
        printf("%d packets received.  %d max.  Approximate packet loss: %.2f%%", 
               client->num_messages_received, 
               client->highest_message_id_received + 1,
               packet_loss_ratio * 100.0f);
    }
}
