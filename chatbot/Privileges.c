//
// Privileges.c
// chatbot
//
// Created on 8/06/2016
// Copyright © 2016 Ashish Ahuja. All rights reserved.
//

#include "Privileges.h"

unsigned int checkPrivUser (ChatBot *bot, long userID)
{
    unsigned int i;
    PrivUsers **users = bot->privUsers;
    
    for (i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (users->userID == userID)
        {
            return 1;
        }
    }
    return 0;
}

PrivUsers *createPrivUsers (long userID)
{
    PrivUsers *pu = malloc (sizeof (PrivUsers));
    
    pu->userID = userID;
    
    return pu;
}

unsigned userPrivCheck (ChatBot *bot, long userID)
{
    PrivUsers **users = bot->privUsers;
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (users[i]->userID == userID)
        {
            return 1;
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
    return 0;
}

unsigned commandPrivCheck (RunningCommand *command, ChatBot *bot, long userID)
{
    if (commandPriv (command))
    {
        if (!userPrivCheck (userID))
        {
            postReply (bot->room, "You nned to be privileged to run that command.", command->message);
            return 1;
        }
    }
    
    return 0;
}
