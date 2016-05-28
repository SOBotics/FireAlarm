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
    checkPost(bot, post, command);
    pthread_mutex_unlock(&bot->detectorLock);
}

unsigned int confirm(RunningCommand *command, void *ctx, unsigned char confirm) {
    ChatBot *bot = ctx;
    Post *post;
    if (bot->latestReports[0] == NULL) {
        postReply(bot->room, "I don't have any reports available.", command->message);
        return 1;
    }
    if (command->message->replyID == 0) {
        bot->latestReports[0]->confirmation = confirm;
        post = bot->latestReports[0]->post;
    }
    else {
        Report *report = reportWithMessage(bot, command->message->replyID);
        if (report) {
            report->confirmation = confirm;
            post = report->post;
        }
        else {
            postReply(bot->room, "That isn't a report.", command->message);
            return 1;
        }
    }
    confirmPost(bot, post, confirm);
    return 0;
}

void truePositive(RunningCommand *command, void *ctx) {
    confirm(command, ctx, 1);
}

void falsePositive(RunningCommand *command, void *ctx) {
    confirm(command, ctx, 0);
}

/* Marks report as true positive and responds to user confirming that the feedback is recorded */

void truePositiveRespond (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    unsigned int check;
    
    check = confirm (command, ctx, 1);
    
    if (check == 0)
        postReply (bot->room, "The report has been successfully been recorded as True Positive.", command->message);
        
    return;
}

/* Marks report a false positive and responds to user confirming that the feedback is recorded */

void falsePositiveRespond (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    unsigned int check;
    
    check = confirm (command, ctx, 0);
    
    if (check == 0)
     postReply (bot->room, "The report has been successfully been recorded as False Positive", command->message);
   
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
    else if (numStats > REPORT_MEMORY)
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

void printLatestReports (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    int numReports = strtol (command->argv [0], NULL, 3);
    const size_t maxLineSize = 256;
    char message [256];
    unsigned int i;
    long postid;
    char postTitle [127];
    
    if (numReports <= 0)
    {
        postReply (bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    else if (numReports > 10)
    {
        postReply (bot->room, "Please enter a number smaller than 11.", command->message);
        return;
    }
    
    Report **reports = bot->latestReports;
    
    if (bot->latestReports [0] == NULL)
    {
        postReply (bot->room, "There are no reports available.", command->message);
        return;
    }
    
    char *messageString = malloc(maxLineSize * (bot->runningCommandCount + 2));
    
    strcpy(messageString,
           "          Stats         |"
           "                                 Link                                 \n"
           "    ---------------------"
           "----------------------------------------------------------------------\n"
           );
           
    sprintf (message, "The latest %d reports are: ", numReports);
    
    postReply (bot->room, message, command->message);
    postMessage (bot->room, messageString);
    
    for (i = 0; i < numReports; i ++)
    {
        Report *report = reports [i];
        
        if (report->confirmation == 1)
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "     True Positive     |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 ",
             postTitle, post->isAnswer ? "a" : "q", post->postID);
             
            postMessage (bot->room, messageString);
        }
        else if (report->confirmation == 0)
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "    False Positive     |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 ",
             postTitle, post->isAnswer ? "a" : "q", post->postID);
             
            postMessage (bot->room, messageString);
        }
        else
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "      Unconfirmed      |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 ",
             postTitle, post->isAnswer ? "a" : "q", post->postID);
             
            postMessage (bot->room, messageString);
        }
    }
    
    free (messageString);
    
    return;
}

void postInfo (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    int postID = strtol (command->argv [0], NULL, 10);
    char message [512];
    char link [256];
    char status [25];
    
    if (postID <= 0)
    {
        postReply (bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    
    Post *post = getPostByID(bot, postID);
    
    if (post == NULL)
    {
        postReply (bot->room, "Please enter the ID of a valid post.", command->message);
        return;
    }
    
    Report **reports = bot->latestReports;
    unsigned int i = 0;
    int copyPostID = 0;
    
    for (; i < REPORT_MEMORY; ++i)
    {
        Reoprt *report = reports [i];
        
        copyPostID = report->postID;
        
        if (copyPostID == postID)
        {
            if (report->confirmation == 1)
            {
                strcpy (status, "True Positive");
            }
            else if (report->confirmation == 0)
            {
                strcpy (status, "False Positive");
            }
            else
            {
                strcpy (status, "Unconfirmed");
            }
            
            sprintf (link, "http://stackoverflow.com/%s/%lu", post->isAnswer ? "a" : "q", postID);
            
            sprintf (message, "[Your Post](%s) was reported recently. It's current status is %s.",
                     link, status);
                     
            postReply (bot->room, message, command->message);
            return;
        }
    }
    
    postReply (bot->room, "Your post was not reported recently.", command->message);
    
    return;
}

#endif /* post_commands_h */
