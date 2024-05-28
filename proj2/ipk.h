/**
 * @author Adam Nieslanik (xniesl00)
 * @brief IPK24chat-server
*/


#ifndef IPK
#define IPK
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>
#include "errors.h"
#include "socket.h"


#define TCP 100
#define UDP 200

#define AUTH 1
#define JOIN 2
#define MSG 3
#define BYE 4
#define RENAME 5
#define HELP 6
#define ERR 7
#define UNKNOWN 8
#define REPLY_OK 9
#define REPLY_NOK 10

#define out_msg 30
#define out_err 40

#define state_ACCEPT 100
#define state_AUTH 200
#define state_OPEN 300
#define state_ERROR 400
#define state_END 500
#define state_AUTH_failed 222


// Represents command line parameters
typedef struct
{
    uint16_t port; // Server's port
    uint16_t timeout; // Timeout for UDP retransmission
    uint8_t retry; // Number of retries
    char* hostname; // Server's hostname
}Params;

#endif
