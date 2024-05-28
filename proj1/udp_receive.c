#include <stdio.h>
#include <stdlib.h>
#include "udp_receive.h"
#include "udp.h"
#include "ipk.h"

int parse_message(char *rec_message, struct server_reply *server_rply, struct server_msg *server_message, struct confirm *conf, struct MSG_type *message_type)
{
    // Received REPLY message type
    if (rec_message[0] == message_type->REPLYtype)
    {
        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        server_rply->rec_reply = strdup(&rec_message[6]);
        rec_message[6] = '\0';
        server_rply->referenceID = (rec_message[4] << 8) | rec_message[5];

        rec_message[4] = '\0';
        server_rply->result = strdup(&rec_message[3]);
        rec_message[3] = '\0';
        server_rply->replyID = strdup(&rec_message[2]);
    }

    // Received CONFIRM message
    else if (rec_message[0] == message_type->CONFIRMtype)
    {
        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        conf->replyID = (rec_message[1] << 8) | rec_message[2];
    }

    // Received MSG message type
    else if (rec_message[0] == message_type->MSGtype)
    {
        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        server_message->display_name = strdup(&rec_message[3]);
        server_message->rec_message = strdup(&rec_message[strlen(server_message->display_name) + 4]);
        rec_message[3] = '\0';
        server_rply->replyID = strdup(&rec_message[2]);
    }

    // Received ERR message type
    else if ((uint8_t)rec_message[0] == message_type->ERRtype)
    {
        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        server_message->display_name = strdup(&rec_message[3]);
        server_message->rec_message = strdup(&rec_message[strlen(server_message->display_name) + 4]);
        rec_message[3] = '\0';
        server_rply->replyID = strdup(&rec_message[2]);
    }

    // Return received message type
    return (int)rec_message[0];
}
