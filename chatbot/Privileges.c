//
// Privileges.c
// chatbot
//
// Created by Ashish Ahuja on 8/06/2016
// Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

#include "Privileges.h"
#include "ChatBot.h"
#include <limits.h>

PrivUser *createPrivUser (long userID, int privLevel)
{
    PrivUser *pu = malloc (sizeof (PrivUser));
    
    pu->userID = userID;
    pu->privLevel = privLevel;
    
    return pu;
}

PrivRequest *createPrivRequest (long userID, int groupType)
{
    PrivRequest *pr = malloc (sizeof (PrivRequest));
    
    pr->userID = userID;
    pr->groupType = groupType;
    
    return pr;
}

void deletePrivRequest (ChatBot *bot, unsigned priv_number)
{
    PrivRequest **requests = bot->privRequests;
    
    int check = 0;
    
    for (int i = 0; i < bot->totalPrivRequests; i ++)
    {
        if (i == priv_number - 1)
        {
            check = i;
        }
    }
    
    for (int i = check; i < bot->totalPrivRequests - 1; i ++)
    {
        requests [i] = requests [i + 1];
    }
    
    requests [--bot->totalPrivRequests] = NULL;
    
    return;
}

unsigned privRequestExist (ChatBot *bot, unsigned priv_number)
{
    if (bot->totalPrivRequests < priv_number)
    {
        return 0;
    }
    return 1;
}

unsigned checkPrivUser (ChatBot *bot, long userID)
{
    PrivUser **users = bot->privUsers;
    
    ChatUser *user = getUserByID(bot->room, userID);
    if (user->isModerator || user->isRoomOwner) {
        return INT_MAX;
    }
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (users[i]->userID == userID)
        {
            return users[i]->privLevel;
        }
    }
    
    return 0;
}

unsigned commandPriv (RunningCommand *command)
{
    return command->command->privileges;
}

PrivUser *getPrivUserByID (ChatBot *bot, long userID)
{
    PrivUser **privUsers = bot->privUsers;
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (privUsers[i]->userID == userID)
        {
            return privUsers[i];
        }
    }
    return NULL;
}

unsigned commandPrivCheck (RunningCommand *command, ChatBot *bot)
{
    long userID = command->message->user->userID;
    int isPrivileged = checkPrivUser (bot, userID);
    int commandPriv = command->command->privileges;
    
    if ((isPrivileged & commandPriv) != commandPriv) {
        postReply(bot->room, "You do not have priveleges to run that command", command->message);
        return 0;
    }
    
    return 1;
}

char **getPrivilegeGroups() {
    static char *groups[] = {
        "none",
        "member",
        "owner",
        NULL
    };
    return groups;
}

unsigned privilegeNamed(char *name) {
    char **groups = getPrivilegeGroups();
    unsigned privilege = 1;
    for (int i = 1; groups[i]; i++) {
        if (!strcmp(groups[i], name)) {
            return privilege;
        }
        privilege *= 2;
    }
    return 0;
}
