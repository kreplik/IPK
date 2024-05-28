//////////////////////////////////////////////////////////////////
//         @@          IPK24CHAT-CLIENT            @@           //
//                                                              //
//         author:  Adam Nieslanik (xniesl00)                   //
//                                                              //
//                                                              //
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////
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


#define TCP 100
#define UDP 200

#define AUTH 1
#define JOIN 2
#define MSG 3
#define BYE 4
#define RENAME 5
#define HELP 6

#define out_msg 30
#define out_err 40

#define state_START 100
#define state_AUTH 200
#define state_OPEN 300
#define state_ERROR 400
#define state_END 500
#define state_AUTH_failed 222

// Changes on C-c signal and handles SIGINT
static volatile bool connection = true;

// represents command line parameters
typedef struct
{
    char* protocol;
    uint16_t port;
    uint16_t timeout;
    uint8_t retry;
    char* hostname;
}Params;


/**
 * @brief Initialize defualt parameter's values
 * @param Params command line parameters structure
*/
void init(Params *params);

/**
 * @brief Prints help
*/
void printHelp(void);
#endif
