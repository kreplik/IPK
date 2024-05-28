#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include "ipk.h"

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
    params->hostname = "0.0.0.0";
}

int main(int argc, char *argv[])
{

    // Initialize parameters
    params = &p;
    init(params);

    // Parse command line arguments
    struct option longOptions[] = {
        {"hostname", optional_argument, NULL, 'l'},
        {"port", optional_argument, NULL, 'p'},
        {"timeout", optional_argument, NULL, 'd'},
        {"retry", optional_argument, NULL, 'r'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}};

    int c, optionIndex;

    // Parse parameters and store them into structure
    while ((c = getopt_long(argc, argv, "l:p:d:r:h", longOptions, &optionIndex)) != -1)
    {
        switch (c)
        {
        case 'l':
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

    // Create sockets for TCP and UDP communication
    create_socket();

    return 0;
}
