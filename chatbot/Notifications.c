//
//  Notifications.c
//  chatbot
//
//  Created by Ashish Ahuja on 4/29/16.
//  Copyright © 2016 Fortunate-MAN. All rights reserved.
//

#include "Notifications.h"
#include <stdlib.h>
#include <string.h>
#include "ChatBot.h"
#include "Post.h"


Notify *createNotification (int type, long userID, char *tag, int totalTags)
{
    Notify *n = malloc (sizeof (Notify));

    n->type = type;
    n->userID = userID;
    n->tags [totalTags -1] = tag;
    n->totalTags = totalTags;

    return n;
}

void deleteNotification (ChatBot *bot, Notify *notify)
{
    Notify **n = bot->notify;

    for (int i = bot->totalNotifications -1; i < bot->totalNotifications; i ++)
    {
        n[i] = n[i + 1];
    }

    bot->totalNotifications --;

    return;
}

void addTagNotification (Notify *notify, char *tag)
{
    char **tags = notify->tags;

    tags [notify->totalTags] = tag;
    notify->totalTags ++;

    return;
}

void deleteTagNotification (Notify *notify, char *tag)
{
    char **tags = notify->tags;
    int tagPos = -1;
    int i;

    for (i = 0; i < notify->totalTags; i ++)
    {
        if (strcmp (tag, tags [i]) == 0)
        {
            tagPos = i;
        }
    }

    if (tagPos == -1)
    {
        return;
    }

    for (i = tagPos; i < notify->totalTags; i ++)
    {
        tags [i] = tags [i + 1];
    }

    notify->totalTags --;

    return;
}

Notify *getNotificationByID (ChatBot *bot, long userID)
{
    Notify **notify = bot->notify;

    for (int i = 0; i < bot->totalNotifications; i ++)
    {
        if (notify [i]->userID == userID)
        {
            return notify [i];
        }
    }

    return NULL;
}

char *getNotificationString (ChatBot *bot, Post *post)
{
    /*char *str = malloc (bot->totalNotifications * 50);
    *str = 0;

    Notify **notify = bot->notify;

    for (int i = 0; i < bot->totalNotifications; i ++)
    {
        if (notify [i]->type == 1)
        {
            char *username = getUsernameByID (bot, notify [i]->userID);

            sprintf (str + strlen (str), "@");
            sprintf (str + strlen (str), "%s", username);
            sprintf (str + strlen (str), " ");
        }
        else if (notify [i]->type == 0)
        {
            if (isUserInRoom (bot->room, notify [i]->userID))
            {
                char *username = getUsernameByID (bot, notify [i]->userID);

                sprintf (str + strlen (str), "@");
                sprintf (str + strlen (str), "%s", username);
                sprintf (str + strlen (str), " ");
            }
        }
    }

    removeSpaces (str);

    return str;*/

    char *str = malloc ((bot->totalNotifications * 50) + 50);
    *str = 0;

    Notify **notify = bot->notify;
    char **postTags = getTagsByID (bot, post->postID);

    for (int i = 0; i < bot->totalNotifications; i ++)
    {
        if (notify [i]->type == 1)
        {
            if (notify [i]->totalTags == -1)
            {
                char *username = getUsernameByID (bot, notify [i]->userID);
                removeSpaces (username);

                sprintf (str + strlen (str), "@");
                sprintf (str + strlen (str), "%s", username);
                sprintf (str + strlen (str), " ");
            }
            else if (notify [i]->totalTags > -1)
            {
                if (postHaveNotifyTag (notify [i], postTags))
                {
                    char *username = getUsernameByID (bot, notify [i]->userID);
                    removeSpaces (username);

                    sprintf (str + strlen (str), "@");
                    sprintf (str + strlen (str), "%s", username);
                    sprintf (str + strlen (str), " ");
                }
            }
        }
        else if (notify [i]->type == 0)
        {
            if (isUserInRoom (bot->room, notify [i]->userID))
            {
                if (notify [i]->totalTags == -1)
                {
                    char *username = getUsernameByID (bot, notify [i]->userID);
                    removeSpaces (username);

                    sprintf (str + strlen (str), "@");
                    sprintf (str + strlen (str), "%s", username);
                    sprintf (str + strlen (str), " ");
                }
                else if (notify [i]->totalTags > -1)
                {
                    if (postHaveNotifyTag (notify [i], postTags))
                    {
                        char *username = getUsernameByID (bot, notify [i]->userID);
                        removeSpaces (username);

                        sprintf (str + strlen (str), "@");
                        sprintf (str + strlen (str), "%s", username);
                        sprintf (str + strlen (str), " ");
                    }
                }
            }
        }
    }

    return str;
}

int postHaveNotifyTag (Notify *notify, char **postTags)
{
    char **notifyTags = notify->tags;

    for (int i = 0; i < 5; i ++)
    {
        char *tag = postTags [i];

        for (int j = 0; j < notify->totalTags; j ++)
        {
            if (strcmp (tag, notify->tags [j]) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

int isTagInNotification (Notify *notify, char *tag)
{
    char **tags = notify->tags;

    for (int i = 0; i < notify->totalTags; i ++)
    {
        if (strcmp (tag, tags [i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}
