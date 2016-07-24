//
//  misc_commands.h
//  chatbot
//
//  Created on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef misc_commands_h
#define misc_commands_h

#include "Privileges.h"
#include "Notifications.h"

void listCommands(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    
    postReply(bot->room, "Running commands:", command->message);
    const size_t maxLineSize = 256;
    pthread_mutex_lock(&bot->runningCommandsLock);
    char *messageString = malloc(maxLineSize * (bot->runningCommandCount + 2));
    strcpy(messageString,
           "          User      |"
           "                      Command                      \n"
           "    ---------------------"
           "---------------------------------------------------\n"
           );
    for (int i = 0; i < bot->runningCommandCount; i++) {
        const size_t maxUsernameLength = 16;
        const size_t maxCommandLength = 48;
        
        char username[strlen(bot->runningCommands[i]->message->user->name) + 1];
        strcpy(username, bot->runningCommands[i]->message->user->name);
        char message[strlen(bot->runningCommands[i]->message->content) + 1];
        strcpy(message, bot->runningCommands[i]->message->content);
        
        //max username length = 16 chars
        if (strlen(username) > maxUsernameLength) {
            username[maxUsernameLength-3] = '.';
            username[maxUsernameLength-2] = '.';
            username[maxUsernameLength-1] = '.';
            username[maxUsernameLength] = '\0';
        }
        //max message length = 48 chars
        if (strlen(message) > maxCommandLength) {
            message[maxCommandLength-3] = '.';
            message[maxCommandLength-2] = '.';
            message[maxCommandLength-1] = '.';
            message[maxCommandLength] = '\0';
        }
        
        snprintf(messageString + strlen(messageString), maxLineSize,
                 "    %*s%*s|%*s%*s\n",
                 (int)((maxUsernameLength/2)+strlen(username)/2),
                 username,
                 (int)((maxUsernameLength/2)-strlen(username)/2),
                 "",
                 
                 
                 (int)((maxCommandLength/2)+strlen(message)/2),
                 message,
                 (int)((maxCommandLength/2)-strlen(message)/2),
                 ""
                 );
    }
    postMessage(bot->room, messageString);
    free(messageString);
    pthread_mutex_unlock(&bot->runningCommandsLock);
}

void help (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    postReply (bot->room, "I'm  [Fire Alarm](https://github.com/NobodyNada/chatbot), a bot which detects questions that need closing whenever they are posted or edited. [My command list is available here.](https://github.com/NobodyNada/chatbot/wiki/Commands)", command->message);
    return;
}

void aliveCheck (RunningCommand *command, void *ctx) 
{
    ChatBot *bot = ctx;
    
    postReply (bot->room, "Why did you think I was dead?", command->message);
    return;
}

void commandList (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    postReply (bot->room, "To see a complete list of commands, visit [this page](https://github.com/NobodyNada/chatbot/wiki/Commands).", command->message);
    return;
}

void changeThreshold (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    long newThreshold = strtol (command->argv [0], NULL, 10);
    
    if (newThreshold > 10000)
    {
        postReply (bot->room, "Please enter a threshold smaller than 10000.", command->message);
        return;
    }
    else if (newThreshold < 100)
    {
        postReply (bot->room, "Please enter a threshold bigger than 100.", command->message);
        return;
    }
    
    THRESHOLD = newThreshold;
    return;
}

void checkThreshold (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    char message [256];
    
    sprintf (message, "The current threshold is %ld.", THRESHOLD);
    
    postReply (bot->room, message, command->message);
    return;
}

void optIn (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    Notify **notify = bot->notify = realloc(bot->notify, ++bot->totalNotifications * sizeof(Notify*));
    
    notify[bot->totalNotifications - 1] = createNotification (0, command->message->user->userID);
    
    postReply (bot->room, "Opt-in successful. You will now be pinged for reports.", command->message);
    
    return;
}

void notifyMe (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    Notify **notify = bot->notify;
    
    notify[bot->totalNotifications - 1] = createNotification (1, command->message->user->userID);
    bot->totalNotifications ++;
    
    postReply (bot->room, "Notify submission successful. You will now be notified for reports.", command->message);
    
    return;
}

void optOut (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    long userID = command->message->user->userID;
    
    Notify *notify = getNotificationByID (bot, userID);
    
    if (notify == NULL || notify->type == 1)
    {
        postReply (bot->room, "You are already opted out.", command->message);
        return;
    }
    
    deleteNotification (bot, notify);
    
    postReply (bot->room, "Opt-out successful. You will now not be pinged for reports.", command->message);
    
    return;
}

