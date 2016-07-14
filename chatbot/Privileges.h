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

typedef struct _ChatBot ChatBot;
typedef struct _RunningCommand RunningCommand;

typedef struct _PrivUser {
    long userID;
    int privLevel;  // 1 if member, 2 if bot owner.
}PrivUser;

typedef struct _PrivRequest {
    long userID;
    int groupType;  // 0 if user wants to join membeers, 1 if user wants to join bot owners.
}PrivRequest;

unsigned int checkPrivUser (ChatBot *bot, long userID);
PrivUser *createPrivUser (long userID, int privLevel);
PrivUser *getPrivUserByID (ChatBot *bot, long userID);
PrivRequest *createPrivRequest (long userID, int groupType);
unsigned checkPrivUser (ChatBot *bot, long userID);
unsigned commandPriv (RunningCommand *commands);
unsigned commandPrivCheck (RunningCommand *command, ChatBot *bot);
unsigned privRequestExist (ChatBot *bot, unsigned priv_number);
void deletePrivRequest (ChatBot *bot, unsigned priv_number);
char **getPrivilegeGroups();
unsigned privilegeNamed(char*);

#endif /* Privileges.h */
