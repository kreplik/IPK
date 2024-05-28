//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
#ifndef TCP_M_H
#define TCP_M_H
#include <stdio.h>
#include <stdlib.h>
#include "tcp.h"
#include "ipk.h"

/**
 * @brief Setup message in demanded way and stores it into structure
 * @param command Parsed command value
 * @param setup_msg Final message's structure
 * @param auth Authentication structure
 * @param join Join structure
 * @param msg User's message structure
*/
void setup_msgT(int command, struct Setup_tcp_msg *setup_msg,struct tcp_auth *auth, struct tcp_join *join, struct tcp_msg *msg);

/**
 * @brief Send BYE message
 * @param socket Socket
*/
void send_byeTCP(int socket);

/**
 * @brief Send ERR message
 * @param socket Socket
 * @param auth Authentication structure
*/
void send_errorTCP(int socket,tcp_auth *auth);
#endif
