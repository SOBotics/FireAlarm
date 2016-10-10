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

char *getCurrentTime ()
{
    time_t t;
    time (&t);
    return asctime (localtime (&t));
}

/*void registerMalloc (char *message, char *var, char *func, size_t size)
{
    /*char *cmd = malloc (sizeof (char) * 1024);
    snprintf (cmd, 1023,
              "echo -e 'REGISTERING MALLOC \n%s \nMessage:%s \nVariable:%s \nFunction:%s \nSize:%zu' >> memory.log",
              getCurrentTime (), message, var, func, size);

    system (cmd);  // I know using 'system' is not preferable, but I'm using it joust for now. will replace it later
    free (cmd);
    return;*/


//}


void registerFree (char *message, char *var, char *func)
{
    char *cmd = malloc (sizeof (char) * 1024);
    snprintf (cmd, 1023,
              "echo 'REGISTERING FREE %s Message:%s Variable:%s Function:%s' >> memory.log",
              getCurrentTime (), message, var, func);
    system (cmd);
    free (cmd);
    return;
}

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
