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
    }
    
    PrivUsers **users = bot->privUsers;
    
    users [bot->numOfPrivUsers]->userID = userID;
    
    bot->numOfPrivUsers ++;
    
    postReply (bot->room, "The user is now privileged.");
    
    printf ("User ID %d added to privilege list.", userID);
    
    return;
}


