#ifndef log_commands_h
#define log_commands_h

#include <stdio.h>
#include <stdlib.h>
#include "Logs.h"

void printErrorLogs (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Log **logs = bot->log;
    unsigned i;
    char *message = malloc (sizeof (char) * bot->totalLogs * 512 + 256);
    sprintf (message, "     The error logs are:\n");
   // message [0] = '\0';

    for (i = 0; i < bot->totalLogs; i ++)
    {
        Log *log = logs [i];

        if (log->type == 1)
        {
            snprintf (message + strlen (message), 512,
                      "     In %s/%s at %s     : %s    \n",
                      log->location, log->funcCaller, log->time, log->message);
        }
    }

    if (message [0] == '\0')
    {
        postReply (bot->room, "There are no error logs currently.", command->message);
        registerError (bot, "log_commands.h/PrintErrorLogs", "error log test", "printErrorLogs");
        return;
    }

    //postReply (bot->room, "The error logs are:", command->message);
    postMessage (bot->room, message);
    free (message);
    return;
}

void clearErrorLogs (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Log **logs = bot->log;
    unsigned i;

    for (i = 0; i < bot->totalLogs; i ++)
    {
        logs [i] = NULL;
    }
    bot->log = realloc (bot->log, sizeof (Log*));
    bot->totalLogs = 0;
    postReply (bot->room, "Cleared all error logs successfully. This cannot be reversed now.", command->message);
    return;
}

#endif // log_commands_h
