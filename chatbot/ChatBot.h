//
//  ChatBot.h
//  chatbot
//
//  Created on 5/5/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef ChatBot_h
#define ChatBot_h

#include <stdio.h>
#include "ChatRoom.h"
#include "Command.h"
#include "RunningCommand.h"
#include "Filter.h"
#include "Post.h"

typedef enum {
    ACTION_NONE = 0,
    ACTION_STOP = 1,
    ACTION_REBOOT = 2
}StopAction;

typedef struct _ChatBot {
    ChatRoom *room;
    Command **commands;
    RunningCommand **runningCommands;
    size_t runningCommandCount;
    pthread_mutex_t runningCommandsLock;
    pthread_mutex_t detectorLock;
    StopAction stopAction;
    char *apiFilter;
    Filter **filters;
    unsigned filterCount;
}ChatBot;

ChatBot *createChatBot(ChatRoom *room, Command **commands, Filter **filters);
StopAction runChatBot(ChatBot *chatbot);
Post *getPostByID(ChatBot *bot, unsigned long postID);
void checkPost(ChatBot *bot, Post *post);

#endif /* ChatBot_h */
