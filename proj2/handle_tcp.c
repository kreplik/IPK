#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include "handle_tcp.h"
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 1500

extern volatile sig_atomic_t enable_listening;

void handle_tcp(int client_socket, struct sockaddr_in client_addr, struct Client_message *client, Channel_list *channel_list, struct Shared_msg *shared_msg)
{
    // Buffer for received messages
    char buffer[BUFFER_SIZE];

    // Clear buffer
    bzero(buffer, sizeof(buffer));

    int bytes_received;
    bool run = true; // Changes on received ERR message

    // For handling sigint while blocked on recv()
    if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("fcntl");
        exit(ERROR);
    }

    // Set current channel to General
    Channel *current_channel = channel_list->channels[0];

    // Communicate with the client
    while (enable_listening && run)
    {
        // Receive client's message
        if ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
        {

            // Parse received message
            if (receive_message(client, buffer, client_addr) != SUCCESS)
            {
                bzero(buffer, sizeof(buffer));

                // Send appropriate reply if error occured
                if (client->message_type == AUTH)
                {
                    strcpy(buffer, "REPLY NOK IS Authentication failed.\r\n");
                }
                else if (client->message_type == JOIN)
                {
                    strcpy(buffer, "REPLY NOK IS Can not join this channel.\r\n");
                }
                send_to_client(client_socket, buffer);
                printf("SENT %s:%d | REPLY\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            }
            // Received valid message
            else
            {
                bzero(buffer, sizeof(buffer));
                switch (client->message_type)
                {
                    // Received AUTH message
                case AUTH:

                    // Send reply to client
                    strcpy(buffer, "REPLY OK IS Authentication successful.\r\n");
                    send_to_client(client_socket, buffer);

                    // Add client to default channel
                    add_client(current_channel, client_socket, NULL, TCP);
                    printf("SENT %s:%d | REPLY\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    bzero(buffer, sizeof(buffer));

                    // Setup message for both variants
                    sprintf(buffer, "%s joined %s.", client->displayname, current_channel->name);
                    get_udp_msg(shared_msg, "Server", buffer, TCP, NULL);

                    // Send all users in default channel, that new user joined, except the sender of AUTH message
                    send_all(current_channel, client_socket, shared_msg, shared_msg->content_size);
                    break;

                case JOIN:
                        // Received JOIN message

                    // Remove client from current channel
                    remove_client_from_channel(current_channel, client_socket);
                    bzero(buffer, sizeof(buffer));

                    // Send all users in current channel, that user left the channel
                    sprintf(buffer, "MSG FROM Server IS %s left %s.\r\n", client->displayname, current_channel->name);
                    send_all(current_channel, client_socket, shared_msg, 0);

                    // Create new current channel or return channel that user wants to join in
                    current_channel = create_channel(channel_list, client->channel);

                    // Add client to different channel
                    add_client(current_channel, client_socket, NULL, TCP);
                    printf("SENT %s:%d | REPLY\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    bzero(buffer, sizeof(buffer));

                    // Send reply to client
                    sprintf(buffer, "REPLY OK IS Join successful %s.\r\n", client->channel);
                    send_to_client(client_socket, buffer);

                    bzero(buffer, sizeof(buffer));

                    // Send all users, that new user joined this channel
                    sprintf(buffer, "MSG FROM Server IS %s joined %s.\r\n", client->displayname, current_channel->name);
                    send_all(current_channel, client_socket, shared_msg, 0);
                    break;

                case MSG:
                        // Received MSG message

                    // Setup message for both variants
                    get_udp_msg(shared_msg, client->displayname, client->message, TCP, NULL);

                    // Forward this message to all users in this channel, except the sender
                    send_all(current_channel, client_socket, shared_msg, shared_msg->content_size);
                    break;

                case BYE:
                        // Received BYE message

                    // Remove user from this channel
                    remove_client_from_channel(current_channel, client_socket);
                    bzero(buffer, sizeof(buffer));
                    sprintf(buffer, "%s left %s.", client->displayname, current_channel->name);

                    // Setup message for both variants
                    get_udp_msg(shared_msg, client->displayname, buffer, TCP, NULL);

                    // Send all users in this channel that another user has left the channel
                    send_all(current_channel, client_socket, shared_msg, shared_msg->content_size);
                    run = false;
                    break;

                case UNKNOWN:
                        // Received unknown message type
                    bzero(buffer, sizeof(buffer));
                    strcpy(buffer, "ERR FROM Server IS Message failed to be parsed.\r\n");

                    // Send err message to this client
                    send_to_client(client_socket, buffer);
                    printf("SENT %s:%d | ERR\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    // Change user's state
                    client->client_state = state_END;
                }

                // User has reached END state
                if (client->client_state == state_END)
                {
                    bzero(buffer, sizeof(buffer));
                    strcpy(buffer, "BYE\r\n");

                    // Send BYE to client
                    send_to_client(client_socket, buffer);
                    printf("SENT %s:%d | BYE\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    // Remove user from this channel
                    remove_client_from_channel(current_channel, client_socket);
                    bzero(buffer, sizeof(buffer));
                    sprintf(buffer, "%s left %s.", client->displayname, current_channel->name);

                    // Setup message for both variants
                    get_udp_msg(shared_msg, client->displayname, buffer, TCP, NULL);

                    // Send all users in this channel that another user has left
                    send_all(current_channel, client_socket, shared_msg, shared_msg->content_size);
                    run = false;
                }

                bzero(buffer, sizeof(buffer));
            }
        }

        else if (bytes_received < 0)
        {
            // No data received yet, continue receiving
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                perror("recv");
                exit(ERROR);
            }
        }
        else if (bytes_received == 0)
        {
            // Connection closed by user
            break;
        }
    }

    return;
}

void send_bye_to_all(struct Channel_list *channel_list, int srv_ID)
{
    // Buffer for TCP BYE
    char buffer[BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));

    // Setup TCP BYE
    strcpy(buffer, "BYE\r\n");

    // Buffer for UDP BYE
    char udp_buffer[3];
    bzero(udp_buffer, sizeof(udp_buffer));

    // Setup UDP BYE
    udp_buffer[0] = 0xFF;
    udp_buffer[1] = (srv_ID >> 8) & 0xFF;
    udp_buffer[2] = srv_ID++ & 0xFF;
    
    pthread_mutex_lock(&(channel_list->channels_mutex));
    pthread_mutex_lock(&(channel_list->num_of_channel_mutex));

    // Iterate over all channels created
    for (int i = 0; i < channel_list->num_of_channels; i++)
    {
        pthread_mutex_lock(&(channel_list->channels[i]->num_users_mutex));
        pthread_mutex_lock(&(channel_list->channels[i]->clients_mutex));

        // Iterate over all users in channel
        for (int j = 0; j < channel_list->channels[i]->num_users; j++)
        {
            // Send BYE to UDP user
            if (channel_list->channels[i]->address[j].udp_client)
            {
                if(sendto(channel_list->channels[i]->address[j].socket,udp_buffer,3,0,
                channel_list->channels[i]->address[j].udp_address,
                channel_list->channels[i]->address[j].udp_address_len) == -1){
                    perror("sendto");
                    exit(ERROR);
                }
            }
            // Send BYE to TCP user
            else
            {
                if (send(channel_list->channels[i]->address[j].socket, buffer, strlen(buffer), 0) == -1)
                {
                    perror("Error sending data to TCP client");
                    exit(ERROR);
                }
            }
        }
        pthread_mutex_unlock(&(channel_list->channels[i]->clients_mutex));
        pthread_mutex_unlock(&(channel_list->channels[i]->num_users_mutex));
        
    }
    pthread_mutex_unlock(&(channel_list->num_of_channel_mutex));
    pthread_mutex_unlock(&(channel_list->channels_mutex));
    
}

void send_to_client(int client_socket, char *buffer)
{
    // Send message to specific client
    if (send(client_socket, buffer, strlen(buffer), 0) == -1)
    {
        perror("Error sending data to TCP client");
        exit(EXIT_FAILURE);
    }
}
