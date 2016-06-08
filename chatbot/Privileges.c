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
