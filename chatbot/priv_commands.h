//
// priv_commands.h
// chatbot
//
// Created on 8/06/2016
// Copyright © 2016 Ashish Ahuja. All rights reserved.
//

void makeUserPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    long userID = (long) strtol(command->argv[0], NULL, 10);
    
    if (checkPrivUsers (bot, userID)
    {
        postReply (bot->room, "The user is already privileged.", command->message);\
        return;
    }
    
    PrivUsers **users = bot->privUsers;
    
    users [bot->numOfPrivUsers + 1]->userID = userID;
    
    bot->numOfPrivUsers ++;
    
    return;
}
