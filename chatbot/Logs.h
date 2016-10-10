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

//#include "ChatBot.h"

/*typedef struct {
    unsigned isOpen; //0 if malloc has been free'd, 1 if still not freed
    size_t size;
    char *openTime;
    char *closeTime;
    unsigned number;
}LogMemory;

typedef struct {
    unsigned type; //1 if memory type
    LogMalloc *memoryLog;
}Log;*/

typedef struct {
    unsigned type;
    char *funcCaller;
    char *location;
    size_t size;
    unsigned key;
    char *time;
    char *message; //Contains an optional description of the log
}Log;

//void registerFree (char *message, char *var, char *func);

#endif // Logs_h
