//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
#ifndef UDP_C_H
#define UDP_C_H
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
 * @brief Parse stdin command
 * @param parsed_message Stdin command
 * @param auth Authentication structure
 * @param join Join structure
 * @param msg User's message structure
 * @returns Command value
*/
int parse_command(char *parsed_message, struct udp_auth *auth, struct udp_join *join, struct user_msg *msg);

/**
 * @brief Checks if command is valid
 * @param parsed_message Stdin command
 * @returns true, if command is valid
*/
bool check_command(char* parsed_message);

/**
 * @brief Checks if command's parameter is valid
 * @param parsed_message Stdin command
 * @param pattern Regular expression pattern
 * @returns true, if parameter is valid
*/
bool regexUDP(char* parsed_message, const char* pattern);

#endif
