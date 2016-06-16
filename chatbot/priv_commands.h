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

void requestPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (commandPrivCheck (command,bot))
    {
        return;
    }
    
    if (command->argc == 0)
    {
        postReply (bot->room, "**Usage:** `@FireAlarm request privilege [group name]`", command->message);
        return;
    }
    
    char *groupType = command->argv [0];
    
    bot->privRequests [totalPrivRequests - 1] = createPrivRequest (command->message->user->userID, command->message->user->username, groupType);

    bot->totalPrivRequests ++;
    
    return;
}

void approvePrivRequest (RunningCommand *command, void *ctx)
{
    Chatbot *bot = ctx;
    
    if (commandPrivCheck (command, bot))
    {
        return;
    }
    
    if (command->argc == 0)
    {
        postReply (bot->room, "**Usage:** `@FireAlarm approve privilege request [request number]`", command->message);
        return;
    }
    
    unsigned priv_number = (int) strtol(command->argv [0], NULL, 10);
    
    if (!privRequestExist (bot, priv_number))
    {
        postReply (bot->room, "There is not request by that number.", command->message);
        return;
    }
    
    PrivRequest **requests = bot->privRequests;
    PrivUsers **users = bot->privUsers;
    
    users [numOfPrivUsers] = createPrivUsers (requests[priv_number - 1]->userID, requests [priv_number - 1]->username, requests [priv_number - 1]->groupType + 1);
    bot->numOfPrivUsers ++;
    
    
    char *message;
    
    asprintf (&message, "Privilege request number %d has been approved. ", priv_number);
    postReply (bot->room, message, command->message);
    
    free (message);
    
    return;
}

void deletePrivrequest (ChatBot *bot, unsigned priv_number)
{
    privRequest **requests = bot->privRequests;
    
    int check = 0;
    
    for (int i = 0; i < bot->totalprivRequests; i ++)
    {
        if (i == priv_number - 1)
        {
            check = i;
        }
    }
    
    for (i = check; i < bot->totalPrivRequests; i ++)
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
    
    if (userPrivCheck (bot, userID) == 1)
    {
        if (check)
        {
            postReply (bot->room, "Yes, you're a member.", command->message);
        }
        else
        {
            postReply (bot->room, "Yes, that user is a member.", command->message);
        }
    }
    else if (userPrivCheck (bot, userID) == 2)
    {
        if (check)
        {
            postReply (bot->room, "Yes, you're a bot owner.", command->message);
        }
        else
        {
            postReply (bot->room, "Yes, that user is a bot owner.", command->message);
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
    
    int check;
    
    if (command->argc == 1)
    {
        if (strcmp (command->argv [0], "bot owner") == 0)
        {
            check = 1;
        }
        else if (strcmp (command->argv [0], "member") == 0)
        {
            check = 2;
        }
    }
    
    postReply (bot->room, "The privileged users are: ", command->message);
    PrivUsers **users = bot->privUsers;
    
    char *messageStringMembers = malloc (sizeof (64 * bot->numOfPrivUsers));
    char *messageStringOwners = malloc (sizeof (64 * bot->numOfPrivUsers));
    char *messageString = malloc (sizeof ((64 * bot->numOfPrivUsers) + 50));
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (users [i]->privLevel == 1)
        {
            snprintf (messageStringMembers + strlen (messageStringMembers), 63,
                      "%s (user ID %ld)\n", users [i]->username, users [i]->userID);
        }
        else if (users [i]->privLevel == 1)
        {
            snprintf (messageStringOwners + strlen (messageStringOwners), 63,
                      "%s (user ID %ld)\n", users [i]->username, users [i]->userID);
        }
    }
    
    if (check != 2 && check != 1)
    {
        sprintf (messageString, "Members:\n");
        sprintf (messageString + strlen (messageString), "%s\n", messageStringMembers);
        sprintf (messageString + strlen (messageString), "Bot Owners:\n");
        sprintf (messageString + strlen (messageString), "%s\n", messageStringOwners);
    }
    else if (check == 2)
    {
        sprintf (messageString, "Members:\n");
        sprintf (messageString + strlen (messageString), "%s\n", messageStringMembers);
    }
    else if (check == 1)
    {
        sprintf (messageString, "Bot Owners:\n");
        sprintf (messageString + strlen (messageString). "%s\n", messageStringOwners);
    }
    
    postMessage (bot->room, messageString);
    
    free (messageString);
    free (messageStringMembers);
    free (messageStringOwners);
    
    return;
}
