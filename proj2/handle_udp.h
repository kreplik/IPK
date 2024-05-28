/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef HANDLE_UDP_H
#define HANDLE_UDP_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "ipk.h"
#include "errors.h"
#include "socket.h"
#include "udp_receiver.h"
#include "converter.h"
#include "udp_setup.h"
#include "tcp_listener.h"


// Message sent by server
typedef struct Server_msg
{
    char content[1500]; // Content of message
    size_t content_size; // Content's size
}Server_msg;

// UDP user's/message parameters
typedef struct UDP_User
{
    uint8_t opcode; // Message opcode
    uint16_t id; // Message ID
    uint16_t unique_id; // Unique ID for each user
    char* username; // User's username
    char* displayname; // User's displayname
    char* channel_name; // User's channel's name
    char* secret; // User's secret
    char* user_message; // User's message
    uint8_t result; // REPLY result
    uint16_t ref_id; // Message's reference ID
    bool authenticated; // User's authentication indicator
    int client_state; // // User's state
    size_t content_size; // User's message content size
    struct Channel *channel; // User's channel
    in_port_t original_port; // User's original port
    in_port_t dynamic_port; // Dynamic port for user
    struct in_addr sin_addr; // User's address
    int dyn_socket; // Dynamic socket for user
}UDP_User;

// Holds UDP users
typedef struct UDP_clients
{
    struct UDP_User *udp_user[100]; // List of UDP users
    int num_clients; // Number of UDP users
}UDP_clients;

// Message type structure
typedef struct MSG_type
{
    uint8_t CONFIRMtype;
    uint8_t REPLYtype;
    uint8_t AUTHtype;
    uint8_t JOINtype;
    uint8_t MSGtype;
    uint8_t ERRtype;
    uint8_t BYEtype;
}MSG_type;


/**
 * @brief Initialize message types structure
 * @param message_type Structure of message types
*/
void initMsg_type(struct MSG_type *message_type);

/**
 * @brief Handles incoming UDP communications
 * @param client_socket Welcome socket for UDP connections
 * @param channel_list Structure for channels
 * @param shared_msg Structure that holds tcp/udp messages
*/
void udp_handler(int client_socket, struct Channel_list *channel_lsit,struct Shared_msg *shared_msg);

/**
 * @brief Sends message to specific UDP client
 * @param buffer Messsage to be sent
 * @param buffer_size Size of message to be sent
 * @param udp_user Client's parameters
*/
void send_to_udp_client(char *buffer, size_t buffer_size, struct UDP_User *udp_user);

/**
 * @brief Sends confirm message to specific client
 * @param udp_user Specific UDP user's parameters
 * @param message_type Message types
*/
void send_confirm(struct UDP_User *udp_user, struct MSG_type *message_type);

/**
 * @brief Sends error message to specific client
 * @param udp_user Specific UDP user's parameters
 * @param message_type Message types
*/
void send_err(struct UDP_User *udp_user, struct MSG_type *message_type);

/**
 * @brief Sends bye message to specific client
 * @param udp_user Specific UDP user's parameters
 * @param message_type Message types
*/
void send_bye(struct UDP_User *udp_user, struct MSG_type *message_type);

/**
 * @brief Removes udp user from structure that holds dynamic ports
 * @param users_socket User's socket
 * @param udp_clients Structure that holds user's dynamic ports
*/
void remove_udp_user(int users_socket,struct UDP_clients *udp_clients);

/**
 * @brief Creates dynamic socket and dynamic port for UDP communication
 * @param client_addr Client's address
 * @param udp_clients List of UDP clients
*/
void dynamic_binder(struct sockaddr_in client_addr, struct UDP_clients *udp_clients);

/**
 * @brief Create new User structure if the client is new
 * @param udp_clients List of UDP clients
 * @param client_addr Client's address
*/
void add_user(struct UDP_clients *udp_clients, struct sockaddr_in client_addr);

/**
 * @brief Get user according to received address
 * @param udp_clients List of UDP client
 * @param client_addr Client's address
 * @return Structure of address to communicate with this client
*/
UDP_User *get_user(struct UDP_clients *udp_clients, struct sockaddr_in client_addr);

#endif
