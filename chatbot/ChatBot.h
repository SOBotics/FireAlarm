//
//  ChatBot.h
//  chatbot
//
//  Created by Jonathan Keller on 5/5/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef ChatBot_h
#define ChatBot_h

#include <stdio.h>
#include "ChatRoom.h"
#include "Command.h"
#include "RunningCommand.h"

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
    StopAction stopAction;
}ChatBot;

ChatBot *createChatBot(ChatRoom *room, Command **commands);
StopAction runChatBot(ChatBot *chatbot);

#endif /* ChatBot_h */
