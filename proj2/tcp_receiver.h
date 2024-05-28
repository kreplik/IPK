/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef TCP_RECEIVER
#define TCP_RECEIVER
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include "tcp_listener.h"
#include "handle_tcp.h"
#include "ipk.h"
#include "errors.h"
#include "socket.h"


#define AUTH_part "AUTH"
#define AS_part "AS"
#define JOIN_part "JOIN"
#define USING_part "USING"
#define IS_part "IS"
#define NOK_part "NOK"
#define OK_part "OK"
#define MSG_part "MSG"
#define FROM_part "FROM"
#define ERR_part "ERR"
#define BYE_part "BYE\r\n"

/**
 * @brief Parse received message
 * @param client Client's parameters
 * @param buffer Received message
 * @param client_addr Client's address
 * @return Parsing result
*/
int receive_message(struct Client_message *client, char* buffer,struct sockaddr_in client_addr);

/**
 * @brief Parse received message according to message type
 * @param parsed_message Parsed message
 * @param client Client's parameters
 * @return Parsing result
*/
int parse_message(char *parsed_message, struct Client_message *client);

/**
 * @brief Checks for message's parameters
 * @param message_part Message part to compare
 * @param pattern Pattern to compare message part with
 * @return Compare result
*/
bool message_checker(char *message_part, char *pattern);

/**
 * @brief Parse AUTH message
 * @param parsed_message Parsed message
 * @param client Client's parameters
 * @return Parsing result
*/
int parse_auth(char *parsed_message, struct Client_message *client);

/**
 * @brief Parse MSG message
 * @param buffer Parsed message
 * @param client Client's parameters
 * @return Parsing result
*/
int parse_msg(char *buffer, struct Client_message *client);

/**
 * @brief Parse JOIN message
 * @param parsed_message Parsed message
 * @param client Client's parameters
 * @return Parsing result
*/
int parse_join(char *buffer, struct Client_message *client);

#endif
