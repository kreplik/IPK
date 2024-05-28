#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "ipk.h"
#include "tcp.h"
#include "tcp_command.h"
#include "tcp_message.h"

// Program's present state
int TCPpresent_state;

// Handles SIGINT
void close_tcp(int socket)
{
    connection = false; // Terminate connection
    return;
}

int run_tcp(int socket, struct sockaddr *address, socklen_t address_size)
{
    signal(SIGINT, close_tcp); // Catch C-c signal
    // Connecting to host
    if (connect(socket, address, address_size) != 0)
    {
        perror("ERR: connect");
        close(socket);
        exit(ERROR);
    }

    int command = 0;
    // Indicates's, if we got the reply for the sent message
    bool gotReply = true;

    // Stdin buffer
    char message[1500];
    // Send message buffer
    char send_message[1500];

    // Received message buffer
    char rec_message[1500];
    // User's message buffer
    char content[1400];

    // For tokenizing message
    char *parsed_message;
    char *parsed_received_message;

    // Allocate important structures
    //////////////////////////////////////////////
    tcp_auth *auth = malloc(sizeof(tcp_auth));
    if (auth == NULL)
    {
        perror("ERR: auth allocation failed");
        close(socket);
        return ERROR;
    }

    tcp_join *join = malloc(sizeof(tcp_join));
    if (join == NULL)
    {
        perror("ERR: join allocation failed");
        close(socket);
        return ERROR;
    }

    tcp_msg *msg = malloc(sizeof(tcp_msg));
    if (msg == NULL)
    {
        perror("ERR: msg allocation failed");
        close(socket);
        return ERROR;
    }

    server_tcp_msg *server_message = malloc(sizeof(server_tcp_msg));
    if (server_message == NULL)
    {
        perror("ERR: server_msg allocation failed");
        close(socket);
        return ERROR;
    }

    server_tcp_reply *server_rply = malloc(sizeof(server_tcp_reply));
    if (server_rply == NULL)
    {
        perror("ERR: server_reply allocation failed");
        close(socket);
        return ERROR;
    }

    Setup_tcp_msg *setup_msg = malloc(sizeof(Setup_tcp_msg));
    if (setup_msg == NULL)
    {
        perror("ERR: setup_msg allocation failed");
        close(socket);
        return ERROR;
    }
    //////////////////////////////////////////////
    // Set file descriptors the check activity on them
    fd_set readfd;
    FD_ZERO(&readfd);

    FD_SET(socket, &readfd);

    // Set present state at the start
    TCPpresent_state = state_START;

    // Run program until end state or connection termination
    while (TCPpresent_state != state_END && connection)
    {

        // Clear buffers
        bzero(rec_message, sizeof(rec_message));
        bzero(message, sizeof(message));

        // Set file descriptors
        FD_ZERO(&readfd);
        FD_SET(socket, &readfd); // Socket descriptor
        FD_SET(STDIN_FILENO, &readfd); // Stdin descriptor

        // Check activity on the provided file descriptors
        int activity = select(FD_SETSIZE, &readfd, NULL, NULL, NULL);
        if (activity < 0)
        {
            // Got SIGINT
            if (errno == EINTR)
            {
                if (!connection)
                {
                    // Sent BYE message after SIGINT
                    strcpy(send_message, "BYE\r\n");
                    signal(SIGINT, SIG_DFL);
                }

                // Send message to server
                int bytes_tx = send(socket, send_message, strlen(send_message), 0);
                if (bytes_tx < 0)
                {
                    perror("ERR: send");
                    shutdown(socket, SHUT_RDWR);
                    close(socket);
                    return ERROR;
                }
                break; // Exit the loop
            }
            else
            {
                perror("ERR: select failed");
                return ERROR;
            }
        }
        else
        {
            // Got activity on stdin
            // If we already received reply, unlock stdin
            if (FD_ISSET(STDIN_FILENO, &readfd) && gotReply)
            {
                // Parse stdin
                char *in = fgets(message, 1500, stdin);
                if (in == NULL || feof(stdin) || strcmp(message, "\n") == 0)
                {
                    break;
                }
                message[strlen(message) - 1] = '\0';
                parsed_message = strtok(message, " ");

                // Parse stdin command
                command = parse_commandTCP(parsed_message, auth, join, msg);

                // Got valid command
                if (command != ERROR_PARAMS && command != ERROR_STATE && command != HELP && command != RENAME)
                {
                    // Setup parsed message, store it to structure
                    setup_msgT(command, setup_msg, auth, join, msg);

                    // Send set-up message to server
                    int bytes_tx = send(socket, setup_msg->message, strlen(setup_msg->message), 0);
                    if (bytes_tx < 0)
                    {
                        perror("ERR: send");
                        shutdown(socket, SHUT_RDWR);
                        close(socket);
                        return ERROR;
                    }

                    // Lock stdin until reply is received
                    if (command == AUTH || command == JOIN)
                    {
                        gotReply = false;
                    }
                }
            }

            // Got activity on socket
            // Recieve server's response
            if (FD_ISSET(socket, &readfd))
            {
                // Clear buffer
                bzero(rec_message, sizeof(rec_message));

                // Receive message
                int bytes_rx = recv(socket, rec_message, 1420, 0);
                if (bytes_rx < 0)
                {
                    perror("ERR: recv");
                    shutdown(socket, SHUT_RDWR);
                    close(socket);
                    return ERROR;
                }

                // Parse received message
                parsed_received_message = strtok(rec_message, " ");

                // Convert case-insensitive message type to upper case
                char upper_case_token[100];
                strcpy(upper_case_token, parsed_received_message);

                for (int i = 0; upper_case_token[i]; i++)
                {
                    upper_case_token[i] = toupper(upper_case_token[i]);
                }


                // Compare received message type
                bzero(content, sizeof(content));
                if (strcmp(upper_case_token, "MSG") == 0 || strcmp(upper_case_token, "ERR") == 0)
                {

                    int out = 0; // Indicates print format

                    // Got MSG type
                    if (strcmp(upper_case_token, "MSG") == 0)
                    {
                        out = out_msg; // Set print format
                    }
                    // Got ERR type
                    else
                    {
                        out = out_err; // Set print format
                        TCPpresent_state = state_ERROR; // Change present state to error
                    }

                    // Parse received message's parameters
                    int i = 0;
                    while (parsed_received_message != 0)
                    {
                        switch (i)
                        {
                        case 0:
                        case 1:
                        case 3:
                            break;

                        case 2:
                            // Store display name
                            server_message->display_name = strdup(parsed_received_message);
                            break;

                        default:
                            // Store message's content
                            strcat(content, parsed_received_message);
                            strcat(content, " ");
                            break;
                        }
                        i++;
                        parsed_received_message = strtok(0, " ");
                    }

                    // Store content to structure
                    content[strlen(content) - 1] = '\0';
                    server_message->rec_message = content;

                    // Print message
                    printMsg(server_message->display_name, server_message->rec_message, out);
                }

                // Got REPLY message type
                else if (strcmp(upper_case_token, "REPLY") == 0)
                {
                    int i = 0;
                    gotReply = true; // Unlock stdin

                    // Parse received message's parameters
                    while (parsed_received_message != 0)
                    {
                        switch (i)
                        {
                        case 0:
                        case 2:
                            break;

                        case 1:
                            // Reply confirmation value OK/NOK
                            server_rply->confirmation = strdup(parsed_received_message);
                            break;

                        default:
                            // Store content
                            strcat(content, parsed_received_message);
                            strcat(content, " ");
                            break;
                        }
                        i++;
                        parsed_received_message = strtok(0, " ");
                    }

                    // Store content to the structure
                    content[strlen(content) - 1] = '\0';
                    server_rply->rec_reply = content;

                    // Print reply
                    printReply(server_rply->confirmation, server_rply->rec_reply);
                }

                // Got BYE message
                else if (strcmp(upper_case_token, "BYE\r\n") == 0)
                {
                    TCPpresent_state = state_END; // End program
                }

                // Got unknown message type
                else
                {
                    // Set present state to ERROR
                    TCPpresent_state = state_ERROR;
                    fprintf(stderr,"ERR: invalid message type\n");

                    // Send ERR message to server
                    send_errorTCP(socket,auth);
                }
            }

            // Send BYE to server if in the ERROR state
            if (TCPpresent_state == state_ERROR)
            {
                send_byeTCP(socket);
                
                // End program
                TCPpresent_state = state_END;
            }
        }
    }

    // Free allocated memory
    freeInstructions(auth, join, msg);
    free(server_message);
    free(server_rply);

    // Close socket
    shutdown(socket, SHUT_RDWR);
    close(socket);
    return 0;
}

void freeInstructions(tcp_auth *auth, tcp_join *join, tcp_msg *msg)
{
    // Free command's structures
    free(auth);

    free(join);

    free(msg);
}

void printReply(char *confirmation, char *reply)
{
    // Got OK REPLY
    if (strcmp(confirmation, "OK") == 0)
    {
        // Print reply in demanded way
        fprintf(stderr, "Success: %s", reply);
        if (TCPpresent_state == state_AUTH)
        {
            TCPpresent_state = state_OPEN;
        }
    }
    // Got NOK REPLY
    else if (strcmp(confirmation, "NOK") == 0)
    {
        // Print reply in demanded way
        fprintf(stderr, "Failure: %s", reply);
        if (TCPpresent_state == state_AUTH)
        {
            TCPpresent_state = state_AUTH_failed;
        }
    }
}

void printMsg(char *name, char *reply, int out)
{
    // Print message in demanded way
    switch (out)
    {
    case out_msg:
        printf("%s: %s", name, reply);
        break;
    case out_err:
        fprintf(stderr, "ERR FROM %s: %s", name, reply);
    default:
        break;
    }
}
