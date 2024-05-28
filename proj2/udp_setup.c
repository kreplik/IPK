#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include "udp_setup.h"

#define BUFFER_SIZE 1500


void setup_message(struct Server_msg *server_msg, struct UDP_User *udp_user, struct MSG_type *message_type, int command, char* reply)
{
    char buffer[BUFFER_SIZE];
    // Clear buffers
    bzero(buffer,sizeof(buffer));
    memset(server_msg->content,0,sizeof(server_msg->content));

    // Setup REPLY message with content
    switch (command)
    {
    case REPLY_OK:

        buffer[0] = message_type->REPLYtype;

        buffer[1] = (udp_user->unique_id >> 8) & 0xFF;
        buffer[2] = udp_user->unique_id & 0xFF;
        udp_user->unique_id++;

        buffer[3] = 1;
        
        buffer[4] = (udp_user->id >> 8) & 0xFF;
        buffer[5] = udp_user->id & 0xFF;
        
        memcpy(&buffer[6],reply, strlen(reply));
        buffer[strlen(reply) + 6] = '\0';
        
        server_msg->content_size = strlen(reply) + 7;
        memcpy(server_msg->content,buffer,server_msg->content_size);        
        
        break;
    
    default:
  
        break;
    }
    
    return;
}
