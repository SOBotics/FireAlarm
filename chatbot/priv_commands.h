//
// priv_commands.h
// chatbot
//
// Created on 8/06/2016
// Copyright © 2016 Ashish Ahuja. All rights reserved.
//

void addUserPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    long userID = (long) strtol(command->argv[0], NULL, 10);
    
    if (checkPrivUsers (bot, userID)
    {
        postReply (bot->room, "The user is already privileged.", command->message);
        return;
    }
    if (userID <= 0)
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    
    PrivUsers **users = bot->privUsers;
    
    users [bot->numOfPrivUsers]->userID = userID;
    
    bot->numOfPrivUsers ++;
    
    postReply (bot->room, "The user is now privileged.", command->message);
    
    printf ("User ID %d added to privilege list.", userID);
    
    return;
}

void removeUserPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    long userID = (long) strtol(command->argv[0], NULL, 10);
    
    if (!checkPrivUsers (bot, userID))
    {
        postReply (bot->room, "The user is not privileged already.", command->message);
        return;
    }
    else if (userID < 1)
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    
    PrivUsers **users = bot->privUsers;
    int check;
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (users[i]->userID == userID)
        {
            check = i;
            break;
        }
    }
    
    for (i = check; i < bot->numOfPrivUsers; i ++)
    {
        users [i] = users [i + 1];
    }
    
    users [numOfPrivUsers] = NULL;
    
    bot->numOfPrivUsers --;
    
    postReply (bot->room, "The user has been removed from the privilege list.", command->message);
    
    printf ("User ID %d removed from privilege list.", userID);
    
    return;
}


