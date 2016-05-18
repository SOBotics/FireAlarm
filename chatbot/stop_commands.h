//
//  stop_commands.h
//  chatbot
//
//  Created by Jonathan Keller on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef stop_commands_h
#define stop_commands_h

#include "ChatBot.h"

void stopBot(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    postReply(bot->room, "Shutting down...", command->message);
    bot->stopAction = ACTION_STOP;
}

void rebootBot(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    postReply(bot->room, "Rebooting...", command->message);
    bot->stopAction = ACTION_REBOOT;
}

void forceStopBot(RunningCommand *command, void *ctx) {
    abort();
}

#endif /* stop_commands_h */