void unnotifyMe (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    long userID = command->message->user->userID;
    
    Notify *notify = getNotificationByID (bot, userID);
    
    if (notify == NULL || notify->type == 0)
    {
        postReply (bot->room, "You are already not notified.", command->message);
        return;
    }
    
    deleteNotification (bot, notify);
    
    postReply (bot->room, "You will now not be notified for any reports.", command->message);
    
    return;
}

void say(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    size_t messageLength = 1;
    char *message = malloc(messageLength);
    *message = 0;
    
    for (int i = 0; i < command->argc; i++) {
        messageLength += strlen(command->argv[i]) + 2;
        message = realloc(message, messageLength);
        snprintf(message + strlen(message), messageLength - strlen(message), "%s ", command->argv[i]);
    }
    postMessage(bot->room, message);
    free(message);
}

void amINotified (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    long userID = command->message->user->userID;
    
    Notify *notify = getNotificationByID (bot, userID);
    
    if (notify == NULL)
    {
        postReply (bot->room, "You are not currently in the notification list.", command->message);
    }
    else if (notify->type == 0)
    {
        postReply (bot->room, "You are currently opted-in.", command->message);
    }
    else if (notify->type == 1)
    {
        postReply (bot->room, "You are currently notified of all reports.", command->message);
    }
    
    return;
}

void isNotified (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    if (command->argc == 0)
    {
        postReply (bot->room, "One argument required.", command->message);
        return;
    }
    
    long userID = strtol (command->argv [0], NULL, 10);
    
    if (!isValidUserID (bot, userID))
    {
        postReply (bot->room, "Please enter a valid user id.", command->message);
        return;
    }
    
    if (command->message->user->userID == userID)
    {
        amINotified (command, ctx);
        return;
    }
    
    Notify *notify = getNotificationByID (bot, userID);
    
    if (notify == NULL)
    {
        postReply (bot->room, "That user is no currently in the notification list.", command->message);
    }
    else if (notify->type == 0)
    {
        postReply (bot->room, "That user is currently opted-in.", command->message);
    }
    else if (notify->type == 1)
    {
        postReply (bot->room, "That user is currently notified for all reports.", command->message);
    }
    
    return;
}

void printNotifiedUsers (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    char *message = malloc (sizeof (127));
    char *messageString = malloc (sizeof (127 * (bot->totalNotifications + 2)));
    
    snprintf (message, 127, "There are %d total notified users: ", bot->totalNotifications);
    postReply (bot->room, message, command->message);
    
    Notify **notifications = bot->notify;
    
    sprintf (messageString, "Opted-in:\n");
    
    for (int i = 0; i < bot->totalNotifications; i ++)
    {
        Notify *notify = notifications [i];
        
        if (notify->type == 0)
        {
            sprintf (messageString + strlen (messageString), "%s\n", getUsernameByID (bot, notify->userID));
        }
    }
    
    sprintf (messageString + strlen (messageString), "\nNotified Users:\n");
    
    for (int i = 0; i < bot->totalNotifications; i ++)
    {
        Notify *notify = notifications [i];
        
        if (notify->type == 1)
        {
            sprintf (messageString + strlen (messageString), "%s\n", getUsernameByID (bot, notify->userID));
        }
    }
    
    postMessage (bot->room, messageString);
    
    free (messageString);
    free (message);
    
    return;
}

void apiQuota (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    
    unsigned max = 256;
    char request [max];
    
    snprintf (request, max,
              "api.stackexchange.com/2.2/info?site=stackoverflow");
              
    curl_easy_setopt(curl, CURLOPT_URL, request);
    
    checkCURL(curl_easy_perform(curl));
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""));
    
    
    pthread_mutex_unlock(&bot->room->clientLock);
    
    cJSON *json = cJSON_Parse(buffer.data);
    
    free(buffer.data);
    
    if (!json || cJSON_GetObjectItem(json, "error_id")) {
        if (json) {
            cJSON_Delete(json);
        }
        puts("Error fetching post!");
        return 0;
    }
    
    cJSON *backoff;
    if ((backoff = cJSON_GetObjectItem(json, "backoff"))) {
        char *str;
        asprintf(&str, "Recieved backoff: %d", backoff->valueint);
        postMessage(bot->room, str);
        free(str);
    }
    
    int apiQuota = cJSON_GetObjectItem (json, "quota_remaining")->valueint;
    
    cJSON_Delete (json);
    
    char *message;
    
    asprintf (&message, "The current api quota is %d.", apiQuota);
    postReply (bot->room, message, command->message);
    
    free (message);
    return;
}

#endif /* misc_commands_h */
