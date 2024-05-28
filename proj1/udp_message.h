//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
#ifndef UDP_M_H
#define UDP_M_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include "ipk.h"
#include "errors.h"
#include "udp.h"

/**
 * @brief Setup final message that is about to be sent
 * @param setup_msg Final message structure
 * @param auth Authentication structure
 * @param join Join structure
 * @param msg // User's message structure
 * @param message_type User's message type
*/
void setup_message(struct Setup_msg *setup_msg,struct udp_auth *auth, struct udp_join *join, struct user_msg *msg, struct MSG_type *message_type);

/**
 * @brief Sends CONFIRM message
 * @param id Message's unique ID
 * @param socket Socket
 * @param address Server's address
 * @param address_size Server's address size
*/
void send_confirm(char* id, int socket, struct sockaddr *address, socklen_t address_size);

/**
 * @brief Sends BYE message
 * @param id Message's unique ID
 * @param socket Socket
 * @param address Server's address
 * @param address_size Server's address size
*/
void send_bye(int id, int socket, struct sockaddr *address, socklen_t address_size);

/**
 * @brief Sends ERR message
 * @param id Message's unique ID
 * @param socket Socket
 * @param address Server's address
 * @param address_size Server's address size
 * @param message_type Message's type
 * @param auth Authentication structure
*/
void send_error(int id, int socket, struct sockaddr *address, socklen_t address_size,struct MSG_type *message_type, struct udp_auth *auth);

/**
 * @brief Sends message to server
 * @param size Message's length
 * @param send_message Message to be sent
 * @param socket Socket
 * @param address Server's address
 * @param address_size Server's address size
 * @returns Sent message's type
*/
char sender(int size,char* send_message,int socket, struct sockaddr *address, socklen_t address_size);
#endif
