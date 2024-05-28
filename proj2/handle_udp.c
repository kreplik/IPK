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
#include <pthread.h>
#include "handle_udp.h"
#include <errno.h>

#define BUFFER_SIZE 1500

// For initializing message type structure
MSG_type message_typeInit;
MSG_type *message_type;

// Server's unique message ID
uint8_t srv_ID = 0;

extern int pipe_fd[2];

void initMsg_type(struct MSG_type *message_type)
{
    message_type->CONFIRMtype = 0x00;
    message_type->REPLYtype = 0x01;
    message_type->AUTHtype = 0x02;
    message_type->JOINtype = 0x03;
    message_type->MSGtype = 0x04;
    message_type->ERRtype = 0xFE;
    message_type->BYEtype = 0xFF;
}

void dynamic_binder(struct sockaddr_in client_addr, struct UDP_clients *udp_clients)
{
    // Create new socket for user
    int dynamic_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in dynamic_client_addr;

    // Create address with dynamic port for user
    dynamic_client_addr.sin_family = AF_INET;
    dynamic_client_addr.sin_addr = client_addr.sin_addr;
    dynamic_client_addr.sin_port = htons(0);

    // Bind new socket
    if (bind(dynamic_socket, (struct sockaddr *)&dynamic_client_addr, sizeof(dynamic_client_addr)) < 0)
    {
        perror("Error binding dynamic socket");
        close(dynamic_socket);
    }

    // Store new address to communicate with this user to list
    udp_clients->udp_user[udp_clients->num_clients]->client_state = state_ACCEPT;
    udp_clients->udp_user[udp_clients->num_clients]->dynamic_port = dynamic_client_addr.sin_port;
    udp_clients->udp_user[udp_clients->num_clients]->dyn_socket = dynamic_socket;
    udp_clients->udp_user[udp_clients->num_clients]->original_port = client_addr.sin_port;
    udp_clients->udp_user[udp_clients->num_clients]->sin_addr = client_addr.sin_addr;

    udp_clients->num_clients++;

    return;
}

