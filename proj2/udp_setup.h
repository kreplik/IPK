/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef UDP_SETUP_H
#define UDP_SETUP_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include "ipk.h"
#include "errors.h"
#include "socket.h"
#include <signal.h>
#include "handle_udp.h"

/**
 * @brief Setup UDP message
 * @param shared_msg Structure to store message in both variants
 * @param udp_user User's parameters
 * @param message_type Type of message
 * @param command Used command by user
 * @param reply Content of reply
*/
void setup_message(struct Server_msg *server_msg, struct UDP_User *udp_user, struct MSG_type *message_type, int command, char* reply);

#endif
