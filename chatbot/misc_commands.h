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
#include "misc_functions.h"

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
    unsigned long userID = command->message->user->userID;
    Notify *n = getNotificationByID(bot, userID);

    if (n != NULL)
    {
        postReply (bot->room, "You are already opted-in.", command->message);
        return;
    }

    Notify **notify = bot->notify = realloc (bot->notify, ++bot->totalNotifications * sizeof (Notify*));
    notify [bot->totalNotifications - 1] = createNotification(0, userID);

    postReply (bot->room, "You will now be pinged for reports whenever you are in the room.", command->message);
    return;
   /* ChatBot *bot = ctx;
    int check = 0;
    char *arg;
    long userID = command->message->user->userID;

    if (command->argc > 0)
    {
        check = 1;
        arg = command->argv [0];
        removeChar (arg, '[');
        removeChar (arg, ']');
        if (arg [0] == 't' && arg [1] == 'a' && arg [2] == 'g' && arg [3] == ':')
        {
            arg += 4; // Chopping of the first 4 characters
        }
    }

    Notify *n = getNotificationByID (bot, userID);

    if (!check)
    {
        arg = NULL;
        if (n != NULL)
        {
            postReply (bot->room, "You are already in the notification list.", command->message);
            return;
        }
    }

    if (!isValidTag (bot, arg))
    {
        postReply (bot->room, "Please enter a valid tag.", command->message);
        return;
    }

    if (check)
    {
        if (isTagInNotification(n, arg))
        {
            char *message;
            asprintf (&message, "You are already opted in for [tag:%s].", arg);
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
    }

    if (n != NULL && check)
    {
        addTagNotification (n, arg);
        char *message;
        asprintf (&message, "Added [tag:%s] to your current notification.", arg);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    Notify **notify = bot->notify = realloc(bot->notify, ++bot->totalNotifications * sizeof(Notify*));

    notify[bot->totalNotifications - 1] = createNotification (0, command->message->user->userID, arg, -1);

    postReply (bot->room, "Opt-in successful. You will now be pinged for reports.", command->message);

    return;*/
}

void notifyMe (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    unsigned long userID = command->message->user->userID;
    Notify *n = getNotificationByID(bot, userID);

    if (n != NULL)
    {
        postReply (bot->room, "You are already in the notification list.", command->message);
        return;
    }

    Notify **notify = bot->notify = realloc (bot->notify, ++bot->totalNotifications * sizeof (Notify*));
    notify [bot->totalNotifications - 1] = createNotification(1, userID);

    postReply (bot->room, "You will now be pinged for reports.", command->message);
    return;
    /*ChatBot *bot = ctx;
    int check = 0;
    char *arg;
    long userID = command->message->user->userID;

    if (command->argc > 0)
    {
        check = 1;
        arg = command->argv [0];
        removeChar (arg, '[');
        removeChar (arg, ']');
        if (arg [0] == 't' && arg [1] == 'a' && arg [2] == 'g' && arg [3] == ':')
        {
            arg += 4; // Chopping of the first 4 characters
        }
    }

    Notify *n = getNotificationByID (bot, userID);

    if (!check)
    {
        arg = NULL;
        if (n != NULL && n->type == 1)
        {
            postReply (bot->room, "You are already in the notification list.", command->message);
            return;
        }
    }

    if (!isValidTag (bot, arg))
    {
        postReply (bot->room, "Please enter a valid tag.", command->message);
        return;
    }

    if (check)
    {
        if (isTagInNotification(n, arg))
        {
            char *message;
            asprintf (&message, "You are already opted in for [tag:%s].", arg);
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
    }

    if (n != NULL && check)
    {
        addTagNotification (n, arg);
        char *message;
        asprintf (&message, "Added [tag:%s] to your current notification.", arg);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    Notify **notify = bot->notify = realloc(bot->notify, ++bot->totalNotifications * sizeof(Notify*));

    notify[bot->totalNotifications - 1] = createNotification (1, command->message->user->userID, arg, -1);
    bot->totalNotifications ++;

    postReply (bot->room, "Notify submission successful. You will now be notified for reports.", command->message);

    return;*/
}

