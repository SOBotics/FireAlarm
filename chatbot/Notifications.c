//
//  Notifications.c
//  chatbot
//
//  Created by Ashish Ahuja on 4/29/16.
//  Copyright © 2016 Fortunate-MAN. All rights reserved.
//

#include "Notifications.h"

Notify *createNotification (int type, long userID)
{
    Notify *n = malloc (sizeof (Notify));
    
    n->type = type;
    n->userID = userID;
    
    return n;
}

void deleteNotificaton (ChatBot *bot, Notify *notify)
{
    Notify *n = bot->notify;
    
    for (int i = bot->totalNotifications -1; i < bot->totalNotifications; i ++)
    {
        n[i] = n[i + 1];
    }
    
    bot->totalNotifications --;
    
    return;
}
