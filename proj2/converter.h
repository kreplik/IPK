/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/

#ifndef CONVERTER
#define CONVERTER

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
#include "handle_tcp.h"
#include "handle_udp.h"

/**
 * @brief Converts message to UDP format
 * @param shared_msg Structure that holds final tcp/udp messages
 * @param displayname Displayname in the final message
 * @param content Content of the final message
 * @param original Mode, that indicates which message format was original (tcp/udp)
 * @param udp_user Structure that holds udp user's parameters
*/
void get_udp_msg(struct Shared_msg *shared_msg, char *displayname, char *content, int original,struct UDP_User *udp_user);

#endif