void optOut (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    unsigned long userID = command->message->user->userID;
    Notify *n = getNotificationByID(bot, userID);

    if (n == NULL)
    {
        postReply (bot->room, "You are already opted-out.", command->message);
        return;
    }

    deleteNotification(bot, n);
    postReply (bot->room, "You will no longer be pinged for reports.", command->message);
    return;
    /*ChatBot *bot = ctx;
    int check = 0;
    char *arg = NULL;

    if (command->argc > 0)
    {
        check = 1;
        arg = command->argv [0];
        removeChar (arg, '[');
        removeChar (arg, ']');
        if (arg [0] == 't' && arg [1] == 'a' && arg [2] == 'g' && arg [3] == ':')
        {
            arg += 4; // Chopping of the first 4 characters
        }
    }

    long userID = command->message->user->userID;

    Notify *notify = getNotificationByID (bot, userID);

    if (notify == NULL)
    {
        postReply (bot->room, "You are already opted out.", command->message);
        return;
    }

    int isUserOptedTag = 0;
    for (int i = 0; i < notify->totalTags; i ++)
    {
        if (strcmp (arg, notify->tags [i]) == 0)
        {
            isUserOptedTag = 1;
        }
    }

    if (!isUserOptedTag)
    {
        char *message;
        asprintf (&message, "You are already not opted in for [tag:%s].", arg);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    if (check && notify->totalTags == 1)
    {
        deleteNotification (bot, notify);
        postReply (bot->room, "Opt-out successful. You will now not be pinged for reports.", command->message);
        return;
    }
    else if (check)
    {
        deleteTagNotification (notify, arg);
        char *message;
        asprintf (&message, "Opt-out successful. You will now not be pinged for reports with [tag:%s].", arg);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    deleteNotification (bot, notify);

    postReply (bot->room, "Opt-out successful. You will now not be pinged for reports.", command->message);

    return*/
}

