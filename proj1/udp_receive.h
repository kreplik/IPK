//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
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
#include "udp.h"

/**
 * @brief Parse received message
 * @param rec_message Received message
 * @param server_rply Received reply structure
 * @param server_message Received message structure
 * @param conf Confirm structure
 * @param message_type Received message type
 * @returns Received message type
*/
int parse_message(char* rec_message, struct server_reply *server_rply, struct server_msg *server_message, struct confirm *conf,struct MSG_type *message_type);

#endif
