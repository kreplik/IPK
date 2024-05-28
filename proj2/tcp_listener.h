/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef LISTENER_H
#define LISTENER_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include "ipk.h"
#include "errors.h"
#include "socket.h"
#include "handle_tcp.h"

#define BUFFER_SIZE 1500


// TCP client structure
typedef struct Client_message
{
    char* username; // Client's username
    char* displayname; // Client's displayname
    char message[BUFFER_SIZE]; // Client's message
    char* secret; // Client's secret
    bool authenticated; // Authentication indicator
    int client_state; // Client's state
    int message_type; // Received message type
    char* channel; // Client's channel
}Client_message;

// Structure for passing parameters to different TCP client thread
struct client_handler_args {
    int client_socket;
    struct sockaddr_in client_addr;
    struct Client_message *client;
    struct Channel_list *channel_list;
    struct Shared_msg *shared_msg;
};

/**
 * @brief Listens for incoming TCP communication
 * @param welcome_socket_tcp Server's TCP welcome socket
 * @param channel_list Structure of channels
 * @param shared_msg Structure that holds tcp/udp messages
*/
void listener(int welcome_socket_tcp, struct Channel_list *channel_list,struct Shared_msg *shared_msg);

/**
 * @brief Accepts incoming TCP connection and creates unique thread for each client
 * @param welcome_socket_tcp Server's TCP welcome socket
 * @param channel_list Structure of channels
 * @param shared_msg Structure that holds tcp/udp messages
*/
void accept_connection(int welcome_socket_tcp, struct Channel_list *channel_list,struct Shared_msg *shared_msg);

/**
 * @brief Creates Client's parameters structure
 * @param client_socket Specific client's socket
 * @return New allocated structure for specific client's parametes
*/
Client_message* alloc_new_user(int client_socket);

/**
 * @brief Specific thread function for each client
 * @param arg Passing parameters to thread function
*/
void *client_handler(void *arg);

#endif
