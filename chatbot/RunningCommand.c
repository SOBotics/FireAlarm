//
//  RunningCommand.c
//  chatbot
//
//  Created by Jonathan Keller on 5/4/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "RunningCommand.h"
#include "Command.h"

struct CommandThreadInfo {
    RunningCommand *command;
    Command **commandList;
    void *ctx;
};

void *commandThread(void *arg) {
    struct CommandThreadInfo *info = arg;
    RunningCommand *command = info->command;
    Command **commandList = info->commandList;
    void *ctx = info->ctx;
    free(info);
    
    char **argumentList = NULL;
    unsigned argListSize = 0;
    unsigned commandToRun = 0;
    
    for (int i = 1; commandList[i]; i++) {  //command 0 is the unrecognized command
        char *name = malloc(strlen(commandList[i]->name) + 1);
        strcpy(name, commandList[i]->name);
        char *name_orig = name; //so we can free it
        
        unsigned match = 1;
        char *token;
        int j;
        for (j = 0; (token = strsep(&name, " ")); j++) {
            if (j == command->argc) {
                match = 0;
                break;
            }
            if (strcmp("*", token) == 0) {  //if we hit an argument, add it to the list.
                argumentList = realloc(argumentList, ++argListSize * sizeof(char*));
                argumentList[argListSize-1] = malloc(strlen(command->argv[j]) + 1);
                strcpy(argumentList[argListSize-1], command->argv[j]);
                continue;
            }
            if (strcmp("...", token) == 0) {
                //if everything else is arguments, add them to the argument list.
                for (; j < command->argc; j++) {
                    argumentList = realloc(argumentList, ++argListSize * sizeof(char*));
                    argumentList[argListSize-1] = malloc(strlen(command->argv[j]) + 1);
                    strcpy(argumentList[argListSize-1], command->argv[j]);
                }
                break;
            }
            if (strcmp(command->argv[j], token)) {
                match = 0;
                break;
            }
        }
        if (j < command->argc) {
            match = 0;
        }
        free(name_orig);
        if (match) {
            commandToRun = i;
            
            break;
        }
    }
    
    //Setup the command parameters
    command->command = commandList[commandToRun];
    
    for (int i = 0; i < command->argc; i++) {
        free(command->argv[i]);
    }
    free(command->argv);
    command->argv = argumentList;
    command->argc = argListSize;
    
    
    //Run the command
    command->command->callback(command, ctx);
    
    
    //Mark the command as completed so it will be cleaned up later
    command->finished = 1;
    return NULL;
}

RunningCommand *launchCommand(ChatMessage *msg, int argc, char **argv, Command **commandList, void *ctx) {
    RunningCommand *c = malloc(sizeof(RunningCommand));
    c->argc = argc;
    c->argv = argv;
    c->message = msg;
    c->finished = 0;
    struct CommandThreadInfo *info = malloc(sizeof(struct CommandThreadInfo));
    info->command = c;
    info->commandList = commandList;
    info->ctx = ctx;
    if (pthread_create(&(c->thread), NULL, commandThread, info)) {
        fputs("Failed to launch command!", stderr);
    }
    
    return c;
}

void deleteRunningCommand(RunningCommand *c) {
    deleteChatMessage(c->message);
    for (int i = 0; i < c->argc; i++) {
        free(c->argv[i]);
    }
    free(c->argv);
    free(c);
}