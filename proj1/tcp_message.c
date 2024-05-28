#include <stdio.h>
#include <stdlib.h>
#include "tcp.h"
#include "tcp_message.h"


void setup_msgT(int command, struct Setup_tcp_msg *setup_msg, struct tcp_auth *auth, struct tcp_join *join, struct tcp_msg *msg)
{
    // Clear messages buffers
    char send_message[1500];
    bzero(send_message, sizeof(send_message));
    memset(setup_msg->message, 0, sizeof(setup_msg->message));

    // Setup message in demanded way based on command value
    switch (command)
    {
    case AUTH:
        sprintf(send_message, "AUTH %s AS %s USING %s\r\n", auth->user_name, auth->display_name, auth->secret);
        break;

    case JOIN:
        sprintf(send_message, "JOIN %s AS %s\r\n", join->channel, auth->display_name);
        break;

    case MSG:
        sprintf(send_message, "MSG FROM %s IS %s\r\n", auth->display_name, msg->user_message);
        break;

    case BYE:
        sprintf(send_message, "%s\r\n", msg->user_message);
        break;

    default:
        break;
    }

    // Store the message to the structure
    memcpy(setup_msg->message, send_message, strlen(send_message));
}

void send_byeTCP(int socket)
{
    char message[1500];
    strcpy(message,"BYE\r\n");

    // Send BYE message
    int bytes_tx = send(socket, message, strlen(message), 0);
    if (bytes_tx < 0)
    {
        perror("ERR: send");
        shutdown(socket, SHUT_RDWR);
        close(socket);
    }
}

void send_errorTCP(int socket,tcp_auth *auth)
{
    // Setup ERROR message
    char message[1500];
    sprintf(message, "ERR FROM %s IS invalid message type\r\n", auth->display_name);

    // Send error message
    int bytes_tx = send(socket, message, strlen(message), 0);
    if (bytes_tx < 0)
    {
        perror("ERR: send");
        shutdown(socket, SHUT_RDWR);
        close(socket);
    }
}
