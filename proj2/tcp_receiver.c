#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "tcp_receiver.h"


int receive_message(struct Client_message *client, char *buffer, struct sockaddr_in client_addr)
{
    char *bufferUP;
    char tmpBuffer[1500];
    bzero(tmpBuffer, sizeof(tmpBuffer));
    bzero(client->message,sizeof(client->message));

    // Parse received message's parameters

    // Capitalize copy of message part
    strcpy(tmpBuffer, buffer);
    bufferUP = strtok(tmpBuffer, " ");
    for (int i = 0; bufferUP[i]; i++)
    {
        bufferUP[i] = toupper(bufferUP[i]);
    }

    // Result of parsing
    int result = 0;

    // Check each message part
    if (strcmp(bufferUP, AUTH_part) == 0)
    {
        client->message_type = AUTH;
    }
    else if (strcmp(bufferUP, JOIN_part) == 0)
    {
        client->message_type = JOIN;
    }
    else if (strcmp(bufferUP, MSG_part) == 0)
    {
        client->message_type = MSG;
    }
    else if (strcmp(bufferUP, ERR_part) == 0)
    {
        client->message_type = ERR;
    }
    else if (strcmp(bufferUP, BYE_part) == 0)
    {
        client->message_type = BYE;
        bufferUP[strlen(bufferUP)-1] = '\0';
    }
    else{
        client->message_type = UNKNOWN;
    }

    // Parse whole message
    result = parse_message(buffer, client);
    
    printf("RECV %s:%d | %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), bufferUP);
    
    // Return parsing result
    return result;
}

int parse_message(char *parsed_message, struct Client_message *client)
{
    int result = 0;
    switch (client->message_type)
    {
    case AUTH:
        if (client->client_state != state_ACCEPT)
        {
            // User already authenticated
            return ERROR_STATE;
        }

        // Parse AUTH message

        // Set client's state
        if ((result = parse_auth(parsed_message, client)) != SUCCESS)
        {
            client->authenticated = false;
            client->client_state = state_AUTH_failed;
        }
        else
        {
            client->authenticated = true;
            client->client_state = state_OPEN;
        }
        break;

    case JOIN:
        if(client->client_state != state_OPEN)
        {
            return ERROR_STATE;
        }

        // Parse JOIN message
        result = parse_join(parsed_message,client);
        break;

    case BYE:
        // Received BYE
        result = SUCCESS;
        break;

    case MSG:
        if (client->client_state != state_OPEN)
        {
            return ERROR_STATE;
        }

        // Parse MSG message
        result = parse_msg(parsed_message, client);
        break;
    
    case UNKNOWN:
        // Set client's state if received unknown type
        client->client_state = state_ERROR;
        result = SUCCESS; // Sends error message later
    }
    return result;
}

bool message_checker(char *message_part, char *pattern)
{
    char message_part_UP[100];
    char patter_UP[100];
    bzero(message_part_UP, sizeof(message_part_UP));
    bzero(patter_UP, sizeof(patter_UP));

    // Capitalize message part
    strcpy(message_part_UP, message_part);
    for (int i = 0; message_part_UP[i]; i++)
    {
        message_part_UP[i] = toupper(message_part_UP[i]);
    }

    // Capitalize patter to compare with
    strcpy(patter_UP, pattern);
    for (int i = 0; patter_UP[i]; i++)
    {
        patter_UP[i] = toupper(patter_UP[i]);
    }

    // Compare passage part with pattern
    if (strcmp(message_part, pattern) != 0)
    {
        // Comparision failed
        return false;
    }

    // Comparision passed
    return true;
}

int parse_auth(char *buffer, struct Client_message *client)
{
    int i = 0;
    bool part_checker = true;

    // Store message parameters to user's structure
    char *parsed_message = strtok(buffer, " ");
    while (parsed_message != 0)
    {
        switch (i)
        {
        case 0:
            // Message type checked earlier
            break;
        case 1:
            // Store username
            client->username = strdup(parsed_message);
            break;

        case 2:
            // Compare message part with expected part
            part_checker = message_checker(parsed_message, AS_part);
            break;

        case 3:
            // Store display name
            client->displayname = strdup(parsed_message);
            break;

        case 4:
            // Compare message part with expected part
            part_checker = message_checker(parsed_message, USING_part);
            break;

        case 5:
            // Store secret
            client->secret = strdup(parsed_message);
            break;

        default:
            // Wrong number of message parts
            fprintf(stderr, "ERR: message cannot be parsed\n");
            return ERROR_PARSE;
            break;
        }
        if (!part_checker)
        {
            // Message part's not as expected
            fprintf(stderr, "ERR: message cannot be parsed\n");
            return ERROR_PARSE;
        }
        i++;
        parsed_message = strtok(0, " ");
    }

    return SUCCESS;
}

int parse_join(char *buffer, struct Client_message *client)
{
    int i = 0;
    bool part_checker = true;

    char *parsed_message = strtok(buffer, " ");
    while (parsed_message != 0)
    {
        switch (i)
        {
        case 0:
            // Message type checked earlier
            break;
        case 1:
            client->channel = strdup(parsed_message);
            break;

        case 2:
            // Compare message part with expected part
            part_checker = message_checker(parsed_message, AS_part);
            break;

        case 3:
            // Store display name
            parsed_message[strlen(parsed_message) -2] = '\0';
            client->displayname = strdup(parsed_message);
            break;

        default:
            // Wrong number of message parts
            fprintf(stderr, "ERR: message cannot be parsed\n");
            return ERROR_PARSE;
            break;
        }
        if (!part_checker)
        {
            // Message part's not as expected
            fprintf(stderr, "ERR: message cannot be parsed\n");
            return ERROR_PARSE;
        }
        i++;
        parsed_message = strtok(0, " ");
    }

    return SUCCESS;
}

int parse_msg(char *buffer, struct Client_message *client)
{
    bool part_checker = true;
    char content[1500];

    char *parsed_message = strtok(buffer, " ");
    int i = 0;
    while (parsed_message != 0)
    {
        switch (i)
        {
        case 0:
            // Message type checked earlier
            break;

        case 1:
            // Compare message part with expected part
            part_checker = message_checker(parsed_message, FROM_part);
            break;

        case 2:
            // Store display name
            client->displayname = strdup(parsed_message);
            break;

        case 3:
            // Compare message part with expected part
            part_checker = message_checker(parsed_message, IS_part);
            break;

        default:
            // Store message's content
            strcat(content, parsed_message);
            strcat(content, " ");
            break;
        }

        if (!part_checker)
        {
            // Message part's not as expected
            fprintf(stderr, "ERR: message cannot be parsed\n");
            return ERROR_PARSE;
        }
        i++;
        parsed_message = strtok(0, " ");
    }

    // Store message's content
    content[strlen(content) - 2] = '\0';
    memcpy(client->message,strdup(content),strlen(content));

    return SUCCESS;
}
