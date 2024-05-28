#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "tcp.h"
#include "tcp_command.h"
#include "ipk.h"

// Program's present state
extern int TCPpresent_state;

// Regular expression's patterns
static const char* UserChannelSecret = "^[A-Za-z0-9.-]+$";
static const char* DisplayName = "^[!-~]+$";
static const char* MsgReg = "^[\x20-\x7E]+$";

int parse_commandTCP(char *parsed_message, struct tcp_auth *auth, struct tcp_join *join, struct tcp_msg *msg)
{
    if(!check_commandTCP(parsed_message))
    {
        fprintf(stderr,"ERR: unknown command\n"); // parsed unknown command
        return ERROR_PARAMS;
    }

    int i = 0;
    char content[1400];
    int command = 0;
    bool regex_check = true;

    // Authentication command
    if (strcmp(parsed_message, "/auth") == 0)
    {
        // Unable to parse authentication command
        if(TCPpresent_state != state_START && TCPpresent_state != state_AUTH_failed){
            fprintf(stderr,"ERR: user already authenticated\n");
            return ERROR_STATE;
        }
        while (parsed_message != 0)
        {
            switch (i)
            {
            case 0:
                command = AUTH; // command value
                break;

            case 1:
                regex_check = regex(parsed_message,UserChannelSecret); // Check if parameter is valid
                
                auth->user_name = strdup(parsed_message); // Store user name
                break;

            case 2:
                regex_check = regex(parsed_message,UserChannelSecret); // Check if parameter is valid

                auth->secret = strdup(parsed_message); // Store secret
                break;

            case 3:
                regex_check = regex(parsed_message,DisplayName); // Check if parameter is valid
                
                auth->display_name = strdup(parsed_message); // Store display name
                break;

            default:
                break;
            }

            i++;
            parsed_message = strtok(0, " ");
            // One of the parameters is not valid
            if(!regex_check)
            {
                fprintf(stderr,"ERR: wrong characters in parameters\n");
                return ERROR_PARAMS;
            }
        }
        // Wrong number of parameters found
        if(i != 4){
            fprintf(stderr,"ERR: wrong amount of authentication parameters\n");
            return ERROR_PARAMS;
        }
        TCPpresent_state = state_AUTH; // Change present state
    }

    // Joind command
    else if (strcmp(parsed_message, "/join") == 0)
    {
        // Unable to join channel in this state
        if(TCPpresent_state != state_OPEN){
            fprintf(stderr,"ERR: join is not possible\n");
            return ERROR_STATE;
        }

        while (parsed_message != 0)
        {
            switch (i)
            {
            case 0:
                command = JOIN; // Command value
                break;

            case 1:
                regex_check = regex(parsed_message,UserChannelSecret); // Check if parameter in valid

                join->channel = strdup(parsed_message); // Store channel's name
                break;

            default:
                break;
            }

            i++;
            parsed_message = strtok(0, " ");
            // One of the parameter is not valid
            if(!regex_check)
            {
                fprintf(stderr,"ERR: wrong characters in parameters\n");
                return ERROR_PARAMS;
            }
        }
        // Wrong number of parameters
        if(i != 2){
            fprintf(stderr,"ERR: wrong amount of join parameters\n");
            return ERROR_PARAMS;
        }
    }

    // Rename command
    else if(strcmp(parsed_message,"/rename") == 0){
        while (parsed_message != 0)
        {
            switch (i)
            {
            case 0:
                command = RENAME; // Command value
                break;

            case 1:
                regex_check = regex(parsed_message,DisplayName); // Check if parameter is valid

                auth->display_name = strdup(parsed_message); // Store display name
                break;

            default:
                break;
            }

            i++;
            parsed_message = strtok(0, " ");
            // One of the parameter is not valid
            if(!regex_check)
            {
                fprintf(stderr,"ERR: wrong characters in parameters\n");
                return ERROR_PARAMS;
            }
        }
        // Wrong number of parameters
        if(i != 2){
            fprintf(stderr,"ERR: wrong amount of rename parameters\n");
            return ERROR_PARAMS;
        }
    }
    // Help command
    else if(strcmp(parsed_message,"/help") == 0)
    {
        printHelp(); // Print help
        command = HELP;
    }

    // User's message
    else
    {
        // Unable to send a message in this state
        if(TCPpresent_state == state_START){
            fprintf(stderr,"ERR: authentication needed first\n");
            return ERROR_STATE;
        }

        bzero(content, sizeof(content));

        // Parsed BYE message
        if (strcmp(parsed_message, "BYE") == 0)
        {
            TCPpresent_state = state_END; // Set present state
            command = BYE;
        }
        else
        {
            // Unable to parse message in this state
            if(TCPpresent_state != state_OPEN){
                fprintf(stderr,"ERR: you are not able to send messages\n");
                return ERROR_STATE;
            }
            command = MSG;
        }

        // Store content of the message
        while (parsed_message != 0)
        {
            regex_check = regex(parsed_message,MsgReg);
            strcat(content, parsed_message);
            strcat(content, " ");
            parsed_message = strtok(0, " ");

            // Check if message is valid
            if(!regex_check)
            {
                fprintf(stderr,"ERR: wrong characters in parameters\n");
                return ERROR_PARAMS;
            }
        }

        content[strlen(content) - 1] = '\0';
        msg->user_message = strdup(content);
    }
    return command; // Return command's value
}

bool check_commandTCP(char* parsed_message)
{
    bool result = true;
    char c = parsed_message[0];
    // Found command
    if(strcmp(&c,"/") == 0)
    {
        result = false;
        if(strcmp(parsed_message,"/auth") == 0)
        {
            result = true;
        }

        else if(strcmp(parsed_message,"/join") == 0)
        {
            result = true;
        }

        else if(strcmp(parsed_message,"/rename") == 0)
        {
            result = true;
        }

        else if(strcmp(parsed_message,"/help") == 0)
        {
            result = true;
        }
    }
    // If the command is none of the expected one's, return false
    return result;
}

bool regex(char* parsed_message, const char* pattern)
{
    regex_t regex;
    // Compile regex
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0) {
        return false;
    }

    // Compare parameter with the regex
    ret = regexec(&regex, parsed_message, 0, NULL, 0);
    if (ret == 0)
    {
        return true;
    }

    regfree(&regex);
    return false;
}
