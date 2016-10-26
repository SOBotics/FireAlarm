//
//  Logs.c
//  chatbot
//
//  Created by Ashish Ahuja on 9/13/16.
//  Copyright © 2016 Fortunate-MAN (Ashish Ahuja). All rights reserved.
//

/*
TODO:
    Add 'bot->logs' to ChatBot.c and ChatBot.h
    Replace using 'system' function
*/
#include "Logs.h"
#include "ChatBot.h"
unsigned errorKey = 0;

Log *createLog (unsigned type, const char *funcCaller, const char *location, size_t size, unsigned key, char *message, char *time)
{
    Log *log = malloc (sizeof (Log));

    log->type = type;
    log->funcCaller = malloc (strlen (funcCaller) + 1);
    strcpy (log->funcCaller, funcCaller);
    log->location = malloc (strlen (location) + 1);
    strcpy (log->location, location);
    log->size = size;
    log->key = key;
    log->message = malloc (strlen (message) + 1);
    strcpy (log->message, message);
    log->time = malloc (strlen (time) + 1);
    strcpy (log->time, time);

    return log;
}

void registerError (ChatBot *bot, char *location, char *message, char *func)
{
    //Log **logs = bot->log;
    unsigned *total = bot->totalLogs;
    bot->totalLogs ++;
    bot->log = realloc (bot->log, bot->totalLogs * sizeof (Log*));
    bot->log [bot->totalLogs - 1] = createLog (1, func, location, 0, ++errorKey, message, getCurrentTime ());
    return;
}

void initLog (char *name)
{
    //setlogmask (LOG_UPTO (priority));
    openlog (name, LOG_PID, LOG_LOCAL1);
}

char *getPriorityStrByNum (unsigned priority)
{
    if (priority == LOG_ERR)
    {
        return ERROR_STR;
    }
    else if (priority == LOG_ALERT)
    {
        return ALERT_STR;
    }
    else if (priority == LOG_INFO)
    {
        return INFO_STR;
    }
    else if (priority == LOG_DEBUG)
    {
        return DEBUG_STR;
    }
    else
    {
        initLog (ERROR_STR);
        syslog (LOG_ERR, "Invalid priority passed in 'getPriorityStrByNum': %u!", priority);
        closelog();
        return NULL;
    }
}

unsigned getStandardPriorityValue (unsigned priority)
{
    if (priority == ERROR_LOG)
    {
        return LOG_ERR;
    }
    else if (priority == INFO_LOG)
    {
        return LOG_INFO;
    }
    else if (priority == ALERT_LOG)
    {
        return LOG_ALERT;
    }
    else if (priority == DEBUG_LOG)
    {
        return LOG_DEBUG;
    }
    else
    {
        initLog (ERROR_STR);
        syslog (LOG_ERR, "Invalid priority passed in 'getStandardPriorityValue': %u!", priority);
        closelog();
        return 0;
    }
}

void registerLog (unsigned priority, char *str)
{
    char *logName = getPriorityStrByNum(priority);
    unsigned standardPriority = getStandardPriorityValue(priority);

    if (!standardPriority)
    {
        fprintf (stderr, "Invalid priority in 'registerLog': %u!", priority);
        initLog (ERROR_STR);
        syslog (LOG_ERR, "Invalid priority in 'registerLog': %u!", priority);
        closelog();
        return;
    }

    initLog (logName);
    syslog (standardPriority, str);
    closelog();
}
