#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "udp.h"
#include "udp_command.h"

extern int present_state; // Program's state

// Regex patterns
static const char* UserChannelSecretUDP = "^[A-Za-z0-9.-]+$";
static const char* DisplayNameUDP = "^[\x21-\x7E]+$";
static const char* MsgRegUDP = "^[\x20-\x7E]+$";

int parse_command(char *parsed_message, struct udp_auth *auth, struct udp_join *join, struct user_msg *msg)
{
    if(!check_command(parsed_message))
    {
        fprintf(stderr,"ERR: unknown command\n"); // Unknown command parsed
        return ERROR_PARAMS;
    }

    int i = 0;
    char content[1400];
    int command = 0;
    bool regex_check = true;

    // Authentication command
    if (strcmp(parsed_message, "/auth") == 0)
    {
        // Unable to authenticate user
        if(present_state != state_START && present_state != state_AUTH_failed){
            fprintf(stderr,"ERR: user already authenticated\n");
            return ERROR_STATE;
        }
        //
        // Parse command's parameters
        while (parsed_message != 0)
        {
            switch (i)
            {
            case 0:
                command = AUTH; // Command value
                break;

            case 1:
                regex_check = regexUDP(parsed_message,UserChannelSecretUDP); // Check if parameter is valid
                auth->user_name = strdup(parsed_message); // Store parameter
                break;

            case 2:
                regex_check = regexUDP(parsed_message,UserChannelSecretUDP); // Check if parameter is valid
                auth->secret = strdup(parsed_message); // Store parameter
                break;

            case 3:
                regex_check = regexUDP(parsed_message,DisplayNameUDP); // Check if parameter is valid
                auth->display_name = strdup(parsed_message); // Store parameter
                break;

            default:
                break;
            }

            i++;
            parsed_message = strtok(0, " ");

            if(!regex_check)
            {
                // One of the parameters is invalid
                fprintf(stderr,"ERR: wrong characters in parameters\n");
                return ERROR_PARAMS;
            }
        }

        // Too many command's parameters
        if(i != 4){
            fprintf(stderr,"ERR: wrong amount of authentication parameters\n");
            return ERROR_PARAMS;
        }
        present_state = state_AUTH; // Change present state
    }

    // Join command
    else if (strcmp(parsed_message, "/join") == 0)
    {
        // Unable to join channel in this state
        if(present_state != state_OPEN){
            fprintf(stderr,"ERR: authentication needed first\n");
            return ERROR_STATE;
        }

        // Parse command's parameters
        while (parsed_message != 0)
        {
            switch (i)
            {
            case 0:
                command = JOIN; // Command value
                break;

            case 1:
                regex_check = regexUDP(parsed_message,UserChannelSecretUDP); // Check if parameter is valid
                join->channel = strdup(parsed_message); // Store parameter
                break;

            default:
                break;
            }

            i++;
            parsed_message = strtok(0, " ");

            if(!regex_check)
            {
                // One of the parameters is invalid
                fprintf(stderr,"ERR: wrong characters in parameters\n");
                return ERROR_PARAMS;
            }
        }

        // Too many command's parameters
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
                regex_check = regexUDP(parsed_message,DisplayNameUDP); // Check if parameter is valid
                auth->display_name = strdup(parsed_message); // Store parameter
                break;

            default:
                break;
            }

            i++;
            parsed_message = strtok(0, " ");
        }
        
        // Too many command's parameters
        if(i != 2){
            fprintf(stderr,"ERR: wrong amount of rename parameters\n");
            return ERROR_PARAMS;
        }
    }

    // Help command
    else if(strcmp(parsed_message,"/help") == 0){
        printHelp(); // Print help
        command = HELP;
    }

    // User's message
    else
    {
        // Unable to send message in this state
        if(present_state == state_START){
            fprintf(stderr,"ERR: authentication needed first\n");
            return ERROR_STATE;
        }

        bzero(content, sizeof(content));

        // Parsed BYE message
        if (strcmp(parsed_message, "BYE") == 0)
        {
            present_state = state_END; // End program
            command = BYE;
        }
        else
        {
            // User's message
            if(present_state != state_OPEN){
                // Unable to send message in non-open state
                fprintf(stderr,"ERR: you are not able to send messages\n");
                return ERROR_STATE;
            }
            command = MSG;
        }

        // Parse user's message
        while (parsed_message != 0)
        {
            regex_check = regexUDP(parsed_message,MsgRegUDP); // Check if parameter is valid

            // Store message in content buffer
            strcat(content, parsed_message);
            strcat(content, " ");
            parsed_message = strtok(0, " ");

            if(!regex_check)
            {
                // One of the messsage part is not valid
                fprintf(stderr,"ERR: wrong characters in parameters\n");
                return ERROR_PARAMS;
            }
        }

        // Store content to structure
        content[strlen(content) - 1] = '\0';
        msg->user_message = strdup(content);
    }


    return command; // Return command value
}

bool check_command(char* parsed_message)
{
    bool result = true;
    char c = parsed_message[0];

    // Found command character
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

    // If the command is not any of the expected ones, return false
    return result;
}

bool regexUDP(char* parsed_message, const char* pattern)
{
    regex_t regex;

    // Compile regex
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0) {
        return false;
    }

    // Compare parameter with the regex pattern
    ret = regexec(&regex, parsed_message, 0, NULL, 0);
    if (ret == 0)
    {
        return true;
    }

    regfree(&regex);

    // Return true, if parameter matches regex pattern
    return false;
}
