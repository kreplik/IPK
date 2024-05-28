/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef HANDLE_TCP_H
#define HANDLE_TCP_H
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
#include "tcp_listener.h"
#include "tcp_receiver.h"
#include "converter.h"

/**
 * @brief Handles specific TCP user's communication
 * @param client_socket Client's socket
 * @param client_addr Client's address
 * @param client TCP client's structure
 * @param channel_list Structure for channels
 * @param shared_msg Structure for tcp/udp messages
*/
void handle_tcp(int client_socket, struct sockaddr_in client_addr,struct Client_message *client, struct Channel_list *channel_list,struct Shared_msg *shared_msg);

/**
 * @brief Sends message to specific client
 * @param client_socket Client's socket
 * @param buffer Message to be sent
*/
void send_to_client(int client_socket, char *buffer);

/**
 * @brief Sends bye message to all client's
 * @param channel_list Structure of channels created
 * @param srv_ID ID of messages sent by server
*/
void send_bye_to_all(struct Channel_list *channel_list, int srv_ID);

#endif
