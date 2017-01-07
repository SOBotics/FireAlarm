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

    PrivRequest **requests = bot->privRequests;

    if (command->argc != 2)
    {
        postReply (bot->room, "**Usage:** privilege user [user id] [group]", command->message);
        return;
    }

    long userID = (long) strtol(command->argv[0], NULL, 10);
    char *privType = command->argv [1];
    int groupType = privilegeNamed(privType);

    if (!isValidUserID (bot, userID))
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }
    unsigned i;
    //printf ("User ID is %lu\n", userID);
    for (i = 0; i < bot->totalPrivRequests; i ++)
    {
        printf ("requests [%u]->userID = %lu\n", i, requests [i]->userID);
        printf ("requests [%u]->groupType = %d\n", i, requests [i]->groupType);
        if (requests [i]->userID == userID)
        {
            puts ("Deleting priv request..");
            //postMessage (bot->room, "Deleting priv request..");
            deletePrivRequest (bot, i);
        }
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
    char *username = getUsernameByID(bot, userID);
    char *noSpaces = malloc (strlen (username) + 1);
    strcpy (noSpaces, username);
    removeSpaces (noSpaces);

    asprintf (&messageString, "Added %s privileges to user %s (ID %lu) (cc @%s).", privType, username, userID, noSpaces);

    postReply (bot->room, messageString, command->message);
    free (messageString);
    free (username);
    free (noSpaces);

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

    if (!isValidUserID (bot, userID))
    {
        postReply (bot->room, "Please enter a valid User ID.", command->message);
        return;
    }

    PrivUser *user = getPrivUserByID(bot, userID);
    if (user) {
        user->privLevel &= ~groupType;
    }

    char *messageString;
    char *username = getUsernameByID (bot, userID);
    char *noSpaces = malloc (strlen (username) + 1);
    strcpy (noSpaces, username);
    removeSpaces (noSpaces);

    asprintf (&messageString, "Removed %s privileges from user %s (ID %lu) (cc @%s).", privType, username, userID, noSpaces);

    postReply (bot->room, messageString, command->message);
    free (messageString);
    free (username);
    free (noSpaces);

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
    removeSpaces (username);

    asprintf (&message, "Privilege request number %d has been approved. (cc @%s)", priv_number, username);
    //free (username);
    postReply (bot->room, message, command->message);

    deletePrivRequest (bot, priv_number);

    free (message);
    free(username);

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
    removeSpaces (username);

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

    if (bot->totalPrivRequests < 1)
    {
        postReply (bot->room, "There are no privilege requests currently.", command->message);
        return;
    }

    const size_t maxMessage = bot->totalPrivRequests * 200 + 200;
    char *message = malloc (maxMessage);

    postReply (bot->room, "The current privilege requests are: ", command->message);

    snprintf (message, maxMessage,
              "        Request Number    |"
              "      Username      |"
              "    Privilege Request Type    \n"
              "    -------------------"
              "------------------"
              "-------------------------------\n"
              );

    char *messageString = malloc (200);
    char groupType [30];
    PrivRequest **requests = bot->privRequests;

    for (int i = 0; i < bot->totalPrivRequests; i ++)
    {
        strcpy(groupType, getPrivilegeGroups()[requests [i]->groupType]);

        char *username = getUsernameByID(bot, requests [i]->userID);
        snprintf (message + strlen (message), 200,
                  "              %d           |"
                  "   %s   |"
                  "        %6s                     \n",
                  i + 1, username, groupType);
        free(username);

        //sprintf (message + strlen (message), "%s", messageString);
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
    PrivUser **users = bot->privUsers;
    if (users == NULL)
    {
        postReply (bot->room, "there are no users in the privilege groups.", command->message);
        return;
    }
    postReply (bot->room, "The privilege groups are: ", command->message);
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
                char *username = getUsernameByID(bot, user->userID);
                asprintf(&newString, "%s\n", username);
                free(username);
                message = realloc(message, strlen(message) + strlen(newString) + 1);
                strcat(message, newString);
                free(newString);
            }
        }
        message = realloc(message, strlen(message) + 2);
        strcat(message, "\n");
    }
    const char *modInfo = "Moderaters and room owners can run any command, regardless of privileges.";
    message = realloc(message, strlen(message) + strlen(modInfo) + 1);
    strcat(message, modInfo);

    postMessage (bot->room, message);

    free(message);

    return;
}

void printPrivCommands (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Command **commands = bot->commands;

    char *str = malloc (sizeof (char) * 100 * 256);

    sprintf (str, "Commands which can be used by users in 'member' group: \n");
    unsigned i;

    for (i = 0; commands [i]; i ++)
    {
        if (commands [i]->privileges == 1)
        {
            sprintf (str + strlen (str), "%s \n", commands [i]->name);
        }
    }

    sprintf (str + strlen (str), "\n");
    sprintf (str + strlen (str), "Commands which can be used by users in 'owner group: \n");

    for (i = 0; commands [i]; i ++)
    {
        if (commands [i]->privileges == 2)
        {
            sprintf (str + strlen (str), "%s \n", commands[i]->name);
        }
    }

    const char *modInfo = "Moderaters and room owners can run any command, regardless of privileges.";
    str = realloc (str, strlen (str) + strlen (modInfo) + 1);
    strcat (str, modInfo);

    postReply (bot->room, "Commands run by users in different privilege groups are:", command->message);
    postMessage (bot->room, str);
    free (str);

    return;
}