void unnotifyMe (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    unsigned long userID = command->message->user->userID;
    Notify *n = getNotificationByID(bot, userID);

    if (n == NULL)
    {
        postReply (bot->room, "You are already not in the notification list.", command->message);
        return;
    }

    deleteNotification(bot, n);
    postReply (bot->room, "You will no longer be pinged for reports.", command->message);
    return;
    /*ChatBot *bot = ctx;
    int check = 0;
    char *arg = NULL;

    if (command->argc > 0)
    {
        check = 1;
        arg = command->argv [0];
        removeChar (arg, '[');
        removeChar (arg, ']');
        if (arg [0] == 't' && arg [1] == 'a' && arg [2] == 'g' && arg [3] == ':')
        {
            arg += 4; // Chopping of the first 4 characters
        }
    }

    long userID = command->message->user->userID;

    Notify *notify = getNotificationByID (bot, userID);

    if (notify == NULL || notify->type == 1)
    {
        postReply (bot->room, "You are already not in the notification list.", command->message);
        return;
    }

    int isUserOptedTag = 0;
    for (int i = 0; i < notify->totalTags; i ++)
    {
        if (strcmp (arg, notify->tags [i]) == 0)
        {
            isUserOptedTag = 1;
        }
    }

    if (!isUserOptedTag)
    {
        char *message;
        asprintf (&message, "You are already not opted in for [tag:%s].", arg);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    if (check && notify->totalTags == 1)
    {
        deleteNotification (bot, notify);
        postReply (bot->room, "Opt-out successful. You will now not be pinged for reports.", command->message);
        return;
    }
    else if (check)
    {
        deleteTagNotification (notify, arg);
        char *message;
        asprintf (&message, "Opt-out successful. You will now not be pinged for reports with [tag:%s].", arg);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }


    deleteNotification (bot, notify);

    postReply (bot->room, "You will now not be notified for any reports.", command->message);

    return;*/
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

    /*int i;
    if (notify == NULL)
    {
        postReply (bot->room, "You are currently not in the notification list.", command->message);
        return;
    }
    else if (notify->type == 0)
    {
        if (notify->totalTags == -1)
        {
            postReply (bot->room, "You are currently opted in.", command->message);
            return;
        }
        else if (notify->totalTags > 0)
        {
            char *message = malloc (sizeof (char) * 127);
            sprintf (message, "You are currently opted in for the %s ", notify->totalTags ? "tag" : "tags");
            for (i = 0; i < notify->totalTags; i ++)
            {
                if (i == notify->totalTags - 1)
                {
                    sprintf (message + strlen (message), "[tag:%s].", notify->tags [i]);
                }
                else
                {
                    sprintf (message + strlen (message), "[tag:%s], ", notify->tags [i]);
                }
            }
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
    }
    else if (notify->type == 1)
    {
        if (notify->totalTags == -1)
        {
            postReply (bot->room, "You are currently notified of all reports.", command->message);
            return;
        }
        else if (notify->totalTags > 0)
        {
            char *message = malloc (sizeof (char) * 127);
            sprintf (message, "You are currently notified of report with the %s ", notify->totalTags ? "tag" : "tags");
            for (i = 0; i < notify->totalTags; i ++)
            {
                if (i == notify->totalTags -1)
                {
                    sprintf (message + strlen (message), "[tag:%s].", notify->tags [i]);
                }
                else
                {
                    sprintf (message + strlen (message), "[tag:%s], ", notify->tags [i]);
                }
            }
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
    }*/
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
        postReply (bot->room, "That user is not currently in the notification list.", command->message);
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

    //char *message = malloc (sizeof (127));
    //char *messageString = malloc (sizeof (127 * (bot->totalNotifications + 2)));
    char *message = malloc (sizeof (char) * 127);
    char *messageString = malloc (sizeof (char) * 64 * (bot->totalNotifications + 2));

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

void printApiQuota (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    int quota = apiQuota (bot);

    char *message;

    asprintf (&message, "The current api quota is %d.", quota);
    postReply (bot->room, message, command->message);

    free (message);
    return;
}

void modifyFilterThreshold (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    long newThreshold = strtol (command->argv [0], NULL, 10);

    if (newThreshold > 10000 || newThreshold < 100)
    {
        postReply (bot->room, "Please enter a threshold smaller than 10000 and bigger than 100.", command->message);
        return;
    }

    long oldThreshold = THRESHOLD;
    THRESHOLD = newThreshold;

    char *message;
    asprintf (&message, "The filter threshold has been changed to %ld from %ld.", oldThreshold, THRESHOLD);
    postReply (bot->room, message, command->message);
    free (message);
    return;
}

void addKeywordToFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    if (command->argc < 3)
    {
        postReply (bot->room, "Expected three arguments.", command->message);
        return;
    }

    char *keyword = command->argv [0];
    int truePositives = (int) strtol (command->argv [1], NULL, 10);
    int falsePositives = (int) strtol (command->argv [2], NULL, 10);

    if (truePositives < 0 || truePositives > 100 || falsePositives < 0 || falsePositives > 100)
    {
        postReply (bot->room, "Please enter values that are only positive and smaller than 100.", command->message);
        return;
    }

    Filter **filters = bot->filters;

    filters = realloc (filters, ++bot->filterCount * sizeof (Filter *));
    filters [bot->filterCount - 1] = createFilter (keyword, keyword, 0, truePositives, falsePositives, 0);

    char *message;
    asprintf (&message, "Keyword \"%s\" added to the filters.", keyword);
    postReply (bot->room, message, command->message);
    free (message);
    return;
}

void addTagToFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    if (command->argc < 3)
    {
        postReply (bot->room, "Expected three arguments.", command->message);
        return;
    }

    char *tag = command->argv [0];
    int truePositives = (int) strtol (command->argv [1], NULL, 10);
    int falsePositives = (int) strtol (command->argv [2], NULL, 10);

    if (truePositives < 0 || truePositives > 100 || falsePositives < 0 || falsePositives > 100)
    {
        postReply (bot->room, "Please enter values that are only positive and smaller than 100. ", command->message);
        return;
    }

    if (!isValidTag (bot, tag))
    {
        char *message;
        asprintf (&message, "[tag:%s] is not a valid tag.", tag);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    char *desc;
    asprintf (&desc, "[tag:%s]", tag);

    Filter **filters = bot->filters;

    filters = realloc (filters, ++bot->filterCount * sizeof (Filter *));
    filters [bot->filterCount - 1] = createFilter (desc, tag, 3, truePositives, falsePositives, 0);
    free (desc);

    char *str;
    asprintf (&str, "Tag \"%s\" added to the filters.", tag);
    postReply (bot->room, str, command->message);
    free (str);
    return;
}

void printTagsInFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    Filter **filters = bot->filters;
    char *str = malloc (bot->filterCount * 256);

    sprintf (str,
             "          Tag           |"
             "       True Positives   |"
             "      False Positives   \n"
             "    ---------------------"
             "-------------------------"
             "-------------------------\n");

    for (int i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == 3)
        {
            sprintf (str + strlen (str),
                     "         %s             |"
                     "            %d          |"
                     "            %d         \n",
                     filter->filter, filter->truePositives, filter->falsePositives);
        }
    }

    postReply (bot->room, "The tags in the current filter are:", command->message);
    postMessage (bot->room, str);
    free (str);
    return;
}

void printKeywordsInFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    Filter **filters = bot->filters;
    char *str = malloc (bot->filterCount * 256);

    sprintf (str,
             "       Keyword          |"
             "       True Positives   |"
             "      False Positives   \n"
             "    ---------------------"
             "-------------------------"
             "-------------------------\n");

    for (int i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == 0)
        {
            sprintf (str + strlen (str),
                     "         %s             |"
                     "            %d          |"
                     "            %d         \n",
                     filter->filter, filter->truePositives, filter->falsePositives);
        }
    }

    postReply (bot->room, "The keywords in the current filter are:", command->message);
    postMessage (bot->room, str);
    free (str);
    return;
}

void modifyKeywordFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    if (command->argc < 4)
    {
        postReply (bot->room, "Expected three arguments. **Usage:** `@FireAlarm filter modify keyword <keyword> true=<value> false=<value>`", command->message);
        return;
    }

    unsigned changeTruePositive = 1;
    unsigned changeFalsePositive = 1;
    int newTruePositive;
    int oldTruePositive;
    int newFalsePositive;
    int oldFalsePositive;

    char *keyword = command->argv [0];

    if (!isKeywordInFilter (bot, keyword))
    {
        char *message;
        asprintf (&message, "Keyword \"%s\" not in the filters.", keyword);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    char *str = command->argv [1];

    if (str [0] == 't' && str [1] == 'r' && str [2] == 'u' && str [3] == 'e' && str [4] == '=')
    {
        str += 5;
        newTruePositive = (int) strtol (str, NULL, 10);
        free (str);
    }
    else
    {
        newTruePositive = (int) strtol (command->argv [0], NULL, 10);
    }

    if (str [0] == 'f' && str [1] == 'a' && str [2] == 'l' && str [3] == 's' && str [4] == 'e' && str [5] == '=')
    {
        str += 6;
        newFalsePositive = (int) strtol (str, NULL, 10);
        free (str);
    }
    else
    {
        newFalsePositive = (int) strtol (command->argv [0], NULL, 10);
    }

    if (newTruePositive == -1)
    {
        changeTruePositive = 0;
    }
    if (newFalsePositive == -1)
    {
        changeFalsePositive = 0;
    }

    if (!changeTruePositive && !changeFalsePositive)
    {
        postReply (bot->room, "Looks like you do not want to change anything. ", command->message);
        return;
    }

    Filter *filter = getFilterByKeyword (bot, keyword);
    oldFalsePositive = filter->falsePositives;

    if (changeTruePositive)
    {
        filter->truePositives = newTruePositive;
    }
    if (changeFalsePositive)
    {
        filter->falsePositives = newFalsePositive;
    }

    char *message;

    if (changeTruePositive)
    {
        asprintf (&message, "Successfully changed true positives of filter \"%s\" from %d to %d.", keyword, oldTruePositive, newTruePositive);
    }
    if (changeFalsePositive)
    {
        asprintf (&message, "Successfully changed false positives of filter \"%s\" from %d to %d.", keyword, oldFalsePositive, newFalsePositive);
    }
    if (changeFalsePositive && changeTruePositive)
    {
        asprintf (&message, "Successfully change true positives of filter \"%s\" from %d to %d, and false positives from %d to %d.", keyword, oldTruePositive, newTruePositive, oldFalsePositive, newFalsePositive);
    }

    postReply (bot->room, message, command->message);
    free (message);
    return;
}

