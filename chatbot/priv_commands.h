//
// priv_commands.h
// chatbot
//
// Created on 8/06/2016
// Copyright © 2016 Ashish Ahuja. All rights reserved.
//

#include "Privileges.h"
#include "misc_functions.h"

void addUserPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    long userID = (long) strtol(command->argv[0], NULL, 10);
    char *privType = command->argv [1];
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
    
    if (groupType == 1 && (checkPrivUser (bot, userID) != 0))
    {
        postReply (bot->room, "The user is already a member.", command->message);
        return;
    }
    else if (groupType == 2 && (checkPrivUser (bot, userID) == 2))
    {
        postReply (bot->room, "The user is already a bot owner.", command->message);
        return;
    }
    
    if (userID <= 0)
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    
    PrivUser **users = bot->privUsers;
    
    users [bot->numOfPrivUsers]->userID = userID;
    users [bot->numOfPrivUsers]->username = getUsernameByID (bot, userID);
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
    
    printf ("User ID %ld added to privilege list.", userID);
    
    return;
}

void removeUserPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    long userID = (long) strtol(command->argv[0], NULL, 10);
    
    if (!checkPrivUser (bot, userID))
    {
        postReply (bot->room, "The user is not privileged already.", command->message);
        return;
    }
    else if (userID < 1)
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    
    PrivUser **users = bot->privUsers;
    int check;
    
    for (int i = 0; i < bot->numOfPrivUsers; i ++)
    {
        if (users[i]->userID == userID)
        {
            check = i;
            break;
        }
    }
    
    for (int i = check; i < bot->numOfPrivUsers; i ++)
    {
        users [i] = users [i + 1];
    }
    
    users [bot->numOfPrivUsers] = NULL;
    
    bot->numOfPrivUsers --;
    
    postReply (bot->room, "The user has been removed from the privilege list.", command->message);
    
    printf ("User ID %ld removed from privilege list.", userID);
    
    return;
}

void requestPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    unsigned long userID = command->message->user->userID;
    
    if (command->argc == 0)
    {
        postReply (bot->room, "**Usage:** `@FireAlarm request privilege [group name]`", command->message);
        return;
    }
    for (int i = 0; i < bot->totalPrivRequests; i++) {
        if (bot->privRequests[i] && bot->privRequests[i]->userID == userID) {
            postReply(bot->room,
                      "You already have a pending privilege request; please wait until that one is approved",
                      command->message);
            return;
        }
    }
    
    char *group = command->argv [0];
    
    unsigned groupID = privilegeNamed(group);
    if (!groupID) {
        postReply(bot->room, "Invalid privilege group", command->message);
    }
    if (getPrivUserByID(bot, userID)->privLevel & groupID) {
        postReply(bot->room, "You already have that privilege", command->message);
    }
    
    bot->privRequests = realloc(bot->privRequests, (++bot->totalPrivRequests + 1) * sizeof(PrivRequest*));
    bot->privRequests [bot->totalPrivRequests - 1] = createPrivRequest (
                                                                        userID,
                                                                        command->message->user->name,
                                                                        groupID
                                                                        );
    bot->privRequests[bot->totalPrivRequests] = NULL;
    
    
    return;
}

void approvePrivRequest (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (command->argc == 0)
    {
        postReply (bot->room, "**Usage:** `@FireAlarm approve privilege request [request number]`", command->message);
        return;
    }
    
    unsigned priv_number = (int) strtol(command->argv [0], NULL, 10);
    
    if (!privRequestExist (bot, priv_number))
    {
        postReply (bot->room, "There is no request by that number.", command->message);
        return;
    }
    
    PrivRequest **requests = bot->privRequests;
    PrivRequest *request = requests[priv_number - 1];
    PrivUser **users = bot->privUsers;
    
    PrivUser *user = getPrivUserByID(bot, request->userID);
    if (user) {
        user->privLevel = request->groupType;
    }
    else {
        bot->privUsers = users = realloc(users, ++bot->numOfPrivUsers);
        users [bot->numOfPrivUsers - 1] = createPrivUser (request->userID, request->username, request->groupType);
    }
    
    
    char *username = requests [priv_number - 1]->username;
    char *message;
    
    asprintf (&message, "Privilege request number %d has been approved. (cc @%s)", priv_number, username);
    free (username);
    postReply (bot->room, message, command->message);
    
    deletePrivRequest (bot, priv_number);
    
    free (message);
    
    return;
}

void rejectPrivRequest (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (command->argc == 0)
    {
        postReply (bot->room, "**Usage:** `@FireAlarm reject privilege request [request number]`", command->message);
        return;
    }
    
    unsigned priv_number = (int) strtol(command->argv [0], NULL, 10);
    
    if (!privRequestExist (bot, priv_number))
    {
        postReply (bot->room, "There is no request by that number.", command->message);
        return;
    }
    
    PrivRequest **requests = bot->privRequests;
    
    char *message;
    char *username = requests [priv_number - 1]->username;
    
    asprintf (&message, "Privilege request number %d has been rejected. (cc @%s)", priv_number, username);
    free (username);
    postReply (bot->room, message, command->message);
    
    deletePrivRequest (bot, priv_number);
    
    free (message);
    
    return;
}

void printPrivRequests (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    char *message = malloc (sizeof (bot->totalPrivRequests * 100 + 200));
    
    postReply (bot->room, "The current privilege requests are: ", command->message);
    
    sprintf (message,
             "    Request Num   |     Username    |     Privilege Request Type    \n"
             "--------------------------------------------------------------------"
             );
    
    char *messageString = malloc (sizeof (200));
    char groupType [30];
    PrivRequest **requests = bot->privRequests;
    
    for (int i = 0; i < bot->totalPrivRequests; i ++)
    {
        if (requests [i]->groupType == 0)
        {
            strcpy (groupType, "Member");
        }
        else if (requests [i]->groupType == 1)
        {
            strcpy (groupType, "Bot Owner");
        }
        
        sprintf (messageString,
                 "       %d         |  %s             |        %s                     \n",
                 i + 1, requests [i]->username, groupType);
        
        sprintf (message + strlen (message), "%s", messageString);
    }
    
    postMessage (bot->room, message);
    
    free (message);
    free (messageString);
    
    return;
}

void isPrivileged (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    unsigned long userID;
    int check = 0;
    
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
    
    if (checkPrivUser (bot, userID) == 1)
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
    else if (checkPrivUser (bot, userID) == 2)
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

void amiPrivileged (RunningCommand *command, void *ctx)
{
    command->argc = 0;
    isPrivileged (command, ctx);
    return;
}

void printPrivUser (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    int check = 0;
    
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
    PrivUser **users = bot->privUsers;
    
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
        sprintf (messageString + strlen (messageString), "%s\n", messageStringOwners);
    }
    
    postMessage (bot->room, messageString);
    
    free (messageString);
    free (messageStringMembers);
    free (messageStringOwners);
    
    return;
}
