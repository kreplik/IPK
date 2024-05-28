#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include "converter.h"

// Server's message unique ID
extern uint8_t srv_ID;
#define BUFFER_SIZE 1500

// Temporary buffer
char buffer[BUFFER_SIZE];

void get_udp_msg(struct Shared_msg *shared_msg, char *displayname, char *content, int original,struct UDP_User *udp_user)
{
    srv_ID++;

    // Clear buffers
    bzero(buffer, sizeof(buffer));
    memset(shared_msg->udp_content, 0, sizeof(shared_msg->udp_content));
    memset(shared_msg->tcp_content, 0, sizeof(shared_msg->tcp_content));

    // Construct message to UDP mode
    if (original == TCP)
    {
        buffer[0] = 0x04;
        buffer[1] = (srv_ID >> 8) & 0xFF;
        buffer[2] = srv_ID & 0xFF;

        size_t displayname_l = strlen(displayname);
        memcpy(&buffer[3], displayname, displayname_l);

        size_t content_l = strlen(content);

        memcpy(&buffer[displayname_l + 4], content, content_l);

        shared_msg->content_size = content_l + displayname_l + 5;
        memcpy(shared_msg->udp_content, buffer, shared_msg->content_size);

        sprintf(shared_msg->tcp_content, "MSG FROM %s IS %s\r\n", displayname, content);
    }
    // Construct message to TCP mode
    else{
        memcpy(shared_msg->udp_content,content,udp_user->content_size);
        shared_msg->content_size = udp_user->content_size;
        sprintf(shared_msg->tcp_content,"MSG FROM %s IS %s\r\n",udp_user->displayname,udp_user->user_message);
    }
    return;
}
