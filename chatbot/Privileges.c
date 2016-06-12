//
// Privileges.c
// chatbot
//
// Created by Ashish Ahuja on 8/06/2016
// Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

#include "Privileges.h"

PrivUsers *createPrivUsers (long userID, char *name, int privLevel)
{
    PrivUsers *pu = malloc (sizeof (PrivUsers));
    
    pu->userID = userID;
    pu->username = name;
    pu->privLevel = privLevel;
    
    return pu;
}

PrivRequest *createPrivRequest (long userID, char *name, int groupType)
{
    PrivRequest *pr = malloc (sizeof (PrivRequest));
    
    pr->userID = userID;
    pu->username = name;
    pu->groupType = groupType;
    
    return pr;
}

unsigned userPrivCheck (ChatBot *bot, long userID)
{
    PrivUsers **users = bot->privUsers;
    
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

PrivUsers *getPrivUserByID (ChatBot *bot, long userID)
{
    PrivUsers *users = malloc (sizeof (PrivUsers));
    PrivUser **privUsers = bot->privUsers;
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (privUsers[i]->userID == userID)
        {
            users->userID = privUsers[i]->userID;
            users->username = privUsers[i]->username;
            users->privLevel = privUsers[i]->privLevel;
            return users;
        }
    }
}

unsigned commandPrivCheck (RunningCommand *command, ChatBot *bot)
{
    long userID = command->message->user->userID;
    PrivUsers *user = getPrivUserByID (userID);
    int isPrivileged = userPrivCheck (userID);
    int commandPriv = userPrivCheck (bot, userID);
    
    if (commandPriv == 1 && isPrivilged == 0)
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
