//
//  post_commands.h
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef post_commands_h
#define post_commands_h

#include <stdlib.h>
#include <limits.h>

void checkPostCallback(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    long postID = strtol(command->argv[0], NULL, 10);
    if (postID <= 0) {
        postReply(bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    Post *post = getPostByID(bot, postID);
    if (post == NULL) {
        postReply(bot->room, "Please enter the ID of a valid post.", command->message);
        return;
    }
    pthread_mutex_lock(&bot->detectorLock);
    checkPost(bot, post);
    pthread_mutex_unlock(&bot->detectorLock);
}

static void confirm(RunningCommand *command, void *ctx, unsigned char confirm) {
    ChatBot *bot = ctx;
    Post *post;
    if (bot->latestReports[0] == NULL) {
        postReply(bot->room, "I don't have any reports available.", command->message);
        return;
    }
    if (command->message->replyID == 0) {
        post = bot->latestReports[0]->post;
    }
    else {
        Report *report = reportWithMessage(bot, command->message->replyID);
        if (report) {
            post = report->post;
        }
        else {
            postReply(bot->room, "That report isn't available.", command->message);
            return;
        }
    }
    confirmPost(bot, post, confirm);
}

void truePositive(RunningCommand *command, void *ctx) {
    confirm(command, ctx, 1);
}

void falsePositive(RunningCommand *command, void *ctx) {
    confirm(command, ctx, 0);
}

#endif /* post_commands_h */
