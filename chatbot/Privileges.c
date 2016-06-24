//
// Privileges.c
// chatbot
//
// Created by Ashish Ahuja on 8/06/2016
// Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

#include "Privileges.h"
#include "ChatBot.h"

PrivUser *createPrivUser (long userID, char *name, int privLevel)
{
    PrivUser *pu = malloc (sizeof (PrivUser));
    
    pu->userID = userID;
    pu->username = name;
    pu->privLevel = privLevel;
    
    return pu;
}

PrivRequest *createPrivRequest (long userID, char *name, int groupType)
{
    PrivRequest *pr = malloc (sizeof (PrivRequest));
    
    pr->userID = userID;
    pr->username = name;
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
    
    for (int i = check; i < bot->totalPrivRequests; i ++)
    {
        requests [i] = requests [i + 1];
    }
    
    requests [bot->totalPrivRequests] = NULL;
    
    bot->totalPrivRequests --;
    
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
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (users[i]->userID == userID)
        {
            if (users[i]->privLevel == 1)
            {
                return 1;
            }
            else if (users[i]->privLevel == 2)
            {
                return 2;
            }
        }
    }
    
    return 0;
}

unsigned commandPriv (RunningCommand *commands)
{
    if (commands->command->isPrivileged == 1)
    {
        return 1;
    }
    else if (commands->command->isPrivileged == 2)
    {
        return 2;
    }
    
    return 0;
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
    int commandPriv = checkPrivUser (bot, userID);
    
    if (commandPriv == 1 && isPrivileged == 0)
    {
        postReply (bot->room, "You need to be a member or a bot owner to run that command.", command->message);
        return 1;
    }
    else if (commandPriv == 2 && isPrivileged != 2)
    {
        postReply (bot->room, "You need to be a bot owner to run that command.", command->message);
        return 1;
    }
    
    return 0;
}
