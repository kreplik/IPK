//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
#ifndef TCP_C_H
#define TCP_C_H
#include <stdio.h>
#include <stdlib.h>
#include "ipk.h"

/**
 * @brief Parse stdin command
 * @param parsed_message Stdin command
 * @param auth Authentication parameter's structure
 * @param join Join parameter's structure
 * @param msg User message paramere's structure
 * @returns command value
*/
int parse_commandTCP(char *parsed_message, struct tcp_auth *auth, struct tcp_join *join, struct tcp_msg *msg);

/**
 * @brief Checks if the command is valid
 * @param parsed_message Stdin command
 * @returns true, if command is valid
*/
bool check_commandTCP(char* parsed_message);

/** 
 * @brief Checks if command parameter's are valid
 * @param parsed_message Stdin command
 * @param patter Regular expression
 * @returns true, if command's parameter is valid
*/
bool regex(char* parsed_message, const char* pattern);

#endif
