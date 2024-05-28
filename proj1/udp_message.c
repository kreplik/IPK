#include <stdio.h>
#include <stdlib.h>
#include "udp.h"
#include "udp_message.h"

char send_message[1500];
extern bool authAlloc;
extern bool joinAlloc;
extern int present_state;

void setup_message(struct Setup_msg *setup_msg, struct udp_auth *auth, struct udp_join *join, struct user_msg *msg, struct MSG_type *message_type)
{

    // Clear buffers
    bzero(send_message, sizeof(send_message));
    int message_length = 0;
    memset(setup_msg->send_message, 0, sizeof(setup_msg->send_message));

    // Setup messages according to the IPK24CHAT PROTOCOL
    switch (setup_msg->command)
    {
    case AUTH:
        // Setup authentication message
        // Setup message according to the IPK24CHAT PROTOCOL
        send_message[0] = message_type->AUTHtype;
        send_message[strlen(send_message)+1] = setup_msg->messageID;

        memcpy(&send_message[3], auth->user_name, strlen(auth->user_name));

        memcpy(&send_message[strlen(auth->user_name) + 4], auth->display_name, strlen(auth->display_name));

        memcpy(&send_message[strlen(auth->user_name) + strlen(auth->display_name) + 5], auth->secret, strlen(auth->secret));
        message_length = strlen(auth->user_name) + strlen(auth->display_name) + strlen(auth->secret) + 5;
        present_state = state_AUTH;
        ////////////////////////////////////////////////////
        break;

    case JOIN:
        // Setup join message
        // Setup message according to the IPK24CHAT PROTOCOL
        send_message[0] = message_type->JOINtype;
        send_message[strlen(send_message)+1] = setup_msg->messageID;

        memcpy(&send_message[3], join->channel, strlen(join->channel));

        memcpy(&send_message[strlen(join->channel) + 4], auth->display_name, strlen(auth->display_name));
        message_length = strlen(join->channel) + strlen(auth->display_name) + 4;
        ////////////////////////////////////////////////////
        break;

    case MSG:
        // Setup user's message
        // Setup message according to the IPK24CHAT PROTOCOL
        send_message[0] = message_type->MSGtype;
        send_message[strlen(send_message) +1] = setup_msg->messageID;

        memcpy(&send_message[3], auth->display_name, strlen(auth->display_name));

        memcpy(&send_message[strlen(auth->display_name) + 4], msg->user_message, strlen(msg->user_message));
        message_length = strlen(auth->display_name) + strlen(msg->user_message) + 4;
        ////////////////////////////////////////////////////
        break;

    case BYE:
        // Setup BYE message
        // Setup message according to the IPK24CHAT PROTOCOL
        send_message[0] = message_type->BYEtype;
        send_message[strlen(send_message)+1] = setup_msg->messageID;
        message_length = 2;
        present_state = state_END;
        ////////////////////////////////////////////////////
    default:
        break;
    }

    // Store message's length
    setup_msg->message_length = message_length;

    // Store message from buffer to the structure
    memcpy(setup_msg->send_message, send_message, setup_msg->message_length);

    return;
}

void send_confirm(char *id, int socket, struct sockaddr *address, socklen_t address_size)
{

    // Clear buffer
    bzero(send_message, sizeof(send_message));

    // Setup CONFIRM message
    send_message[0] = 0x00;
    memcpy(&send_message[2], id, strlen(id));

    // Send this message to server
    sender(3, send_message, socket, address, address_size);
}

void send_bye(int id, int socket, struct sockaddr *address, socklen_t address_size)
{
    // Clear buffer
    bzero(send_message, sizeof(send_message));

    // Setup BYE message
    send_message[0] = 0xFF;
    send_message[strlen(send_message)] = 0x00;
    send_message[2] = id;
    ////////////////////
    // Send this message to server
    sender(3, send_message, socket, address, address_size);
}

void send_error(int id, int socket, struct sockaddr *address, socklen_t address_size,struct MSG_type *message_type, struct udp_auth *auth)
{
    // Clear buffer
    bzero(send_message, sizeof(send_message));

    // Setup ERR message
    //////////////////////
    int message_length = 0;
    char *error_message = "ERR: unknown message type received\n";

    send_message[0] = message_type->ERRtype;
    send_message[strlen(send_message)] = 0x00;
    send_message[2] = id;

    memcpy(&send_message[3], auth->display_name, strlen(auth->display_name));

    memcpy(&send_message[strlen(auth->display_name) + 4],error_message,strlen(error_message));
    message_length = strlen(auth->display_name) + strlen(error_message) + 4;
    ///////////////////////

    // Send this message to server
    sender(message_length+1,send_message,socket,address,address_size);
}

char sender(int size, char *send_message, int socket, struct sockaddr *address, socklen_t address_size)
{
    // Main function for sending messages
    int bytestx = sendto(socket, send_message, size, 0, address, address_size);
    if (bytestx < 0)
    {
        perror("ERR: sendto");
        shutdown(socket, SHUT_RDWR);
        close(socket);
        exit(ERROR);
    }

    // Return sent message's type
    return send_message[0];
}
