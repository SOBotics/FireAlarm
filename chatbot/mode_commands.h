//
//  chatbot
//  mode_commands.h
//
//  Created by Ashish Ahuja on 25/06/2016
//  Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

#ifndef mode_commands_h
#define mode_commands_h

void changeMode (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    if (command->argc == 0 || command->argc == 1)
    {
        postReply (bot->room, "**Usage:** `@FireAlarm mode [enable/disbale] [group name] [optional item]", command->message);
        return;
    }

    int check = 0;

    if (command->argc == 3)
    {
        check = 1;
    }

    char *option = command->argv [0];
    char *group = command->argv [1];
    char *item = NULL;
    if (check)
    {
        item = command->argv [2];
    }

    int enable = 0;
    int disable = 0;
    int report = 0;
    int filter = 0;
    int keywordFilter = 0;
    int lengthFilter = 0;
    int message = 0;

    if (strcmp (option, "enable") == 0)
    {
        enable = 1;
    }
    else if (strcmp (option, "disable") == 0)
    {
        disable = 1;
    }
    else
    {
        char *message;
        asprintf (&message, "`%s` is invalid. You can either enter `enable` or `disable`.", option);
        postReply (bot->room, message, command->message);
        free (message);
        free (group);
        free (item);
        free (option);
        return;
    }

    if (strcmp (group, "report") == 0)
    {
        report = 1;
    }
    else if (strcmp (group, "filter") == 0)
    {
        if (!check)
        {
            postReply (bot->room, "If you use the group `filter`, you need to provide a third argument specifying the filter type.", command->message);
            return;
        }

        filter = 1;

        if (strcmp (item, "keyword") == 0)
        {
            keywordFilter = 1;
        }
        else if (strcmp (item, "length") == 0)
        {
            lengthFilter = 1;
        }
    }
    else if (strcmp (group, "message") == 0 || strcmp (group, "messages") == 0)
    {
        message = 1;
    }
    else
    {
        char *message;
        asprintf (&message, "Invalid group type %s. ", group);
        postReply (bot->room, message, command->message);
        free (message);
        //free (group);
        //free (item);
        //free (option);
        return;
    }

    if (disable)
    {
        if (report)
        {
            disableMode (bot, 0);
        }
        else if (keywordFilter)
        {
            disableMode (bot, 1);
        }
        else if (lengthFilter)
        {
            disableMode (bot, 2);
        }
        else if (message)
        {
            disableMode (bot, 3);
        }
    }
    else if (enable)
    {
        if (report)
        {
            enableMode (bot, 0);
        }
        else if (keywordFilter)
        {
            enableMode (bot, 1);
        }
        else if (lengthFilter)
        {
            enableMode (bot, 2);
        }
        else if (message)
        {
            enableMode (bot, 3);
        }
    }

    char *toPost;
    asprintf (&toPost, "`%s` has been %sd successfully.", group, option);
    postReply (bot->room, toPost, command->message);
    free (toPost);
    //free (group);
    //free (item);
    //free (option);
    return;
}

void printCurrentMode (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    char *currentMode = malloc (sizeof (char) * 100);

    Modes *modes = bot->modes;

    if (!modes->reportMode)
    {
        sprintf (currentMode, "Reporting is off, ");
    }
    else if (modes->reportMode)
    {
        sprintf (currentMode, "Reporting is on, ");
    }

    if (!modes->keywordFilter)
    {
        sprintf (currentMode + strlen (currentMode), "the keyword filter is off, ");
    }
    else if (modes->keywordFilter)
    {
        sprintf (currentMode + strlen (currentMode), "the keyword filter is on, ");
    }

    if (!modes->lengthFilter)
    {
        sprintf (currentMode + strlen (currentMode), "the length filter is off, ");
    }
    else if (modes->lengthFilter)
    {
        sprintf (currentMode + strlen (currentMode), "the length filter on, ");
    }

    if (!modes->messagePost)
    {
        sprintf (currentMode + strlen (currentMode), "and posting messages is off.");
    }
    else if (modes->messagePost)
    {
        sprintf (currentMode + strlen (currentMode), "and posting messages is on");
    }

    postReply (bot->room, currentMode, command->message);
    free (currentMode);

    return;
}

#endif /* mode_commands.h */