void add_user(struct UDP_clients *udp_clients, struct sockaddr_in client_addr)
{
    // Check if user is new for this server
    if (udp_clients->num_clients != 0)
    {
        for (int i = 0; i < udp_clients->num_clients; i++)
        {
            // User found in list
            if (udp_clients->udp_user[i]->original_port == client_addr.sin_port)
            {
                return;
            }
        }
    }

    // New client
    UDP_User *udp_user = malloc(sizeof(UDP_User));
    if (udp_user == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    // Add this client to list
    udp_clients->udp_user[udp_clients->num_clients] = udp_user;

    // Create new address to communicate with this user
    dynamic_binder(client_addr, udp_clients);
}

UDP_User *get_user(struct UDP_clients *udp_clients, struct sockaddr_in client_addr)
{
    // Find address to communicate with this client
    for (int i = 0; i < udp_clients->num_clients; i++)
    {
        if (udp_clients->udp_user[i]->original_port == client_addr.sin_port)
        {
            return udp_clients->udp_user[i];
        }
    }
}

void udp_handler(int client_socket, struct Channel_list *channel_list, struct Shared_msg *shared_msg)
{
    struct sockaddr_in client_address;
    socklen_t address_size = sizeof(client_address);
    struct sockaddr *address = (struct sockaddr *)&client_address;

    // Allocate global structures

    UDP_clients *udp_clients = malloc(sizeof(UDP_clients));
    if (udp_clients == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    Server_msg *server_msg = malloc(sizeof(Server_msg));
    if (server_msg == NULL)
    {
        perror("malloc");
        exit(ERROR);
    }

    // Initialize message type structure
    message_type = &message_typeInit;
    initMsg_type(message_type);

    // Guffer for received message
    char buffer[BUFFER_SIZE];

    // Buffer for sent replies
    char server_reply[BUFFER_SIZE];
    int bytes_rx;

    // Set current channel to General
    Channel *current_channel = channel_list->channels[0];

    // Represents current user's informations
    UDP_User *current_user;

    fd_set readfds;

    // Run until SIGINT
    while (1)
    {

        bzero(buffer, sizeof(buffer));

        FD_ZERO(&readfds);

        // Add all server sockets to the set
        for (int i = 0; i < udp_clients->num_clients; i++)
        {
            FD_SET(udp_clients->udp_user[i]->dyn_socket, &readfds);
        }

        // Add default server socket to the set
        FD_SET(client_socket, &readfds);

        // Add pipe file descriptor to the set
        FD_SET(pipe_fd[0], &readfds);

        // Wait for activity on sockets or pipe file descriptor
        int activity = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("select");
            exit(ERROR);
        }

        // Activity created by SIGINT, close UDP receiving
        if(FD_ISSET(pipe_fd[0],&readfds))
        {
            close(pipe_fd[0]);
            break;
        }

        // Check for activity on dynamic sockets
        for (int i = 0; i < udp_clients->num_clients; i++)
        {
            // Activity found on dynamic socket
            if (FD_ISSET(udp_clients->udp_user[i]->dyn_socket, &readfds))
            {
                // Receive message from this address
                bytes_rx = recvfrom(udp_clients->udp_user[i]->dyn_socket, buffer, BUFFER_SIZE, 0, address, &address_size);
                if (bytes_rx < 0)
                {
                    perror("recv");
                    continue;
                }

                // Get address to communicate with this user
                current_user = get_user(udp_clients, client_address);

                break;
            }
        }

        // Activity found on default server's socket
        if (FD_ISSET(client_socket, &readfds))
        {
            // Receive message on this socket
            bytes_rx = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, address, &address_size);
            if (bytes_rx < 0)
            {
                perror("recv");
                continue;
            }

            // Add this user to User's list
            add_user(udp_clients, client_address);

            // Get address to communicate with this user
            current_user = get_user(udp_clients, client_address);
        }

        // Parse received message
        int opcode = parse_messageUDP(buffer, current_user, message_type, client_address);

        switch (opcode)
        {
        case 0:
            // Received CONFIRM
            break;

        //////////////////////////////////////////////////////////////////////
        case 2:
                // Received AUTH message
            bzero(buffer, sizeof(buffer));
            bzero(server_reply, sizeof(server_reply));

            // Send confirmation of this message
            send_confirm(current_user, message_type);

            // Setup UDP message with content
            strcpy(server_reply, "Authentication successful.");
            setup_message(server_msg, current_user, message_type, REPLY_OK, server_reply);

            // Send reply to this user
            send_to_udp_client(server_msg->content, server_msg->content_size, current_user);
            printf("SENT %s:%d | REPLY\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

            // Add this user to default channel
            add_client(current_channel, current_user->dyn_socket, current_user, UDP);
            current_user->channel = current_channel;

            sprintf(buffer, "%s joined %s.", current_user->displayname, current_channel->name);

            // Construct message for both variant
            get_udp_msg(shared_msg, "Server", buffer, TCP, NULL);

            // Send message to all users in this channel, that new user has joined
            send_all(current_channel, current_user->dyn_socket, shared_msg, shared_msg->content_size);

            break;
        //////////////////////////////////////////////////////////////////////
        case 3:
                // Received JOIN message
            bzero(buffer, sizeof(buffer));
            bzero(server_reply, sizeof(server_reply));

            // Send confirmation of this message
            send_confirm(current_user, message_type);

            sprintf(buffer, "%s left %s.", current_user->displayname, current_user->channel->name);

            // Construct message for both variant
            get_udp_msg(shared_msg, "Server", buffer, TCP, NULL);

            // Send message to all users in this channel, that user has left
            send_all(current_user->channel, current_user->dyn_socket, shared_msg, shared_msg->content_size);
            
            // Remove user from this channel
            remove_client_from_channel(current_user->channel, current_user->dyn_socket);

            // Create new channel or return channel, that user wants to join in
            current_user->channel = create_channel(channel_list, current_user->channel_name);

            strcpy(server_reply, "Join successful.");

            // Setup UDP message with content
            setup_message(server_msg, current_user, message_type, REPLY_OK, server_reply);

            // Send reply to this client
            send_to_udp_client(server_msg->content, server_msg->content_size, current_user);
            printf("SENT %s:%d | REPLY\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

            // Add user to different channel
            add_client(current_user->channel, current_user->dyn_socket, current_user, UDP);

            break;
        //////////////////////////////////////////////////////////////////////
        case 4:
                // Received MSG message
            // Send confirmation of this message
            send_confirm(current_user, message_type);
            
            // Construct message for both variant
            get_udp_msg(shared_msg, current_user->displayname, buffer, UDP, current_user);

            // Forward this message to all users in this channel except the sender
            send_all(current_user->channel, current_user->dyn_socket, shared_msg, shared_msg->content_size);
            break;
        //////////////////////////////////////////////////////////////////////
        case -1:
                // Received BYE message
            bzero(buffer, sizeof(buffer));

            // Send confirmation of this message
            send_confirm(current_user, message_type);

            sprintf(buffer, "%s left %s.", current_user->displayname, current_user->channel->name);

            // Construct message for both variant
            get_udp_msg(shared_msg, "Server", buffer, TCP, NULL);

            // Send message to all users in this channel, that user has left
            send_all(current_user->channel, current_user->dyn_socket, shared_msg, shared_msg->content_size);

            // Remove user from this channel
            remove_client_from_channel(current_user->channel, current_user->dyn_socket);

            // Remove user from UDP users list
            remove_udp_user(current_user->dyn_socket, udp_clients);
            break;
        //////////////////////////////////////////////////////////////////////
        case -2:
                // Received ERR message

            // Set clients state to END
            current_user->client_state = state_END;

            // Send confirmation of this message
            send_confirm(current_user, message_type);

            // Send BYE to this user
            send_bye(current_user, message_type);
            break;
        //////////////////////////////////////////////////////////////////////
        default:
                // Received unknown message type
            current_user->client_state = state_ERROR;

            // Send confirmation of this message
            send_confirm(current_user, message_type);

            // Send ERR message to this user
            send_err(current_user, message_type);

            // Set clients state to END
            current_user->client_state = state_END;
            break;
        }

        // User has reached END state
        if (current_user->client_state == state_END)
        {
            // Remove user from this channel and UDP users list
            remove_client_from_channel(current_user->channel, current_user->dyn_socket);
            remove_udp_user(current_user->dyn_socket, udp_clients);
        }
    }

    // Stopped receiving messages after SIGINT
    // Send BYE to all listening users
    send_bye_to_all(channel_list,srv_ID++);

    // Free allocated memory of users list
    for (int i = 0; i < udp_clients->num_clients; i++)
    {
        free(udp_clients->udp_user[i]);
    }

    // Close server's default socket
    shutdown(client_socket,SHUT_RDWR);
    close(client_socket);

    // Free important structures
    free(udp_clients);
    free(server_msg);
}

void send_to_udp_client(char *buffer, size_t buffer_size, struct UDP_User *udp_user)
{
    struct sockaddr_in dynamic_client_addr;

    dynamic_client_addr.sin_family = AF_INET;
    dynamic_client_addr.sin_addr = udp_user->sin_addr;
    dynamic_client_addr.sin_port = udp_user->original_port;

    // Send message to specific UDP user from according address
    int rx = sendto(udp_user->dyn_socket, buffer, buffer_size, 0, (struct sockaddr *)&dynamic_client_addr, sizeof(dynamic_client_addr));
    if (rx < 0)
    {
        perror("sendto");
        exit(ERROR);
    }
}

void send_confirm(struct UDP_User *udp_user, struct MSG_type *message_type)
{
    struct sockaddr_in dynamic_client_addr;

    dynamic_client_addr.sin_family = AF_INET;
    dynamic_client_addr.sin_addr = udp_user->sin_addr;
    dynamic_client_addr.sin_port = udp_user->original_port;

    char buffer[3];

    // Setup CONFIRM message
    buffer[0] = message_type->CONFIRMtype;
    buffer[1] = (udp_user->id >> 8) & 0xFF;
    buffer[2] = udp_user->id & 0xFF;

    // Send CONFIRM to specific UDP user from according address
    int rx = sendto(udp_user->dyn_socket, buffer, 3, 0, (struct sockaddr *)&dynamic_client_addr, sizeof(dynamic_client_addr));
    if (rx < 0)
    {
        perror("sendto");
        exit(ERROR);
    }

    printf("SENT %s:%d | CONFIRM\n", inet_ntoa(udp_user->sin_addr), ntohs(udp_user->original_port));
}

void send_err(struct UDP_User *udp_user, struct MSG_type *message_type)
{
    struct sockaddr_in dynamic_client_addr;

    dynamic_client_addr.sin_family = AF_INET;
    dynamic_client_addr.sin_addr = udp_user->sin_addr;
    dynamic_client_addr.sin_port = udp_user->original_port;

    char buffer[BUFFER_SIZE];

    // Setup ERR message
    buffer[0] = message_type->ERRtype;
    buffer[1] = (srv_ID >> 8) & 0xFF;
    buffer[2] = srv_ID++ & 0xFF;

    // Setup content of message
    size_t displayname_l = strlen("Server");
    memcpy(&buffer[3], "Server", displayname_l);

    size_t content_l = strlen("Message failed to be parsed");

    memcpy(&buffer[displayname_l + 4], "Message failed to be parsed", content_l);

    // Send ERR message to specific user from according address
    int rx = sendto(udp_user->dyn_socket, buffer, displayname_l + content_l + 5, 0, (struct sockaddr *)&dynamic_client_addr, sizeof(dynamic_client_addr));
    if (rx < 0)
    {
        perror("sendto");
        exit(ERROR);
    }

    printf("SENT %s:%d | ERR\n", inet_ntoa(udp_user->sin_addr), ntohs(udp_user->original_port));
}

void send_bye(struct UDP_User *udp_user, struct MSG_type *message_type)
{
    struct sockaddr_in dynamic_client_addr;

    dynamic_client_addr.sin_family = AF_INET;
    dynamic_client_addr.sin_addr = udp_user->sin_addr;
    dynamic_client_addr.sin_port = udp_user->original_port;

    char buffer[3];
    bzero(buffer,sizeof(buffer));

    // Setup BYE message
    buffer[0] = message_type->BYEtype;
    buffer[1] = (srv_ID >> 8) & 0xFF;
    buffer[2] = srv_ID++ & 0xFF;

    // Send BYE message to specific user from according address
    int rx = sendto(udp_user->dyn_socket, buffer, 3, 0, (struct sockaddr *)&dynamic_client_addr, sizeof(dynamic_client_addr));
    if (rx < 0)
    {
        perror("sendto");
        exit(ERROR);
    }

    printf("SENT %s:%d | BYE\n", inet_ntoa(udp_user->sin_addr), ntohs(udp_user->original_port));
}

void remove_udp_user(int users_socket, struct UDP_clients *udp_clients)
{
    // Iterate over all clients in udp users list
    for (int i = 0; i < udp_clients->num_clients; i++)
    {
        // Close this user's socket and free memory allocated for this user
        if (udp_clients->udp_user[i]->dyn_socket == users_socket)
        {
            close(users_socket);

            free(udp_clients->udp_user[i]);

            // Remove users structure from list
            for (int j = i; j < udp_clients->num_clients - 1; j++)
            {
                udp_clients->udp_user[j] = udp_clients->udp_user[j + 1];
            }

            // Decrement number of users in list
            udp_clients->num_clients--;
            break;
        }
    }
}
