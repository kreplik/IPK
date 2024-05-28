//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
#ifndef UDP_H
#define UDP_H
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

// Authentication structure
typedef struct udp_auth
{
    char* display_name; // User's display name
    char* user_name; // User's user name
    char* secret; // User's secret
}udp_auth;

// Join structure
typedef struct udp_join
{
    char* display_name; // User's display name
    char* channel; // Channel's name
}udp_join;

// Received message structure
typedef struct server_msg
{
    char* rec_message; // Received message's content
    char* display_name; // Received message's display name
}server_msg;

// Received reply structure
typedef struct server_reply
{
    char* rec_reply; // Reply content
    char* result; // Reply result OK/NOK
    uint16_t referenceID; // Reference to sent message
    char* replyID; // Reply message ID
}server_reply;

// Confirm structure
typedef struct confirm
{
    uint16_t replyID; // Reference to reply
}confirm;

// User message structure
typedef struct user_msg
{
    char* user_message; // User's message
}user_msg;

// Final message that is about to be sent
typedef struct Setup_msg
{
    char send_message[1500]; // Content
    int message_length; // Message length
    int command; // Command value
    uint16_t messageID; // Message unique ID
}Setup_msg;

/**
 * @brief Run program in UDP mode
 * @param socket Socket
 * @param address Server's address
 * @param addres_size Server's address size
*/
int run_udp(int socket,struct sockaddr *address, socklen_t address_size);

/**
 * @brief Handle SIGINT
*/
void close_udp(int socket);

/**
 * @brief Initialize message type structure
 * @param message_type Message type structure
*/
void initMsg_type(struct MSG_type *message_type);
#endif
