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

Notify *createNotification (int type, long userID)
{
    Notify *n = malloc (sizeof (Notify));
    
    n->type = type;
    n->userID = userID;
    
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

char *getNotificationString (ChatBot *bot)
{
    char *str = malloc (bot->totalNotifications * 50);
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
    
    return str;
}
