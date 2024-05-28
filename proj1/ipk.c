#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include "ipk.h"
#include "tcp.h"
#include "udp.h"
#include "errors.h"


// Command line parameters
Params p;
Params *params;

// server's address
struct hostent *server;
struct sockaddr_in server_address;
struct sockaddr *address;
socklen_t address_size;

// Init defualt parameter's values
void init(Params *params)
{
    params->port = 4567;
    params->retry = 3;
    params->timeout = 250;
}

int main(int argc, char *argv[])
{
   
    // Initialize parameters
    params = &p;
    init(params);

    // Parse command line arguments
    struct option longOptions[] = {
        {"protocol", required_argument, NULL, 't'},
        {"hostname", required_argument, NULL, 's'},
        {"port", optional_argument, NULL, 'p'},
        {"timeout", optional_argument, NULL, 'd'},
        {"retry", optional_argument, NULL, 'r'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}};

    int c, optionIndex;

    // Parse parameters and store them into structure
    while ((c = getopt_long(argc, argv, "t:s:p:drh", longOptions, &optionIndex)) != -1)
    {
        switch (c)
        {
        case 't':
            params->protocol = optarg;
            break;

        case 's':
            params->hostname = optarg;
            break;

        case 'p':
            params->port = atoi(optarg);
            break;

        case 'd':
            params->timeout = atoi(optarg);
            break;

        case 'r':
            params->retry = atoi(optarg);
            break;

        case 'h':
            printf("Usage: -t [tcp/udp]\n -s [hostname]\n -p [port]\n -d [timeout]\n -r [retries]\n"); // TODO
            return 0;

        default:
            fprintf(stderr, "Unknown command line argument");
            return ERROR;
        }
    }

    // Set connection parameters
    int socket_type;
    int socket_family = AF_INET;
    int mode;

    if (strcmp(params->protocol,"tcp") == 0)
    {
        socket_type = SOCK_STREAM;
        mode = TCP;
    }
    else
    {
        socket_type = SOCK_DGRAM;
        mode = UDP;
    }


    struct addrinfo server, *result;

    memset(&server,0,sizeof(server));
    server.ai_family = AF_INET;
    server.ai_protocol = 0;
    server.ai_socktype = socket_type;

    // Get server's info
    int status = getaddrinfo(params->hostname,NULL,&server,&result);
    if(status != 0)
    {
        fprintf(stderr,"ERR: getaddrinfo\n");
        exit(ERROR);
    }

   
        memset(&server_address, 0, sizeof(server_address));
        memcpy(&server_address, result->ai_addr, sizeof(server_address));
        server_address.sin_port = htons(params->port);  // Set the port


    // Creating socket
    int client_socket = socket(socket_family, socket_type, 0);
    if (client_socket <= 0)
    {
        perror("ERR: socket");
        exit(ERROR);
    }
    
    address = (struct sockaddr *)&server_address;
    address_size = sizeof(server_address);
    
    int client_result = 0;
    if(mode == UDP)
    {
        // Run program in UDP mode
       client_result = run_udp(client_socket,address,address_size);
    }
    else if(mode == TCP)
    {
        // Run program in TCP mode
        client_result = run_tcp(client_socket,address,address_size);
    }


    return client_result;
}

// Print help
void printHelp(void)
{
    printf("--------HELP-------\n");
    printf("Commands:\n");
    printf("\t /auth {Username} {Secret} {DisplayName} - Sends AUTH message to the server, locally sets the DisplayName value\n");
    printf("\n\t /join {Channel} - Sends JOIN message with channel name to the server\n");
    printf("\n\t /rename {DisplayName} - Locally changes the display name of the user\n");
    printf("\n\t /help - prints this HELP\n");
    printf("--------HELP-------\n");
}
