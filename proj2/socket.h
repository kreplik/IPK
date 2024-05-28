/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef SOCKET_H
#define SOCKET_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include "ipk.h"
#include "errors.h"
#include "tcp_listener.h"
#include "handle_udp.h"

#define BUFFER_SIZE 1500

// Holds client's address parameters
typedef struct Clients_address
{
    int socket;
    struct sockaddr *udp_address;
    socklen_t udp_address_len;
    bool udp_client; // Client uses UDP protocol
}Clients_address;

// Concrete channel
typedef struct Channel
{
    char* name; // Channel's name
    int num_users; // Channel's number of users
    struct Clients_address address[100]; // Users in this channel
    pthread_mutex_t num_users_mutex;
    pthread_mutex_t clients_mutex;
}Channel;


// List of all channels created
typedef struct Channel_list{
    struct Channel *channels[100]; // All channels
    int num_of_channels; // Number of channels
    pthread_mutex_t num_of_channel_mutex;
    pthread_mutex_t channels_mutex;
}Channel_list;

// Structure for passing parameters to different UDP thread
typedef struct UDP_args
{
    int udp_socket;
    struct Channel_list *channel_list;
    struct Shared_msg *shared_msg;
}UDP_args;

// Holds shared message in both variants (TCP/UDP)
typedef struct Shared_msg
{
    char udp_content[BUFFER_SIZE];
    char tcp_content[BUFFER_SIZE];
    size_t content_size;
}Shared_msg;

/**
 * @brief Creates default server's sockets for both variants
*/
void create_socket(void);

/**
 * @brief Binds socket
 * @param socket Socket to be binded
 * @param address Server's address
 * @param addrlen Address size
*/
void bind_socket(int socket, struct sockaddr *address, socklen_t addrlen);

/**
 * @brief Creates new thread for TCP and UDP communication
 * @param welcome_socket_tcp TCP welcome socket
 * @param udp_socket UDP default socket
*/
void concrete_connection(int welcome_socket_tcp, int udp_socket);

/**
 * @brief Free allocated memory of every channel
 * @param channel_list
*/
void free_channels(struct Channel_list *channel_list);

/**
 * @brief Free allocated memory of every user in the channel
 * @param channel Concrete channel
*/
void destroy_channel(struct Channel *channel);

/**
 * @brief Initialize channel's mutex
 * @param channel Concrete channel
*/
void init_channel(struct Channel *channel);

/**
 * @brief Initialize channel list's mutex
 * @param channel_list List of channels
*/
void init_channel_list(struct Channel_list *channel_list);

/**
 * @brief Destroy channel list's mutex
 * @param channel_list List of channels
*/
void destroy_channel_list(struct Channel_list *channel_list);

/**
 * @brief Removes client from concrete channel
 * @param channel Concrete channel
 * @param client_socket Client's socket
*/
void remove_client_from_channel(Channel *channel, int client_socket);

/**
 * @brief Add client to concrete channel
 * @param channel Concrete channel
 * @param client_socket Client's socket
 * @param udp_user Client's parameters
 * @param mode Mode that indicates if client is TCP or UDP
*/
void add_client(Channel *channel, int client_socket, struct UDP_User *udp_user, int mode);

/**
 * @brief Creates channel and adds it to the list of channels
 * @param channel_list List of channels
 * @param channel_name Name of new channel
 * @return New channel
*/
Channel *create_channel(struct Channel_list *channel_list, char* channel_name);

/**
 * @brief Forwards message to all clients in concrete channel
 * @param channel Concrete channel
 * @param client_socket Sender's socket
 * @param shared_msg Message in both variants
 * @param buffer_size Size of message
*/
void send_all(Channel *channel, int client_socket, struct Shared_msg *shared_msg,size_t buffer_size);

#endif
