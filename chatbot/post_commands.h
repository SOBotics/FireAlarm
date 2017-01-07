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
#include "Post.h"
#include "ChatBot.h"

void checkPostCallback(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;

    //printf ("In function 'checkPostCallback'.");
    char *arg = removeAllChars (command->argv [0]);
    //arg [37] = '\0';
    arg [8] = '\0';
    long postID = strtol(arg, NULL, 10);
    int check;

    //printf ("Starting first if....");
    if (postID <= 0) {
        postReply(bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    //printf ("Completed first if.");

    Post *post = getPostByID(bot, postID);

    //printf ("Starting second if.");
    if (post == NULL) {
        postReply(bot->room, "Please enter the ID of a valid post.", command->message);
        char *str;
        asprintf (&str, "The postID is %ld.", postID);
        postMessage (bot->room, str);
        free (str);
        return;
    }
    //printf ("Finished second if.");

    pthread_mutex_lock(&bot->detectorLock);
    //printf ("mutex locked.");
    check = checkPost(bot, post);
    //printf ("post checked.");
    pthread_mutex_unlock(&bot->detectorLock);

    //printf ("mutex unlocked.");
    /*if (check == 1)
    {
        postReply (bot->room, "The post did not match the filters.", command->message);
    }*/

    //printf ("finished function.");

    return;
}

unsigned int confirm(RunningCommand *command, void *ctx, unsigned char confirm) {
    ChatBot *bot = ctx;
    Post *post;
    Report *report;
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
        report = bot->latestReports [0];
        //editFilter (bot, post, confirm);
    }
    else {
        report = reportWithMessage(bot, command->message->replyID);

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
            //editFilter (bot, post, confirm);
        }
        else {
            postReply(bot->room, "That isn't a report.", command->message);
            return 1;
        }
    }

    confirmPost(bot, post, confirm);

    if (confirm == 1)
    {
        sprintf (messageString, REPORT_HEADER" [True Positive](http://chat.stackoverflow.com/transcript/message/%lu#%lu) report: [%s](http://stackoverflow.com/%s/%lu)",
                                                report->messageID, report->messageID, post->title, post->isAnswer ? "a" : "q", post->postID);
        //postMessage (bot->roomPostTrue, messageString);
        //not going to post to LQPHQ until the bot is more accurate
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

void statistics (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    char *message = malloc (sizeof (char) * 256);
    //registerMalloc ("registering malloc", "message", "statistics", "sizeof (char) * 256");
    unsigned int i = 0;
    unsigned int truePositives = 0;
    unsigned int falsePositives = 0;
    unsigned int unconfirmed = 0;
    int accuracy = 0;
    int check = 2;
    int numStats;
    if (command->argc == 1) {
        numStats = (int)strtol (command->argv [0], NULL, 10);
    }
    else if (command->argc == 0) {
        numStats = REPORT_MEMORY;
    }
    else {
        postReply(bot->room, "Usage: @FireAlarm stats [number of reports]", command->message);
        return;
    }

    Report **reports = bot->latestReports;

    /* Checking for errors */

    if (numStats <= 0)
    {
        postReply (bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    else if (numStats > REPORT_MEMORY)
    {
        sprintf (message, "Please enter a number less than %d.", (REPORT_MEMORY + 1));
        return;

        postReply (bot->room, message, command->message);
    }
    else if (bot->latestReports [0] == NULL)
    {
        postReply (bot->room, "There are no reports available.", command->message);
        return;
    }

    /* Calculating the number of truePositives, falsePositives, and unconfirmed reports */

    for (; i < numStats; ++i)
    {
        Report *report = reports [i];
        if (report == NULL) {
            continue;
        }
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

    accuracy = ((float)truePositives/(truePositives + falsePositives)) * 100;

    sprintf (message, "Out of the last %d reports, %d are true positives, %d are false positives, and %d are unconfirmed. The accuracy rate is %d%%.",
             numStats, truePositives, falsePositives, unconfirmed, accuracy);

    postReply (bot->room, message, command->message);
    free (message);
    //registerFree ("freeing", "message", "statistics");

    return;
}

void printLatestReports (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    int numReports = (int)strtol (command->argv [0], NULL, 10);
    const char *typePrinted;
    if (command->argc > 1) {
        typePrinted = command->argv[1];
    }
    else {
        typePrinted = NULL;
    }
    const size_t maxLineSize = 256;
    char message [256];
    unsigned int i;
    char reportLink [256];

    if (numReports <= 0)
    {
        postReply (bot->room, "Please enter a number greater than 0.", command->message);
        return;
    }
    else if (numReports > MAX_LATEST_REPORTS)
    {
        sprintf (message, "Please enter a number smaller than %d.", (MAX_LATEST_REPORTS + 1));

        postReply (bot->room, message, command->message);
        return;
    }

    Report **reports = bot->latestReports;

    if (bot->latestReports [0] == NULL)
    {
        postReply (bot->room, "There are no reports available.", command->message);
        return;
    }

    char *messageString = malloc(maxLineSize * numReports);

    sprintf (message, "The latest %d reports are: ", numReports);

    postReply (bot->room, message, command->message);

    //Commented the below code as link formatting in messages which are more than one line are broken
    /*for (i = 0; i < numReports; i ++)
    {
        Report *report = reports [i];

        sprintf (reportLink,
                 "[message %lu](http://chat.stackoverflow.com/transcript/message/%lu#%lu)",
                 report->messageID, report->messageID, report->messageID);

        char *confirmation;

        int shouldPrint = 0;
        if (typePrinted == NULL) {
            shouldPrint = 1;
        }
        else if (report->confirmation == 1) {
            shouldPrint = !strcmp(typePrinted, "true");
        }
        else if (report->confirmation == 0) {
            shouldPrint = !strcmp(typePrinted, "false");
        }
        else {
            shouldPrint = (!strcmp(typePrinted, "unconfirmed") || !strcmp(typePrinted, "unknown"));
        }

        if (shouldPrint) {
            if (report->confirmation == 1) confirmation = "True positive";
            else if (report->confirmation == 0) confirmation = "False positive";
            else confirmation = "Unconfirmed";

            char *postType = report->post->isAnswer ? "a" :"q";
            snprintf(messageString + strlen(messageString), maxLineSize,
                     "%s: [%s](http://stackoverflow.com/%s/%lu) (%s)\n",
                     confirmation, report->post->title, postType, report->post->postID, reportLink
                     );
        }
    }*/

    for (i = 0; i < numReports; i ++)
    {
        Report *report = reports [i];

        sprintf (reportLink,
                 "http://chat.stackoverflow.com/transcript/message/%lu#%lu",
                 report->messageID, report->messageID);

        char *confirmation;

        int shouldPrint = 0;
        if (typePrinted == NULL) {
            shouldPrint = 1;
        }
        else if (report->confirmation == 1) {
            shouldPrint = !strcmp(typePrinted, "true");
        }
        else if (report->confirmation == 0) {
            shouldPrint = !strcmp(typePrinted, "false");
        }
        else {
            shouldPrint = (!strcmp(typePrinted, "unconfirmed") || !strcmp(typePrinted, "unknown"));
        }

        if (shouldPrint) {
            if (report->confirmation == 1) confirmation = "True positive";
            else if (report->confirmation == 0) confirmation = "False positive";
            else confirmation = "Unconfirmed";

            char *postType = report->post->isAnswer ? "a" :"q";
            snprintf(messageString + strlen(messageString), maxLineSize,
                     "%s: http://stackoverflow.com/%s/%lu (%s)\n",
                     confirmation, postType, report->post->postID, reportLink
                     );
        }
    }

    postMessage(bot->room, messageString);

    free (messageString);

    return;
}

void postInfo (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    long postID = strtol (command->argv [0], NULL, 10);
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
    long copyPostID = 0;

    for (; i < REPORT_MEMORY; ++i)
    {
        Report *report = reports [i];

        if (!report) {
            continue;
        }

        copyPostID = report->post->postID;

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

            sprintf (message, "[That post](%s) was reported recently. It's current status is %s.",
                     link, status);

            postReply (bot->room, message, command->message);
            return;
        }
    }

    postReply (bot->room, "That post was not reported recently.", command->message);

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

void printUnclosedTP (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Report **reports = bot->latestReports;

    char *message = malloc (sizeof (256 * 10));
    int f = 0;

    for (int i = 0; i < REPORT_MEMORY && f < 6; i ++)
    {
        Report *report = reports [i];

        if (report->confirmation)
        {
            int cv = getCloseVotesByID (bot, report->post->postID);
            Post *post = report->post;

            if (!isPostClosed (bot, post->postID))
            {
                sprintf (message + strlen (message), "%d close votes: [%s](http://stackoverflow.com/%s/%lu)",
                         cv, post->title, post->isAnswer ? "a" : "q", post->postID);

                f ++;
            }
        }
    }

    if (message == NULL)
    {
        postReply (bot->room, "There are currently no reports true positive reports which are not closed.", command->message);
        free (message);
        return;
    }

    postReply (bot->room, "True reports with close votes are:", command->message);
    postMessage (bot->room, message);

    free (message);

    return;
}

void printClosedTP (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Report **reports = bot->latestReports;

    char *message = malloc (sizeof (256 * 10));
    int f = 0;

    for (int i = 0; i < REPORT_MEMORY && f < 6; i ++)
    {
        Report *report = reports [i];

        if (report->confirmation)
        {
            Post *post = report->post;

            if (isPostClosed (bot, post->postID))
            {
                sprintf (message + strlen (message), "%s: [%s](http://stackoverflow.com/%s/%lu)",
                         getClosedReasonByID (bot, post->postID), post->title, post->isAnswer ? "a" : "q", post->postID);

                f ++;
            }
        }
    }

    if (message == NULL)
    {
        postReply (bot->room, "There are currently no reports true positive reports which are closed.", command->message);
        free (message);
        return;
    }

    postReply (bot->room, "True reports which are closed are:", command->message);
    postMessage (bot->room, message);

    free (message);

    return;
}

void printCloseVotesOnFP (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Report **reports = bot->latestReports;

    char *message = malloc (sizeof (264 * 10));
    int f = 0;

    for (int i = 0; i < REPORT_MEMORY && f < 6; i ++)
    {
        Report *report = reports [i];

        if (!report->confirmation)
        {
            int cv = getCloseVotesByID (bot, report->post->postID);
            Post *post = report->post;

            if (cv > 0 && cv < 5)
            {
                sprintf (message + strlen (message), "%d close votes: [%s](http://stackoverflow.com/%s/%lu)",
                         cv, post->title, post->isAnswer ? "a" : "q", post->postID);

                f ++;
            }
        }
    }

    if (message == NULL)
    {
        postReply (bot->room, "There are currently no false positive reports with close votes.", command->message);
    }

    postReply (bot->room, "False positive reports with close votes are:", command->message);
    postMessage (bot->room, message);

    free (message);

    return;
}

#endif /* post_commands_h */
