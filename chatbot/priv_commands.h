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
    
    if (command->argc != 2)
    {
        postReply (bot->room, "**Usage:** privilege user [user id] [group]", command->message);
        return;
    }
    
    long userID = (long) strtol(command->argv[0], NULL, 10);
    char *privType = command->argv [1];
    int groupType = privilegeNamed(privType);
    
    if (userID <= 0)
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    
    PrivUser **users = bot->privUsers;
    
    PrivUser *user = getPrivUserByID(bot, userID);
    if (user) {
        user->privLevel |= groupType;
    }
    else {
        user = createPrivUser(userID, groupType);
        users = bot->privUsers = realloc(users, (bot->numOfPrivUsers + 1) * sizeof(PrivUser*));
        
        users[bot->numOfPrivUsers] = user;
        
        bot->numOfPrivUsers ++;
    }
    
    char *messageString;
    
    asprintf (&messageString, "Added %s privileges to user ID %lu.", privType, userID);
    
    postReply (bot->room, messageString, command->message);
    
    return;
}

void removeUserPriv (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (command->argc != 2)
    {
        postReply (bot->room, "**Usage:** unprivilege user [user id] [group]", command->message);
        return;
    }
    
    long userID = (long) strtol(command->argv[0], NULL, 10);
    char *privType = command->argv [1];
    int groupType = privilegeNamed(privType);
    
    if (userID <= 0)
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    
    PrivUser *user = getPrivUserByID(bot, userID);
    if (user) {
        user->privLevel &= ~groupType;
    }
    
    char *messageString;
    
    asprintf (&messageString, "Removed %s privileges from user ID %lu.", privType, userID);
    
    postReply (bot->room, messageString, command->message);
    
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
        return;
    }
    if (getPrivUserByID(bot, userID) && getPrivUserByID(bot, userID)->privLevel & groupID) {
        postReply(bot->room, "You already have that privilege", command->message);
        return;
    }
    
    bot->privRequests = realloc(bot->privRequests, (++bot->totalPrivRequests + 1) * sizeof(PrivRequest*));
    bot->privRequests [bot->totalPrivRequests - 1] = createPrivRequest (
                                                                        userID,
                                                                        groupID
                                                                        );
    bot->privRequests[bot->totalPrivRequests] = NULL;
    
    char *message;
    asprintf(&message, "Request #%d created.", bot->totalPrivRequests);
    postReply(bot->room, message, command->message);
    
    
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
        users [bot->numOfPrivUsers - 1] = createPrivUser (request->userID, request->groupType);
    }
    
    
    char *username = getUsernameByID(bot, requests [priv_number - 1]->userID);
    char *message;
    
    asprintf (&message, "Privilege request number %d has been approved. (cc @%s)", priv_number, username);
    //free (username);
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
    char *username = getUsernameByID(bot, requests [priv_number - 1]->userID);
    
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
    
    const size_t maxMessage = bot->totalPrivRequests * 200 + 200;
    char *message = malloc (maxMessage);
    
    postReply (bot->room, "The current privilege requests are: ", command->message);
    
    snprintf (message, maxMessage,
              "    Request Num   |     Username    |     Privilege Request Type    \n"
              "--------------------------------------------------------------------\n"
              );
    
    char *messageString = malloc (200);
    char groupType [30];
    PrivRequest **requests = bot->privRequests;
    
    for (int i = 0; i < bot->totalPrivRequests; i ++)
    {
        strcpy(groupType, getPrivilegeGroups()[requests [i]->groupType]);
        
        snprintf (messageString, 200,
                  "       %d         |  %s             |        %s                     \n",
                  i + 1, getUsernameByID(bot, requests [i]->userID), groupType);
        
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
    
    if (command->argc == 1) {
        check = privilegeNamed(command->argv[0]);
    }
    else if (command->argc > 1) {
        postReply(bot->room, "**Usage**: membership [group]", command->message);
        return;
    }
    
    postReply (bot->room, "The privilege groups are: ", command->message);
    PrivUser **users = bot->privUsers;
    unsigned userCount = bot->numOfPrivUsers;
    
    char *message = malloc(1);
    *message = 0;
    
    char *newString;
    
    for (char **groups = getPrivilegeGroups(); *groups; groups++) {
        unsigned groupID = privilegeNamed(*groups);
        if (groupID == 0 || (check && check != groupID)) {
            continue;
        }
        asprintf(&newString, "%s:\n", *groups);
        message = realloc(message, strlen(message) + strlen(newString) + 1);
        strcat(message, newString);
        free(newString);
        
        for (int i = 0; i < userCount; i++) {
            PrivUser *user = users[i];
            if (user->privLevel & groupID) {
                asprintf(&newString, "%s\n", getUsernameByID(bot, user->userID));
                message = realloc(message, strlen(message) + strlen(newString) + 1);
                strcat(message, newString);
                free(newString);
            }
        }
        message = realloc(message, strlen(message) + 2);
        strcat(message, "\n");
    }
    const char *modInfo = "Mods and room owners can run any command, regardless of privileges.";
    message = realloc(message, strlen(message) + strlen(modInfo) + 1);
    strcat(message, modInfo);
    
    postMessage (bot->room, message);
    
    free(message);
    
    return;
}
