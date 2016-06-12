//
// Privileges.h
// chatbot
//
// Created by Ashish Ahuja on 8/06/2016
// Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
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
    int privLevel;  // 1 if member, 2 if bot owner.
}PrivUsers;

typedef struct {
    long userID;
    char *username;
    int groupType;  // 0 if user wants to join membeers, 1 if user wants to join bot owners.
}PrivRequest;

unsigned int checkPrivUser (ChatBot *bot, long userID);
PrivUsers *createPrivUsers (long userID, char *username, int privLevel);
unsigned userPrivCheck (ChatBot *bot, long userID);
unsigned commandPriv (RunningCommand *commands);
unsigned commandPrivCheck (RunningCommand *command, ChatBot *bot);

#endif /* Privileges.h */
