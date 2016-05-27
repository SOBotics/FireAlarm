//
//  misc_commands.h
//  chatbot
//
//  Created on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef misc_commands_h
#define misc_commands_h

void listCommands(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    postReply(bot->room, "Running commands:", command->message);
    const size_t maxLineSize = 256;
    pthread_mutex_lock(&bot->runningCommandsLock);
    char *messageString = malloc(maxLineSize * (bot->runningCommandCount + 2));
    strcpy(messageString,
           "          User      |"
           "                      Command                      \n"
           "    ---------------------"
           "---------------------------------------------------\n"
           );
    for (int i = 0; i < bot->runningCommandCount; i++) {
        const size_t maxUsernameLength = 16;
        const size_t maxCommandLength = 48;
        
        char username[strlen(bot->runningCommands[i]->message->user->name) + 1];
        strcpy(username, bot->runningCommands[i]->message->user->name);
        char message[strlen(bot->runningCommands[i]->message->content) + 1];
        strcpy(message, bot->runningCommands[i]->message->content);
        
        //max username length = 16 chars
        if (strlen(username) > maxUsernameLength) {
            username[maxUsernameLength-3] = '.';
            username[maxUsernameLength-2] = '.';
            username[maxUsernameLength-1] = '.';
            username[maxUsernameLength] = '\0';
        }
        //max message length = 48 chars
        if (strlen(message) > maxCommandLength) {
            message[maxCommandLength-3] = '.';
            message[maxCommandLength-2] = '.';
            message[maxCommandLength-1] = '.';
            message[maxCommandLength] = '\0';
        }
        
        snprintf(messageString + strlen(messageString), maxLineSize,
                 "    %*s%*s|%*s%*s\n",
                 (int)((maxUsernameLength/2)+strlen(username)/2),
                 username,
                 (int)((maxUsernameLength/2)-strlen(username)/2),
                 "",
                 
                 
                 (int)((maxCommandLength/2)+strlen(message)/2),
                 message,
                 (int)((maxCommandLength/2)-strlen(message)/2),
                 ""
                 );
    }
    postMessage(bot->room, messageString);
    free(messageString);
    pthread_mutex_unlock(&bot->runningCommandsLock);
}

void help (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    postReply (bot->room, "[I am a bot](https://github.com/NobodyNada/chatbot) which detects questions that need closing whenever they are posted or edited. ", command->message);
    return;
}

void aliveCheck (RunningCommand *command, void *ctx) 
{
    ChatBot *bot = ctx;
    postReply (bot->room, "Why did you think I was dead?", command->message);
    return;
}

void commandList (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    postReply (bot->room, "To see a complete list of commands, visit [this page](https://github.com/NobodyNada/chatbot/wiki/Commands).", command->message);
    return;
}

unsigned int statistics (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    char message [200];
    unsigned int i = 0;
    unsigned int truePositives = 0;
    unsigned int falsePositives = 0;
    unsigned int unConfirmed = 0;
    int check = 2;
    int numStats = strtol (command->argv [0], NULL, 5);
    
    Report **reports = bot->latestReports;
    
    /* Checking for errors */
    
    if (numStats <= 0)
    {
        postReply (bot->room, "Please enter a number bigger than 0.", command->message);
        return 1;
    }
    else if (numStats > 100)
    {
        postReply (bot->room, "Please enter a number smaller than 100.", command->message);
        return 1;
    }
    else if (bot->latestReports [0] == NULL)
    {
        postReply (bot->room, "There are no reports available.");
        return 1;
    }
    
    /* Calculating the number of truePositives, falsePositives, and unconfirmed reports */
    
    for (; i < numStats; ++i)
    {
        Report *report = reports [i];
        check = report->confirmation;
        
        if (check == 0)
        {
            falsePositives ++;
        }
        else if (check == 1)
        {
            truePositives ++;
        }
        else
        {
            unconfirmed ++;
        }
    }
    
    sprintf (message, "Statistics of last %d reports: %d True Positives    %d False Positives    %d Un-Confirmed", 
             numStats, truePositives, falsePositives, unConfirmed);
             
    postReply (bot->room, message, command->message);
    
    return 0;
}

#endif /* misc_commands_h */
