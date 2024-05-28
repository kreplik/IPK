#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include "socket.h"
#include <pthread.h>

extern Params *params;

void create_socket(void)
{
    // Create TCP socket
    int family = AF_INET;
    int type = SOCK_STREAM;
    int welcome_socket_tcp = socket(family, type, 0);
    if (welcome_socket_tcp <= 0)
    {
        perror("ERROR: socket");
        exit(ERROR);
    }
    int enable = 1;
    setsockopt(welcome_socket_tcp, SOL_SOCKET, SO_REUSEADDR,
               &enable, sizeof(enable));

    // Create UDP socket
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket <= 0)
    {
        perror("ERROR: socket");
        exit(ERROR);
    }

    // Get server's hostname
    struct in_addr ip_addr;
    if (inet_pton(AF_INET, params->hostname, &ip_addr) != 1)
    {
        perror("inet_pton");
        exit(ERROR);
    }

    struct sockaddr_in server_addr;
    // Set server's address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = ip_addr;
    server_addr.sin_port = htons(params->port);

    // Bind TCP sockets
    bind_socket(welcome_socket_tcp, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Bind UDP sockets
    bind_socket(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Create thread for each variant
    concrete_connection(welcome_socket_tcp, udp_socket);
}

void bind_socket(int socket, struct sockaddr *address, socklen_t addrlen)
{
    // Bind this socket to this address
    if (bind(socket, address, addrlen) == -1)
    {
        perror("Error binding socket");
        exit(ERROR);
    }
    return;
}

void *udp_thread(void *arguments)
{
    // Pass parameters for UDP communication
    struct UDP_args *args = (struct UDP_args *)arguments;
    int udp_socket = args->udp_socket;
    struct Channel_list *channel_list = args->channel_list;
    struct Shared_msg *shared_msg = args->shared_msg;

    // Start receiving UDP messages
    udp_handler(udp_socket, channel_list, shared_msg);

    free(args);
    pthread_exit(NULL);

    return NULL;
}

void concrete_connection(int welcome_socket_tcp, int udp_socket)
{
    // Allocate important structures
    Clients_address *clients_address = malloc(sizeof(Clients_address));
    if (clients_address == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    Shared_msg *shared_msg = malloc(sizeof(Shared_msg));
    if (shared_msg == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    Channel *channel = malloc(sizeof(Channel));
    if (channel == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    Channel_list *channel_list = malloc(sizeof(Channel_list));
    if (channel_list == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    // Set parameters of default channel
    channel->num_users = 0;
    channel->name = "General";

    // Init default channel's mutex
    init_channel(channel);

    // Add defualt channel to channel list
    channel_list->num_of_channels = 0;
    channel_list->channels[channel_list->num_of_channels++] = channel;

    // Initialize channel list mutex
    init_channel_list(channel_list);

    UDP_args *udp_args = malloc(sizeof(UDP_args));
    if (udp_args == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    udp_args->udp_socket = udp_socket;
    udp_args->channel_list = channel_list;
    udp_args->shared_msg = shared_msg;

    pthread_t udp_tid;
    // Thread for UDP connections
    if (pthread_create(&udp_tid, NULL, udp_thread, udp_args) != 0)
    {
        perror("pthread_create");
        free(udp_args);
        close(udp_socket);

        exit(ERROR);
    }

    // TCP connections
    listener(welcome_socket_tcp, channel_list, shared_msg);

    // Wait for UDP thread to exit
    pthread_join(udp_tid, NULL);

    free_channels(channel_list);
    free(shared_msg);
}

void free_channels(struct Channel_list *channel_list)
{
    // Free all channels in channel list
    for (int i = 0; i < channel_list->num_of_channels; i++)
    {
        //Destroy channel's mutex
        destroy_channel(channel_list->channels[i]);
        for (int j = 0; j < channel_list->channels[i]->num_users; j++)
        {
            // Close user's socket and free user's addres in this channel
            close(channel_list->channels[i]->address[j].socket);
            free(channel_list->channels[i]->address[j].udp_address);
        }
        free(channel_list->channels[i]);
    }

    // Destroy channel list mutex
    destroy_channel_list(channel_list);
    free(channel_list);
}

void init_channel(struct Channel *channel)
{
    pthread_mutex_init(&(channel->num_users_mutex), NULL);
    pthread_mutex_init(&(channel->clients_mutex), NULL);
}

void destroy_channel(struct Channel *channel)
{
    pthread_mutex_destroy(&(channel->num_users_mutex));
    pthread_mutex_destroy(&(channel->clients_mutex));
}

void init_channel_list(struct Channel_list *channel_list)
{
    pthread_mutex_init(&(channel_list->num_of_channel_mutex), NULL);
    pthread_mutex_init(&(channel_list->channels_mutex), NULL);
}

void destroy_channel_list(struct Channel_list *channel_list)
{
    pthread_mutex_destroy(&(channel_list->num_of_channel_mutex));
    pthread_mutex_destroy(&(channel_list->channels_mutex));
}

void add_client(Channel *channel, int client_socket, struct UDP_User *udp_user, int mode)
{
    pthread_mutex_lock(&(channel->clients_mutex));
    pthread_mutex_lock(&(channel->num_users_mutex));

    // Check is user isnt already in the channel
    for (int i = 0; i < channel->num_users; i++)
    {
        // User is in this channel
        if (channel->address[i].socket == client_socket)
        {
            pthread_mutex_unlock(&(channel->num_users_mutex));
            pthread_mutex_unlock(&(channel->clients_mutex));
            return;
        }
    }

    if (channel->num_users < 100)
    {
        // Add UDP client to channel
        if (mode == UDP)
        {
            struct sockaddr_in *dynamic_client_addr = malloc(sizeof(struct sockaddr_in));
            if (dynamic_client_addr == NULL)
            {
                pthread_mutex_unlock(&(channel->num_users_mutex));
                pthread_mutex_unlock(&(channel->clients_mutex));
                return;
            }

            // Set dynamic address to communicate with this client
            dynamic_client_addr->sin_family = AF_INET;
            dynamic_client_addr->sin_addr = udp_user->sin_addr;
            dynamic_client_addr->sin_port = udp_user->original_port;

            socklen_t addr_size = sizeof(dynamic_client_addr);

            // Store users's address in the channel
            channel->address[channel->num_users].udp_client = true;
            channel->address[channel->num_users].udp_address = (struct sockaddr *)dynamic_client_addr;
            channel->address[channel->num_users].udp_address_len = addr_size;
        }

        // Store TCP and UDP user's socket in the channel
        channel->address[channel->num_users].socket = client_socket;

        // Increment number of users in channel
        channel->num_users++;
    }

    pthread_mutex_unlock(&(channel->num_users_mutex));
    pthread_mutex_unlock(&(channel->clients_mutex));
}

// Remove a client from the channel
void remove_client_from_channel(Channel *channel, int client_socket)
{
    pthread_mutex_lock(&(channel->clients_mutex));
    pthread_mutex_lock(&(channel->num_users_mutex));

    // Iterate over all users in channel
    for (int i = 0; i < channel->num_users; i++)
    {
        // Found user's address index
        if ((channel->address[i]).socket == client_socket)
        {
            // Free user's address 
            if (channel->address[i].udp_client)
            {
                free(channel->address[i].udp_address);
            }

            // Remove user's address from list
            for (int j = i; j < channel->num_users - 1; j++)
            {
                channel->address[j] = channel->address[j + 1];
            }
            channel->num_users--;
            break;
        }
    }

    pthread_mutex_unlock(&(channel->num_users_mutex));
    pthread_mutex_unlock(&(channel->clients_mutex));
}

Channel *create_channel(struct Channel_list *channel_list, char *channel_name)
{
    pthread_mutex_lock(&(channel_list->channels_mutex));
    pthread_mutex_lock(&(channel_list->num_of_channel_mutex));

    // Check if channel already exists
    for (int i = 0; i < channel_list->num_of_channels; i++)
    {
        // Channel already exists
        if (strcmp(channel_list->channels[i]->name, channel_name) == 0)
        {
            pthread_mutex_unlock(&(channel_list->num_of_channel_mutex));
            pthread_mutex_unlock(&(channel_list->channels_mutex));

            // Return this channel
            return channel_list->channels[i];
        }
    }

    Channel *channel = malloc(sizeof(Channel));
    if (channel == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    // Set channel's name and default number of users
    channel->num_users = 0;
    channel->name = channel_name;
    init_channel(channel); // Init mutex

    // Add new channel to channel list
    channel_list->channels[channel_list->num_of_channels++] = channel;

    pthread_mutex_unlock(&(channel_list->num_of_channel_mutex));
    pthread_mutex_unlock(&(channel_list->channels_mutex));

    return channel;
}

void send_all(Channel *channel, int client_socket, struct Shared_msg *shared_msg, size_t buffer_size)
{
    pthread_mutex_lock(&(channel->clients_mutex));
    pthread_mutex_lock(&(channel->num_users_mutex));

    // Iterate over all user in channel
    for (int i = 0; i < channel->num_users; i++)
    {
        // Forward message to all users except original sender
        if (channel->address[i].socket != client_socket)
        {
            // Send to UDP client
            if (channel->address[i].udp_client)
            {
                if (sendto(channel->address[i].socket, shared_msg->udp_content, buffer_size, 0, channel->address[i].udp_address, channel->address[i].udp_address_len) == -1)
                {
                    perror("Error sending data to UDP client");
                    exit(ERROR);
                }
            }
            // Send to TCP client
            else
            {
                if (send(channel->address[i].socket, shared_msg->tcp_content, strlen(shared_msg->tcp_content), 0) == -1)
                {
                    perror("Error sending data to TCP client");
                    exit(ERROR);
                }
            }
        }
    }

    pthread_mutex_unlock(&(channel->num_users_mutex));
    pthread_mutex_unlock(&(channel->clients_mutex));
}