void filterInfo (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    if (command->argc < 2)
    {
        postReply (bot->room, "Expected two arguments.", command->message);
        return;
    }

    char *argOne = command->argv [0];
    int type = -1;

    if (strcmp (argOne, "keyword") == 0)
    {
        type = 0;
    }
    if (strcmp (argOne, "tag") == 0)
    {
        type = 1;
    }

    if (type == -1)
    {
        char *message;
        asprintf (&message, "First argument '%s' is invalid. Enter either `keyword` or `tag`.", argOne);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    char *argTwo = command->argv [1];

    if (!type)
    {
        if (!isKeywordInFilter (bot, argTwo))
        {
            char *message;
            asprintf (&message, "Keyword '%s' is not currently in the list of keyword in the filter.", argTwo);
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
        else if (isKeywordInFilter(bot, argTwo))
        {
            Filter *filter = getFilterByKeyword(bot, argTwo);
            char *message;
            asprintf (&message, "Keyword '%s' is currently in the filter. It has %d true positives, %d false positives, and its true positive rate is %f.", filter->filter, filter->truePositives, filter->falsePositives, filter->truePositives / (filter->truePositives + filter->falsePositives));
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
    }
    if (type)
    {
        if (!isTagInFilter (bot, argTwo))
        {
            char *message;
            asprintf (&message, "Tag [tag:%s] is not currently in the list of tags in the filter.", argTwo);
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
        else if (isTagInFilter (bot, argTwo))
        {
            Filter *filter = getFilterByTag(bot, argTwo);
            char *message;
            asprintf (&message, "Tag [tag:%s] is currently in the filter. It has %d true positives, %d false positives, and its true positive rate is %f.", filter->filter, filter->truePositives, filter->falsePositives, filter->truePositives / (filter->truePositives + filter->falsePositives));
            postReply (bot->room, message, command->message);
            free (message);
            return;
        }
    }

    return;
}

void printAccuracyOfFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    if (command->argc < 1)
    {
        postReply (bot->room, "Expected at least 1 argument.", command->message);
        return;
    }

    char *type = command->argv [0];
    int filterType = -1;

    if (strcmp (type, "keyword") == 0)
    {
        filterType = 0;
    }
    else if (strcmp (type, "tag") == 0)
    {
        filterType = 3;
    }
    else if (strcmp (type, "body") == 0 || strcmp (type, "length") == 0)
    {
        filterType = 2;
    }
    else
    {
        char *message;
        asprintf (&message, "'%s' is not a valid first argument. Enter either `keyword` or `tag` or `length`.", type);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    unsigned totalOccurences;
    unsigned trueOccurences;
    unsigned falseOccurences;
    unsigned i;
    float accuracy;
    Filter **filters = bot->filters;

    for (i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == filterType)
        {
            trueOccurences += filter->truePositives;
            falseOccurences += filter->falsePositives;
        }
    }

    totalOccurences = trueOccurences + falseOccurences;
    accuracy = trueOccurences / totalOccurences;

    char *message;
    if (!filterType)
    {
        asprintf (&message, "The accuracy of the keyword filter is %f, with %u true positives and %u false positives.", accuracy, trueOccurences, falseOccurences);
    }
    else if (filterType == 2)
    {
        asprintf (&message, "The accuracy of the body length filter is %f, with %u true positives and %u false positives.", accuracy, trueOccurences, falseOccurences);
    }
    else if (filterType == 3)
    {
        asprintf (&message, "The accuracy of the tag filter is %f, with %u true positives and %u false positives.", accuracy, trueOccurences, falseOccurences);
    }

    postReply (bot->room, message, command->message);
    free (message);

    return;
}

void printReportsByFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    if (command->argc < 1)
    {
        postReply (bot->room, "Expected at least 1 argument.", command->message);
        return;
    }

    unsigned printCount = 10;
    int typeToPrint = -1;
    unsigned filterType;

    if (command->argc > 1)
    {
        if (isStringContainingNumbers2 (command->argv [1]))
        {
            printCount = (unsigned) strtol (command->argv [1], NULL, 10);

            if (printCount < 1 || printCount > 10)
            {
                postReply (bot->room, "Please enter a second argument smaller than 11 and bigger than 0.", command->message);
                return;
            }
        }
        else
        {
            if (strcmp (command->argv [1], "true") == 0)
            {
                typeToPrint = 1;
            }
            else if (strcmp (command->argv [1], "unconfirmed") == 0 || strcmp (command->argv [1], "unknown") == 0)
            {
                typeToPrint = 2;
            }
            else if (strcmp (command->argv [1], "false") == 0)
            {
                typeToPrint = 0;
            }
            else
            {
                char *message;
                asprintf (&message, "\"%s\" is not a valid second argument. Enter either `true` or `false` or `unconfirmed`.", command->argv [1]);
                postReply (bot->room, message, command->message);
                free (message);
                return;
            }
        }
    }

    if (strcmp (command->argv [0], "keyword") == 0)
    {
        filterType = 0;
    }
    else if (strcmp (command->argv [0], "body") == 0 || strcmp (command->argv [0], "length") == 0)
    {
        filterType = 2;
    }
    else if (strcmp (command->argv [0], "tag") == 0)
    {
        filterType = 3;
    }
    else
    {
        char *message;
        asprintf (&message, "\"%s\" is not a valid first argument. Enter either `keyword` or `body` or `tag`.", command->argv [0]);
        postReply (bot->room, message, command->message);
        free (message);
        return;
    }

    char *toPrint = malloc (sizeof (char) * 11 * 256);
    Report **reports;
    unsigned i;

    reports = getReportsByFilter (bot, filterType, printCount);
    char *status;

    for (i = 0; i < printCount; i ++)
    {
        Report *report = reports [i];

        if (report == NULL)
        {
            break;
        }

        if (report->confirmation == -1)
        {
            asprintf (&status, "Unconfirmed");
        }
        else if (report->confirmation == 0)
        {
            asprintf (&status, "False Positive");
        }
        else if (report->confirmation == 1)
        {
            asprintf (&status, "True Positive");
        }

        if (typeToPrint == -1)
        {
            snprintf (toPrint, 255,
                      "%s: [%s](http://stackoverflow.com/%s/%lu) ([Message %lu](http://chat.stackoverflow.com/transcript/message/%lu#%lu))",
                      status, report->post->title, report->post->isAnswer ? "a" :"q", report->post->postID, report->messageID, report->messageID, report->messageID);
        }
        else
        {
            if (typeToPrint == 2 && !strcmp (status, "Unconfirmed"))
            {
                snprintf (toPrint, 255,
                      "%s: [%s](http://stackoverflow.com/%s/%lu) ([Message %lu](http://chat.stackoverflow.com/transcript/message/%lu#%lu))",
                      status, report->post->title, report->post->isAnswer ? "a" :"q", report->post->postID, report->messageID, report->messageID, report->messageID);
            }
            else if (typeToPrint == report->confirmation)
            {
                snprintf (toPrint, 255,
                      "%s: [%s](http://stackoverflow.com/%s/%lu) ([Message %lu](http://chat.stackoverflow.com/transcript/message/%lu#%lu))",
                      status, report->post->title, report->post->isAnswer ? "a" :"q", report->post->postID, report->messageID, report->messageID, report->messageID);
            }
        }
    }

    free (status);

    char *message;

    if (i < printCount)
    {
        printCount = i;
    }

    asprintf (&message, "%u reports in the %s filter are:", printCount, command->argv [0]);
    postReply (bot->room, message, command->message);
    postMessage (bot->room, toPrint);
    free (message);
    free (toPrint);

    return;
}

void printFilters (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    unsigned keywordType = 0;
    unsigned lengthType = 2;
    unsigned tagType = 3;
    unsigned totalFilters = 3;
    unsigned trueOccurences = 0;
    unsigned falseOccurences = 0;
    float accuracy = 0.0;
    char *str = malloc (sizeof (char) * totalFilters * 500);

    sprintf (str,
             "        Filter          |"
             "       Accuracy Rate    |"
             "       True Positives   |"
             "      False Positives   \n"
             "    ---------------------"
             "-------------------------"
             "-------------------------"
             "-------------------------\n");

    Filter **filters = bot->filters;

    for (int i = 0; i < totalFilters; i ++)
    {
        if (i == keywordType)
        {
            sprintf (str + strlen (str),
                     "     Keyword Filter     |");

            for (int j = 0; j < bot->filterCount; j ++)
            {
                if (filters [j]->type == keywordType)
                {
                    trueOccurences += filters [j]->truePositives;
                    falseOccurences += filters [j]->falsePositives;
                }
            }

            accuracy = trueOccurences / (trueOccurences + falseOccurences);

            sprintf (str + strlen (str),
                     "         %f       |"
                     "            %u         |"
                     "            %u          \n",
                     accuracy, trueOccurences, falseOccurences);

            trueOccurences = 0;
            falseOccurences = 0;
            accuracy = 0.0;
        }
        else if (i == lengthType)
        {
            sprintf (str + strlen (str),
                     "     Length Filter      |");

            for (int j = 0; j < bot->filterCount; j ++)
            {
                if (filters [j]->type == lengthType)
                {
                    trueOccurences += filters [j]->truePositives;
                    falseOccurences += filters [j]->falsePositives;
                }
            }

            accuracy = trueOccurences / (trueOccurences + falseOccurences);

            sprintf (str + strlen (str),
                     "         %f       |"
                     "            %u          |"
                     "            %u          \n",
                     accuracy, trueOccurences, falseOccurences);

            trueOccurences = 0;
            falseOccurences = 0;
            accuracy = 0.0;
        }
        else if (i == tagType)
        {
            sprintf (str + strlen (str),
                     "     Tag Filter         |");

            for (int j = 0; j < bot->filterCount; j ++)
            {
                if (filters [j]->type == tagType)
                {
                    trueOccurences += filters [j]->truePositives;
                    falseOccurences += filters [j]->falsePositives;
                }
            }

            accuracy = trueOccurences / (trueOccurences + falseOccurences);

            sprintf (str + strlen (str),
                     "            %f          |"
                     "            %u          |"
                     "            %u          \n",
                     accuracy, trueOccurences, falseOccurences);

            trueOccurences = 0;
            falseOccurences = 0;
            accuracy = 0.0;
        }
    }

    postReply (bot->room, "All the filters used by the bot are: ", command->message);
    postMessage (bot->room, str);
    free (str);
    return;
}

void disableFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Filter **filters = bot->filters;

    if (command->argc < 2)
    {
        postReply (bot->room, "Expected at least 1 argument. [See the commands page for more information.](https://git.io/vP6aq)", command->message);
        return;
    }

    char *toDisable = command->argv [0];
    int filterType = filterNamed(toDisable);

    if (filterType == -1)
    {
        char *msg;
        asprintf (&msg, "`%s` is an invalid argument. [See the commands page for more information.](https://git.io/vP6aq)", toDisable);
        postReply (bot->room, msg, command->message);
        free (msg);
        return;
    }

    for (unsigned i = 0; i < bot->filterCount; i ++)
    {
        if (filters [i]->type == filterType)
        {
            filters [i]->isDisabled = 1;
        }
    }

    char *str;
    asprintf (&str, "Filter `%s` has been disabled. ", toDisable);
    postReply(bot->room, str, command->message);
    free (str);
    return;
}

void enableFilter (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    Filter **filters = bot->filters;

    if (command->argc < 2)
    {
        postReply (bot->room, "Expected at least 1 argument. ", command->message);
        return;
    }

    char *toEnable = command->argv [0];
    int filterType = filterNamed (toEnable);

    if (filterType == -1)
    {
        char *msg;
        asprintf (&msg, "`%s` is an invalid argument. [See the commands page for more information.](https://git.io/vP6aq)", toEnable);
        postReply (bot->room, msg, command->message);
        free (msg);
        return;
    }

    for (unsigned i = 0; i < bot->filterCount; i ++)
    {
        if (filters [i]->type == filterType)
        {
            filters [i]->isDisabled = 0;
        }
    }

    char *str;
    asprintf (&str, "Filter `%s` has been enabled. ", toEnable);
    postReply(bot->room, str, command->message);
    free (str);
    return;
}

#endif /* misc_commands_h */
