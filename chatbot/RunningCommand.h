//
//  RunningCommand.h
//  chatbot
//
//  Created by Jonathan Keller on 5/4/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef RunningCommand_h
#define RunningCommand_h

#include <stdio.h>
#include <pthread.h>
#include "ChatMessage.h"

typedef struct _Command Command;

typedef struct _RunningCommand {
    Command *command;
    ChatMessage *message;
    int argc;
    char **argv;
    pthread_t thread;
    int finished;   //1 if this command has finished running.
}RunningCommand;

RunningCommand *launchCommand(ChatMessage *msg, int argc, char **argv, Command **commandList, void *ctx);
void deleteRunningCommand(RunningCommand *command);

#endif /* RunningCommand_h */
