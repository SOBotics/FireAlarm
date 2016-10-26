//
//  Logs.c
//  chatbot
//
//  Created by Ashish Ahuja on 9/13/16.
//  Copyright © 2016 Fortunate-MAN (Ashish Ahuja). All rights reserved.
//

#ifndef Logs_h
#define Logs_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>

#define ERROR_LOG 1
#define INFO_LOG 2
#define DEBUG_LOG 3
#define ALERT_LOG 4
#define ERROR_STR "FireAlarm_Error"
#define INFO_STR "FireAlarm_Info"
#define DEBUG_STR "FireAlarm_Debug"
#define ALERT_STR "FireAlarm_Alert"

typedef struct {
    unsigned type;
    char *funcCaller;
    char *location;
    size_t size;
    unsigned key;
    char *time;
    char *message; //Contains an optional description of the log
}Log;

Log *createLog (unsigned type, const char *funcCaller, const char *location, size_t size, unsigned key, char *message, char *time);
void initLog (char *name);
char *getPriorityStrByNum (unsigned priority);
unsigned getStandardPriorityValue (unsigned priority);
void registerLog (unsigned priority, char *str);

#endif // Logs_h
