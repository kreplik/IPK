#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "tcp_listener.h"
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

volatile sig_atomic_t enable_listening = 1;

int pipe_fd[2];

void stop_listening(int socket)
{
    enable_listening = 0;

    // Create activity to close connection after SIGINT
    if (write(pipe_fd[1], "Sigint", strlen("Sigint") + 1) == -1) {
        perror("write");
        exit(ERROR);
    }

    close(pipe_fd[1]);
}

void listener(int welcome_socket_tcp, struct Channel_list *channel_list,struct Shared_msg *shared_msg)
{
    // Create pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(ERROR);
    }

    // Enable listening on socket
    int max_waiting_connections = 1;
    if (listen(welcome_socket_tcp, max_waiting_connections) < 0)
    {
        perror("ERROR: listen");
        exit(ERROR);
    }

    // Accept incoming connections
    accept_connection(welcome_socket_tcp,channel_list,shared_msg);
}

void accept_connection(int welcome_socket_tcp, struct Channel_list *channel_list,struct Shared_msg *shared_msg)
{
   
    struct sockaddr_in client_addr;
    pthread_t tid;
    socklen_t client_len = sizeof(client_addr);

    // SIGINT handler
    struct sigaction sa;
    sa.sa_handler = stop_listening;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    while (enable_listening)
    {
        
        // Accept TCP connection
        struct client_handler_args *args;
        int client_socket = accept(welcome_socket_tcp, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1)
        {
            // Got SIGINT
            if (errno == EINTR && !enable_listening)
            {
                break;
            }
            else
            {
                perror("accept");
                break;
                
            }
        }

        args = malloc(sizeof(struct client_handler_args));
        if (args == NULL)
        {
            perror("malloc");
            close(client_socket);
            continue;
        }

        // Create new client's structure
        Client_message *new_client = alloc_new_user(client_socket);

        args->client_socket = client_socket;
        args->client_addr = client_addr;
        args->client = new_client;
        args->channel_list = channel_list;
        args->shared_msg = shared_msg;

        // Create new thread for each incoming connection
        if (pthread_create(&tid, NULL, client_handler, args) != 0)
        {
            perror("pthread_create");
            shutdown(client_socket,SHUT_RDWR);
            close(client_socket);
            free(args);
            free(new_client);
            exit(ERROR);
        }

    }

    // Wait for thread to exit
    pthread_join(tid,NULL);

    // Close welcome socket
    shutdown(welcome_socket_tcp,SHUT_RDWR);
    close(welcome_socket_tcp);

}


void *client_handler(void *arg)
{
    // Pass parameters
    struct client_handler_args *args = (struct client_handler_args *)arg;
    int client_socket = args->client_socket;
    struct sockaddr_in client_addr = args->client_addr;
    struct Client_message *client = args->client;
    Channel_list *channel_list = args->channel_list;
    struct Shared_msg *shared_msg = args->shared_msg;

    // Handle communication with this client
    handle_tcp(client_socket, client_addr, client, channel_list,shared_msg);
    
    free(client);

    free(args);
    pthread_exit(NULL);
    return NULL;
}

Client_message *alloc_new_user(int client_socket)
{
    // Allocate client's structure
    Client_message *client = malloc(sizeof(Client_message));
    if (client == NULL)
    {
        perror("malloc");
        close(client_socket);
        exit(ERROR);
    }

    // Set client's default parameters
    client->authenticated = false;
    client->client_state = state_ACCEPT;
    client->message_type = 0;
    return client;
}
