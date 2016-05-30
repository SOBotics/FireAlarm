//
//  post_commands.h
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef post_commands_h
#define post_commands_h
#define MAX_LATEST_REPORTS 10

#include <stdlib.h>
#include <limits.h>

void checkPostCallback(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    long postID = strtol(command->argv[0], NULL, 10);
    int check;
    
    if (postID <= 0) {
        postReply(bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    
    Post *post = getPostByID(bot, postID);
    
    if (post == NULL) {
        postReply(bot->room, "Please enter the ID of a valid post.", command->message);
        return;
    }
    
    check = recentlyReported (postID, bot);
    
    if (check == 1)
    {
        postReply (bot->room, "The post was already reported recently.", command->message);
        return;
    }
    
    pthread_mutex_lock(&bot->detectorLock);
    check = checkPost(bot, post);
    pthread_mutex_unlock(&bot->detectorLock);
    
    if (check == 1)
    {
        postReply (bot->room, "The post did not match the filters, and thus now isn't reported.", command->message);
    }
    
    return;
}

unsigned int confirm(RunningCommand *command, void *ctx, unsigned char confirm) {
    ChatBot *bot = ctx;
    Post *post;
    char messageString [256];
    char type [32];
    char type2 [32];
    
    if (bot->latestReports[0] == NULL) {
        postReply(bot->room, "I don't have any reports available.", command->message);
        return 1;
    }
    
    if (command->message->replyID == 0) {
        if ((bot->latestReports [0]->confirmation == 0 || bot->latestReports [0]->confirmation == 1) &&
            bot->latestReports [0]->confirmation != confirm)
        {
            if (confirm == 0)
            {
                strcpy (type, "true positive");
                strcpy (type2, "false positive");
            }
            else if (confirm == 1)
            {
                strcpy (type, "false positive");
                strcpy (type2, "true positive");
            }
                
            sprintf (messageString, "The report is now marked as %s. For your information, earlier the post was marked as %s. To change the feedback, just respond to the report again.", type2, type);
        }
        
        bot->latestReports[0]->confirmation = confirm;
        post = bot->latestReports[0]->post;
    }
    else {
        Report *report = reportWithMessage(bot, command->message->replyID);
        
        if (report) {
            if ((report->confirmation == 0 || report->confirmation == 1) &&
            report->confirmation != confirm)
            {
            if (confirm == 0)
            {
                strcpy (type, "true positive");
                strcpy (type2, "false positive");
            }
            else if (confirm == 1)
            {
                strcpy (type, "false positive");
                strcpy (type2, "true positive");
            }
                
            sprintf (messageString, "The report is now marked as %s. For your information, earlier the post was marked as %s. To change the feedback, just respond to the report again.", type2, type);
            }
            
            report->confirmation = confirm;
            post = report->post;
        }
        else {
            postReply(bot->room, "That isn't a report.", command->message);
            return 1;
        }
    }
    
    confirmPost(bot, post, confirm);
    
    if (confirm == 1)
    {
        sprintf (messageString, "**Bad Post:** [%s](http://stackoverflow.com/%s/%lu)", post->title, post->isAnswer ? "a" : "q", post->postID);
        postMessage (bot->roomPostTrue, messageString);
    }
    
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
        sprintf (message, "Please enter a number smaller than %d.", (REPORT_MEMORY + 1));
        
        postReply (bot->room, message, command->message);
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
             
    sprintf (message, "Out of the last %d reports, %d are true positives, %d are false positives, and %d are unconfirmed.",
             numStats, truePositives, falsePositives, unconfirmed);
             
    postReply (bot->room, message, command->message);
    
    return 0;
}

void printLatestReports (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    int numReports = strtol (command->argv [0], NULL, 3);
    char typePrinted [16] = strtol (command->argv [1], NULL, 16);
    const size_t maxLineSize = 256;
    char message [256];
    unsigned int i;
    long postid;
    char postTitle [127];
    char reportLink [256];
    
    if (numReports <= 0)
    {
        postReply (bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    else if (numReports > MAX_LATEST_REPORTS)
    {
        sprintf (message, "Please enter a number smaller than %d.", (MAX_LATEST_REPORTS + 1)
        
        postReply (bot->room, message, command->message);
        return;
    }
    
    Report **reports = bot->latestReports;
    
    if (bot->latestReports [0] == NULL)
    {
        postReply (bot->room, "There are no reports available.", command->message);
        return;
    }
    
    char *messageString = malloc(maxLineSize * (bot->runningCommandCount + 5));
    
    if (typePrinted == NULL || strcmp (typePrinted, " ") == 0)
    {
    
    strcpy(messageString,
           "          Stats         |"
           "                                 Link                                  |"
           "       Likelihood       |"
           "       Message ID        \n"
           "-------------------------"
           "-----------------------------------------------------------------------"
           "-------------------------"
           "-------------------------\n"
           );
           
    sprintf (message, "The latest %d reports are: ", numReports);
    
    postReply (bot->room, message, command->message);
    postMessage (bot->room, messageString);
    
    for (i = 0; i < numReports; i ++)
    {
        Report *report = reports [i];
        
        sprintf (reportLink, 
                 "[%d](http:/chat.stackoverflow.com/transcript/message/%d#%d)",
                 report->messageID, report->messageID, report->messageID);
        
        if (report->confirmation == 1)
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "     True Positive     |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 "
             "          %d           |"
             "        %s              \n",
             postTitle, post->isAnswer ? "a" : "q", post->postID, report->likelihood, reportLink);
             
            postMessage (bot->room, messageString);
        }
        else if (report->confirmation == 0)
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "    False Positive     |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 "
             "          %d           |"
             "        %s              \n",
             postTitle, post->isAnswer ? "a" : "q", post->postID, report->likelihood, reportLink);
             
            postMessage (bot->room, messageString);
        }
        else
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "      Unconfirmed      |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 |"
             "          %d           |"
             "        %s              \n",
             postTitle, post->isAnswer ? "a" : "q", post->postID, report->likelihood, reportLink);
             
            postMessage (bot->room, messageString);
        }
    }
    
    }
    else if (strcmp (typePrinted, "true") == 0 || strcmp (typePrinted, "true positive") == 0)
    {
        Report **trueReports;
        
        unsigned int j = 0;
        unsigned int totalTrue = 0;
        unsigned int check = 0;
        
        for (i = 0; i < REPORT_MEMORY; ++i)
        {
            if (reports [i]->confirmation == 1)
            {
                trueReports [j] = reports [i];
                j ++;
                totalTrue ++;
            }
        }
        
        strcpy(messageString,
           "          Stats         |"
           "                                 Link                                  |"
           "       Likelihood       |"
           "       Message ID        \n"
           "-------------------------"
           "-----------------------------------------------------------------------"
           "-------------------------"
           "-------------------------\n"
           );
           
        if (totalTrue < numReports)
        {
            sprintf (message, "There are only %d true positive reports available. They are: ", totalTrue);
            postReply (bot->room, message, command->message);
        }
        else
        {
            sprintf (message, "The latest %d reports are: ", numReports);
            postReply (bot->room, message, command->message);
        }
        
        if (numReports > totalTrue)
            check = totalTrue;
        else
            check = numReports;
            
        postMessage (bot->room, messageString);
        
        for (i = 0; i < check; i ++)
        {
        Report *report = trueReports [i];
        
        sprintf (reportLink, 
                 "[%d](http:/chat.stackoverflow.com/transcript/message/%d#%d)",
                 report->messageID, report->messageID, report->messageID);
        
        if (report->confirmation == 1)
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "     True Positive     |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 "
             "          %d           |"
             "        %s              \n",
             postTitle, post->isAnswer ? "a" : "q", post->postID, report->likelihood, reportLink);
             
            postMessage (bot->room, messageString);
        }
        
        }
    }
    else if (strcmp (typePrinted, "false") == 0 || strcmp (typePrinted, "false positive") == 0)
    {
        Report **trueReports;
        
        unsigned int j = 0;
        unsigned int totalTrue = 0;
        unsigned int check = 0;
        
        for (i = 0; i < REPORT_MEMORY; ++i)
        {
            if (reports [i]->confirmation == 1)
            {
                trueReports [j] = reports [i];
                j ++;
                totalTrue ++;
            }
        }
        
        strcpy(messageString,
           "          Stats         |"
           "                                 Link                                  |"
           "       Likelihood       |"
           "       Message ID        \n"
           "-------------------------"
           "-----------------------------------------------------------------------"
           "-------------------------"
           "-------------------------\n"
           );
           
        if (totalTrue < numReports)
        {
            sprintf (message, "There are only %d false positive reports available. They are: ", totalTrue);
            postReply (bot->room, message, command->message);
        }
        else
        {
            sprintf (message, "The latest %d reports are: ", numReports);
            postReply (bot->room, message, command->message);
        }
        
        if (numReports > totalTrue)
            check = totalTrue;
        else
            check = numReports;
            
        postMessage (bot->room, messageString);
        
        for (i = 0; i < check; i ++)
        {
        Report *report = trueReports [i];
        
        sprintf (reportLink, 
                 "[%d](http:/chat.stackoverflow.com/transcript/message/%d#%d)",
                 report->messageID, report->messageID, report->messageID);
        
        if (report->confirmation == 1)
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "    False Positive     |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 "
             "          %d           |"
             "        %s              \n",
             postTitle, post->isAnswer ? "a" : "q", post->postID, report->likelihood, reportLink);
             
            postMessage (bot->room, messageString);
        }
        
        }
    }
    else if (strcmp (typePrinted, "unconfirmed") == 0 || strcmp (typePrinted, "unknown") == 0)
    {
         Report **trueReports;
        
        unsigned int j = 0;
        unsigned int totalTrue = 0;
        unsigned int check = 0;
        
        for (i = 0; i < REPORT_MEMORY; ++i)
        {
            if (reports [i]->confirmation != 1 && reports [i]->confirmation != 0)
            {
                trueReports [j] = reports [i];
                j ++;
                totalTrue ++;
            }
        }
        
        strcpy(messageString,
           "          Stats         |"
           "                                 Link                                  |"
           "       Likelihood       |"
           "       Message ID        \n"
           "-------------------------"
           "-----------------------------------------------------------------------"
           "-------------------------"
           "-------------------------\n"
           );
           
        if (totalTrue < numReports)
        {
            sprintf (message, "There are only %d unconfirmed reports available. They are: ", totalTrue);
            postReply (bot->room, message, command->message);
        }
        else
        {
            sprintf (message, "The latest %d reports are: ", numReports);
            postReply (bot->room, message, command->message);
        }
        
        if (numReports > totalTrue)
            check = totalTrue;
        else
            check = numReports;
            
        postMessage (bot->room, messageString);
        
        for (i = 0; i < check; i ++)
        {
        Report *report = trueReports [i];
        
        sprintf (reportLink, 
                 "[%d](http:/chat.stackoverflow.com/transcript/message/%d#%d)",
                 report->messageID, report->messageID, report->messageID);
        
        if (report->confirmation != 1 && report->confirmation [i] != 0)
        {
            Post *post = report->post;
            
            strcpy (postTitle, post->title);
            
            sprintf (messageString,
             "     Unconfirmed       |"
             "  [%s](http://stackoverflow.com/%s/%lu)                                 "
             "          %d           |"
             "        %s              \n",
             postTitle, post->isAnswer ? "a" : "q", post->postID, report->likelihood, reportLink);
             
            postMessage (bot->room, messageString);
        }
        
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

void testPostCallback (RunningCommand *command, void *ctx)
{
     ChatBot *bot = ctx;
     long postID = strtol(command->argv[0], NULL, 10);
     
     if (postID <= 0)
     {
         postReply (bot->room, "Please enter a number bigger than 0", command->message);
         return;
     }
     
     Post *post = getPostByID(bot, postID);
     
     if (post == NULL)
     {
         postReply (bot->room, "Please enter the ID of a valid post.", command->message);
         return;
     }
     
     pthread_mutex_lock (&bot->detectorLock);
     testPost (bot, post, command);
     pthread_mutex_unlock (&bot->detectorLock);
     
     return;
}

#endif /* post_commands_h */
