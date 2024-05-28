/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef UDP_R_H
#define UDP_R_H
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
#include "handle_udp.h"
#include "socket.h"

struct MSG_type;
struct UDP_User;

/**
 * @brief Parse received message
 * @param rec_message Received message
 * @param udp_user UDP user's/message parameters
 * @param message_type Message's opcode
 * @param client_address Client's address
 * @return Parsing result
*/
int parse_messageUDP(char *rec_message, struct UDP_User *udp_user  ,struct MSG_type *message_type, struct sockaddr_in client_address);

#endif
