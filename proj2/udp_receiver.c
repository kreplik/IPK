#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "udp_receiver.h"


int parse_messageUDP(char *rec_message, struct UDP_User *udp_user  ,struct MSG_type *message_type, struct sockaddr_in client_address)
{
    char *msg_opcode = "DEFAULT";
    
    if((uint8_t)rec_message[0] == message_type->AUTHtype)
    {
        if(udp_user->client_state != state_ACCEPT)
        {
            fprintf(stderr,"User already authenticated.\n");
            return ERROR_STATE;
        }

        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        msg_opcode = "AUTH";
        udp_user->id = (uint16_t)(rec_message[1] << 8) | (uint16_t)rec_message[2];

        udp_user->username = strdup(&rec_message[3]);
        size_t username_l = strlen(udp_user->username);

        udp_user->displayname = strdup(&rec_message[username_l + 4]);
        size_t displayname_l = strlen(udp_user->displayname);

        udp_user->secret = strdup(&rec_message[username_l + displayname_l + 5]);
        udp_user->client_state = state_OPEN;
        udp_user->authenticated = true;
    }

    // Received JOIN message type
    else if((uint8_t)rec_message[0] == message_type->JOINtype)
    {
        if(udp_user->client_state != state_OPEN)
        {
            fprintf(stderr,"Cannot join channel now.\n");
            return ERROR_STATE;
        }

        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        msg_opcode = "JOIN";
        udp_user->id = (uint16_t)(rec_message[1] << 8) | (uint16_t)rec_message[2];

        udp_user->channel_name = strdup(&rec_message[3]);
        size_t channel_l = strlen(udp_user->channel_name);

        udp_user->displayname = strdup(&rec_message[channel_l + 4]);

    }

    // Received MSG message type
    else if ((uint8_t)rec_message[0] == message_type->MSGtype)
    {
        if(udp_user->client_state != state_OPEN)
        {
            fprintf(stderr,"Cannot send messages now.\n");
            return ERROR_STATE;
        }
        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        msg_opcode = "MSG";
        udp_user->id = (uint16_t)(rec_message[1] << 8) | (uint16_t)rec_message[2];

        udp_user->displayname= strdup(&rec_message[3]);
        size_t displayname_l = strlen(udp_user->displayname);

        udp_user->user_message = strdup(&rec_message[displayname_l + 4]);
        size_t user_message_l = strlen(udp_user->user_message);

        udp_user->content_size = displayname_l + user_message_l + 5;
        
    }

    // Received CONFIRM message
    else if ((uint8_t)rec_message[0] == message_type->CONFIRMtype)
    {
        msg_opcode = "CONFIRM";
        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        udp_user->id = (uint16_t)(rec_message[1] << 8) | (uint16_t)rec_message[2];
    }

    // Received BYE message type
    else if ((uint8_t)rec_message[0] == message_type->BYEtype)
    {
        // Store message's parameters according to the IPK24CHAT PROTOCOL into appropriate structure
        msg_opcode = "BYE";
        udp_user->id = (uint16_t)(rec_message[1] << 8) | (uint16_t)rec_message[2];
    }
    else if((uint8_t)rec_message[0] == message_type->ERRtype)
    {
        msg_opcode = "ERR";
    }
    else{
        return 444;
    }

    printf("RECV %s:%d | %s\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), msg_opcode);
    
    // Return received message type
    return (int)rec_message[0];
}
