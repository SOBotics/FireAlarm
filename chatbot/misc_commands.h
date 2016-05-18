//
//  misc_commands.h
//  chatbot
//
//  Created by Jonathan Keller on 5/9/16.
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

#endif /* misc_commands_h */
