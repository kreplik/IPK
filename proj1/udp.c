#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "ipk.h"
#include "udp.h"
#include "udp_command.h"
#include "udp_message.h"
#include "udp_receive.h"

// Message type structure
MSG_type message_typeInit;
MSG_type *message_type;

// Init Setup_msg structure
Setup_msg setup_msgInit;
Setup_msg *setup_msg;

uint16_t messageID = 0; // Represents message's unique number

extern Params *params; // Get command line parameters

int present_state; // Represents program's state

void close_udp(int socket)
{
    connection = false; // Terminate connection
}

void initMsg_type(struct MSG_type *message_type)
{
    message_type->CONFIRMtype = 0x00;
    message_type->REPLYtype = 0x01;
    message_type->AUTHtype = 0x02;
    message_type->JOINtype = 0x03;
    message_type->MSGtype = 0x04;
    message_type->ERRtype = 0xFE;
    message_type->BYEtype = 0xFF;
}

int run_udp(int socket, struct sockaddr *address, socklen_t address_size)
{
    // Catch C-c signal
    signal(SIGINT, close_udp);

    int command = 0;      // Command value
    int retryCounter = 0; // Indicates number of retransmission tries
    char sent_opcode;     // Sent message type

    // Stdin buffer
    char message[1500];

    // Sent message buffer
    char send_message[1500];

    // Received message buffer
    char rec_message[1500];

    // Message's content
    char content[1400];

    // Tokenized message
    char *parsed_message;

    message_type = &message_typeInit;
    setup_msg = &setup_msgInit;

    bool messageConfirmed = true; // Indicates, if message was confirmed
    bool needReply = false;       // Indicates, if we need to wait for the reply and lock stdin

    initMsg_type(message_type);

    // Set file descriptors to check activity on them
    fd_set readfd;
    FD_ZERO(&readfd);

    FD_SET(socket, &readfd);

    // Clear buffer
    bzero(message, sizeof(message));

    // Set file descriptors
    FD_ZERO(&readfd);
    FD_SET(socket, &readfd);       // Socket file descriptor
    FD_SET(STDIN_FILENO, &readfd); // Stdin file descriptor

    // Allocate important structures
    ///////////////////////////////////////////////////////
    udp_auth *auth;
    udp_join *join;
    user_msg *msg;

    server_msg *server_message = malloc(sizeof(server_msg));
    if (server_message == NULL)
    {
        perror("ERR: server_msg allocation failed");
        close(socket);
        return ERROR;
    }

    server_reply *server_rply = malloc(sizeof(server_reply));
    if (server_rply == NULL)
    {
        perror("ERR: server_reply allocation failed");
        close(socket);
        return ERROR;
    }

    confirm *conf = malloc(sizeof(confirm));
    if (conf == NULL)
    {
        perror("ERR: confirm allocation failed");
        close(socket);
        return ERROR;
    }

    auth = malloc(sizeof(udp_auth));
    if (auth == NULL)
    {
        perror("ERR: auth allocation failed");
        close(socket);
        return ERROR;
    }

    join = malloc(sizeof(udp_join));
    if (join == NULL)
    {
        perror("ERR: join allocation failed");
        close(socket);
        return ERROR;
    }

    msg = malloc(sizeof(user_msg));
    if (msg == NULL)
    {
        perror("ERR: msg allocation failed");
        close(socket);
        return ERROR;
    }
    ///////////////////////////////////////////////////////

    present_state = state_START; // Set starting state
    bool enable_reading = true;  // Indicates EOF

    struct timeval start_time, current_time; // Timer for retransmission

    // Start running program
    while (present_state != state_END && connection)
    {
        // Clear buffers
        bzero(message, sizeof(message));
        bzero(send_message, sizeof(send_message));

        // Set file descriptors
        FD_ZERO(&readfd);
        FD_SET(socket, &readfd);
        FD_SET(STDIN_FILENO, &readfd);

        // Check for the activity
        int activity = select(FD_SETSIZE, &readfd, NULL, NULL, NULL);
        if (activity < 0)
        {
            if (errno == EINTR)
            {
                // Got SIGINT
                signal(SIGINT, SIG_DFL);
                present_state = state_END;
            }
            else
            {
                perror("ERR: select failed");
                break;
            }
        }
        else if (activity == 0) // No activity at file descriptors
        {
            present_state = state_END;
        }
        else
        {
            // Got activity on stdin
            // If message was confirmed and reply received, unlock stdin
            if (FD_ISSET(STDIN_FILENO, &readfd) && (messageConfirmed && !needReply))
            {
                // Parse stdin
                char *in = fgets(message, sizeof(message), stdin);
                if (in == NULL || feof(stdin) || strcmp(message, "\n") == 0)
                {
                    // Got EOF
                    enable_reading = false;
                    present_state = state_ERROR; // Results in sending bye message and ending communication
                }
                // Got empty message
                else if (message[0] == '\n' || message[0] == ' ')
                {
                    enable_reading = false;
                }

                if (enable_reading)
                {
                    // Parse stdin message
                    message[strlen(message) - 1] = '\0';
                    parsed_message = strtok(message, " ");

                    ///////////////////////
                    // Parse command from stdin
                    command = parse_command(parsed_message, auth, join, msg);

                    // If the command was valid, setup the final message
                    sent_opcode = message_type->MSGtype; // Default value
                    if (command != RENAME && command != ERROR_PARAMS && command != ERROR_STATE && command != HELP)
                    {

                        setup_msg->command = command;     // Store command value
                        setup_msg->messageID = messageID; // Store message ID

                        //////////////////////////////////////////////////
                        // Setup final message that is about to be sent
                        setup_message(setup_msg, auth, join, msg, message_type);

                        messageID++;                     // Increment message ID for next message
                        messageConfirmed = false;        // Lock stdin
                        gettimeofday(&start_time, NULL); // Start timer

                        // Send message to server
                        sent_opcode = sender(setup_msg->message_length + 1, setup_msg->send_message, socket, address, address_size);
                    }
                }
                enable_reading = true;
            }

            // Got activity on socket
            // Parse received message
            if (FD_ISSET(socket, &readfd))
            {
                // Receive server's response
                // Clear buffer
                bzero(rec_message, sizeof(rec_message));
                int bytes_rx = recvfrom(socket, rec_message, 1500, 0, address, &address_size);
                if (bytes_rx < 0)
                {
                    perror("ERR: recvfrom\n");
                    shutdown(socket, SHUT_RDWR);
                    close(socket);
                    return ERROR;
                }

                int opcode = 0; // Received message type
                // Clear buffers
                bzero(content, sizeof(content));
                server_message->rec_message = NULL;

                // Parse received message
                opcode = parse_message(rec_message, server_rply, server_message, conf, message_type);

                // If we sent AUTH or JOIN message, wait for the reply and lock stdin
                if ((sent_opcode == message_type->AUTHtype) || sent_opcode == message_type->JOINtype)
                {
                    needReply = true;
                }

                // Print received message in demanded way
                switch (opcode)
                {
                case 0:
                    // Got confirmation of sent message
                    if (conf->replyID == setup_msg->messageID)
                    {
                        messageConfirmed = true; // Partially unlock stdin
                    }
                    break;

                case 1:
                    // Got REPLY
                    if (server_rply->referenceID == conf->replyID)
                    {
                        // Print OK reply
                        if (server_rply->result[0] == 0x01)
                        {
                            fprintf(stderr, "Success: %s\n", server_rply->rec_reply);
                            present_state = state_OPEN; // Continue in the open state
                        }
                        else
                        {
                            // Print NOK reply
                            fprintf(stderr, "Failure: %s\n", server_rply->rec_reply);
                            if (present_state == state_AUTH)
                            {
                                // Got reply for failed authentication
                                present_state = state_AUTH_failed;
                            }
                        }

                        // Send CONFIRM for receiving this message
                        send_confirm(server_rply->replyID, socket, address, address_size);
                        needReply = false; // Partially unlock stdin
                    }
                    break;

                case 4:
                    // Received MSG type
                    // Print message in demanded way
                    fprintf(stdout, "%s: %s\n", server_message->display_name, server_message->rec_message);

                    // Send confirmation
                    send_confirm(server_rply->replyID, socket, address, address_size);
                    break;

                case -2:
                    // Received ERR type
                    // Print ERR in demanded way
                    fprintf(stderr, "ERR FROM %s: %s\n", server_message->display_name, server_message->rec_message);

                    // Send confirmation
                    send_confirm(server_rply->replyID, socket, address, address_size);
                    present_state = state_ERROR; // Change state
                    break;

                case -1:
                    // Received BYE message
                    // End program
                    present_state = state_END;
                    break;

                default:
                    // Received unknown message type
                    // Print ERROR
                    present_state = state_ERROR; // Change state
                    fprintf(stderr, "ERR: received invalid message\n");

                    // Send ERROR to the server
                    send_error(messageID++, socket, address, address_size, message_type, auth);
                    break;
                }

                // Too many retransmission tries
                if (retryCounter == params->retry)
                {
                    present_state = state_END; // End program
                }

                if (present_state == state_ERROR)
                {
                    // Send BYE
                    send_bye(messageID, socket, address, address_size);

                    // End program
                    present_state = state_END;
                }
            }

            // Get the timer
            gettimeofday(&current_time, NULL);

            long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                                (current_time.tv_usec - start_time.tv_usec) / 1000;

            // Check the timer
            // If message was not confirmed, resend it
            if (!messageConfirmed && retryCounter < params->retry && elapsed_time >= params->timeout * 1000)
            {
                sender(setup_msg->message_length, setup_msg->send_message, socket, address, address_size);
                retryCounter++;
            }
        }
    }

    // Got SIGINT
    if (!connection)
    {
        // Send BYE and end program
        messageID++;
        send_bye(messageID, socket, address, address_size);
    }

    // Close socket
    shutdown(socket, SHUT_RDWR);
    close(socket);

    // Free allocated memory
    free(server_rply);
    free(server_message);
    free(conf);

    free(auth);

    free(join);

    free(msg);

    return 0;
}
