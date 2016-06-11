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
    char privType [20] = command->argv [1];
    int groupType;
    
    if (strcmp (privType, "bot owner") != 0 && strcmp (privType, "member") != 0)
    {
        postReply (bot->room, "**Usage:** privilege user [user id] [group]", command->message);
        return;
    }
    if (strcmp (privType, "bot owner") == 0)
    {
        groupType = 0;
    }
    else if (strcmp (privType, "member") == 0)
    {
        groupType = 1;
    }
    
    if (groupType == 1 && (userPrivCheck (bot, userID) != 0))
    {
        postReply (bot->room, "The user is already a member.", command->message);
        return;
    }
    else if (groupType == 2 && (userPrivCheck (bot, userID) == 2))
    {
        postReply (bot->room, "The user is already a bot owner.", command->message);
        return;
    }
    
    if (userID <= 0)
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    
    PrivUsers **users = bot->privUsers;
    
    users [bot->numOfPrivUsers]->userID = userID;
    users [bot->numOfPrivUsers]->username = getUsernameByID (userID);
    users [bot->numOfPrivUsers]->privLevel = groupType;
    
    bot->numOfPrivUsers ++;
    
    char *messageString;
    
    if (groupType == 1)
    {
        asprintf (&messageString, "The user is now a member.");
    }
    else if (groupType == 2)
    {
        asprintf (&messageString, "The user is now a bot owner.");
    }
    
    postReply (bot->room, messageString, command->message);
    
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

void isPrivileged (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    int userID;
    int check;
    
    if (command->argc == 1)
    {
        userID = (int)strtol (command->argv [0], NULL, 10);
    }
    else if (command->argc == 0)
    {
        userID = command->message->user->userID;
        check = 1;
    }
    else
    {
        postReply(bot->room, "**Usage:** @FireAlarm is privileged [user id]", command->message);
        return;
    }
    
    if (userPrivCheck (bot, userID))
    {
        if (check)
        {
            postReply (bot->room, "Yes, you're a privileged user.", command->message);
        }
        else
        {
            postReply (bot->room, "Yes, that user is privileged.", command->message);
        }
    }
    else
    {
        if (check)
        {
            postReply (bot->room, "No, you're not a privileged user.", command->message);
        }
        else
        {
            postReply (bot->room, "No, that user is not privileged.", command->message);
        }
    }
    
    return;
}

void printPrivUser (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    char message [35];
    
    postReply (bot->room, "The privileged users are: ", command->message);
    PrivUsers **users = bot->privUsers;
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        sprintf (message, "%s (user id %ld)", users [i]->username, users[i]->userID);
        postMessage (bot->room, message);
    }
    
    return;
}
