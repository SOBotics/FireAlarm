//
// Privileges.h
// chatbot
//
// Created on 8/06/2016
// Copyright © 2016 Ashish Ahuja. All rights reserved.
//

#ifndef Client_h
#define Client_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

typedef struct {
    long userId;
    char *username;
}PrivUsers;

unsigned int checkPrivUser (ChatBot *bot, long userID);
PrivUsers *createPrivUsers (long userID);
unsigned userPrivCheck (ChatBot *bot, long userID);
unsigned commandPriv (RunningCommand *commands);
unsigned commandPrivCheck (RunningCommand *command, ChatBot *bot);

#endif /* Privileges.h */
