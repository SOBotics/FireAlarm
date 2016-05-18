//
//  ChatBot.c
//  chatbot
//
//  Created by Jonathan Keller on 5/5/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "ChatBot.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ChatMessage.h"

ChatBot *createChatBot(ChatRoom *room, Command **commands) {
    ChatBot *c = malloc(sizeof(ChatBot));
    c->room = room;
    c->commands = commands;
    c->runningCommands = NULL;
    c->runningCommandCount = 0;
    c->stopAction = ACTION_NONE;
    pthread_mutex_init(&c->runningCommandsLock, NULL);
    return c;
}

void runCommand(ChatBot *bot, ChatMessage *message, char *command) {
    //Get the space-separated components of this command.
    char **components = NULL;
    size_t componentCount = 0;
    
    char *component;
    while ((component = strsep(&command, " "))) {
        //add command to components
        components = realloc(components, (++componentCount) * sizeof(char*));
        components[componentCount-1] = malloc(strlen(component) + 1);
        strcpy(components[componentCount-1], component);
    };
    pthread_mutex_lock(&bot->runningCommandsLock);
    RunningCommand *c = launchCommand(message, (int)componentCount, components, bot->commands, bot);
    bot->runningCommands = realloc(bot->runningCommands, ++bot->runningCommandCount * sizeof(RunningCommand *));
    bot->runningCommands[bot->runningCommandCount-1] = c;
    pthread_mutex_unlock(&bot->runningCommandsLock);
}

void processMessage(ChatBot *bot, ChatMessage *message) {
    char *messageText = malloc(strlen(message->content) + 1);
    strcpy(messageText, message->content);
    if (strstr(messageText, "@Bot") == messageText) {
        //messageText starts with "@Bot"
        char *command = strchr(messageText, ' ');
        if (command) {
            while (*(++command) == ' ');
            if (*command && bot->stopAction == ACTION_NONE) {
                runCommand(bot, message, command);
                free(messageText);
                return;
            }
        }
    }
    deleteChatMessage(message);
    free(messageText);
}

StopAction runChatBot(ChatBot *c) {
    ChatMessage **messages = processChatRoomEvents(c->room);
    ChatMessage *message;
    for (int i = 0; (message = messages[i]); i++) {
        processMessage(c, message);
    }
    free(messages);
    
    //clean up old commands
    for (int i = 0; i < c->runningCommandCount; i++) {
        if (c->runningCommands[i]->finished) {
            //delete the command
            c->runningCommandCount--;
            int j = i;
            for (deleteRunningCommand(c->runningCommands[j]); j < c->runningCommandCount; j++) {
                c->runningCommands[i] = c->runningCommands[i+1];
            }
            c->runningCommands = realloc(c->runningCommands, c->runningCommandCount * sizeof(RunningCommand*));
        }
    }
    if (c->stopAction != ACTION_NONE) {
        if (c->room->pendingMessageLinkedList == NULL && (c->runningCommandCount == 0)) {
            return c->stopAction;
        }
    }
    
    return ACTION_NONE;
}