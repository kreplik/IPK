//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
#ifndef TCP_H
#define TCP_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include "ipk.h"
#include "errors.h"

// Authentication structure
typedef struct tcp_auth
{
    char* display_name; // User's display name
    char* user_name; // User's user name
    char* secret; // User's secret
}tcp_auth;

// Join structure
typedef struct tcp_join
{
    char* display_name; // User's display name
    char* channel; // Channel's name
}tcp_join;

// Message structure
typedef struct tcp_msg
{
    char* user_message; // User's message
}tcp_msg;

// Final message structure
typedef struct Setup_tcp_msg
{
    char message[1500];
}Setup_tcp_msg;

// Received message structure
typedef struct server_tcp_msg
{
    char* rec_message; // Received message's content
    char* display_name; // Received message's display name
}server_tcp_msg;

// Received reply structure
typedef struct server_tcp_reply
{
    char* rec_reply; // Reply content
    char* confirmation; // Reply confirmation value
}server_tcp_reply;

/**
 * @brief Run program in TCP mode
 * @param socket Socket
 * @param address Server's address
 * @param address_size Addresses size
 * @returns Error if program has failed
*/
int run_tcp(int socket,struct sockaddr *address, socklen_t address_size);

/**
 * @brief Free allocated command's structures
*/
void freeInstructions(tcp_auth *auth,tcp_join *join,tcp_msg *msg);

/**
 * @brief Print reply in demanded way
*/
void printReply(char* confirmation, char* reply);

/**
 * @brief Print message in demanded way
*/
void printMsg(char* name, char* reply,int out);

/**
 * @brief Handle SIGINT
*/
void close_tcp(int socket);
#endif
