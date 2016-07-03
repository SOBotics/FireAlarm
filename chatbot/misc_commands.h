//
//  misc_commands.h
//  chatbot
//
//  Created on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef misc_commands_h
#define misc_commands_h

#include "Privileges.h"

void listCommands(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
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
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    postReply (bot->room, "I'm  [Fire Alarm](https://github.com/NobodyNada/chatbot), a bot which detects questions that need closing whenever they are posted or edited. [My command list is available here.](https://github.com/NobodyNada/chatbot/wiki/Commands)", command->message);
    return;
}

void aliveCheck (RunningCommand *command, void *ctx) 
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    postReply (bot->room, "Why did you think I was dead?", command->message);
    return;
}

void commandList (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    postReply (bot->room, "To see a complete list of commands, visit [this page](https://github.com/NobodyNada/chatbot/wiki/Commands).", command->message);
    return;
}

void changeThreshold (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    long newThreshold = strtol (command->argv [0], NULL, 10);
    
    if (newThreshold > 10000)
    {
        postReply (bot->room, "Please enter a threshold smaller than 10000.", command->message);
        return;
    }
    else if (newThreshold < 100)
    {
        postReply (bot->room, "Please enter a threshold bigger than 100.", command->message);
        return;
    }
    
    THRESHOLD = newThreshold;
    return;
}

void checkThreshold (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    char message [256];
    
    sprintf (message, "The current threshold is %ld.", THRESHOLD);
    
    postReply (bot->room, message, command->message);
    return;
}

void optIn (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    Notify **notify = bot->notify;
    
    notify[bot->totalNotifications - 1] = createNotification (0, command->message->user->userID);
    bot->totalNotifications ++;
    
    postReply (bot->room, "Opt-in successful. You will now be pinged for reports.", command->message);
    
    return;
}

void notifyMe (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    Notify **notify = bot->notify;
    
    notify[bot->totalNotifications - 1] = createNotification (1, command->message->user->userID);
    bot->totalNotifications ++;
    
    postReply (bot->room, "Notify submission successful. You will now be notified for reports.", command->message);
    
    return;
}

#endif /* misc_commands_h */
