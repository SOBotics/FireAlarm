//
//  Notifications.h
//  chatbot
//
//  Created by Ashish Ahuja on 4/29/16.
//  Copyright © 2016 Fortunate-MAN. All rights reserved.
//

#ifndef Notifications_h
#define Notifications_h

#include "Post.h"

typedef struct _ChatBot ChatBot;

typedef struct {
  long userID;
  //char **tags;
  //int totalTags; // -1, if all tags, otherwise number of tags
  int type;  // 0 if user has opted-in, 1 if user has notified in
}Notify;

Notify *createNotification (int type, long userID);
unsigned deleteNotification (ChatBot *bot, Notify *notify);
Notify *getNotificationByID (ChatBot *bot, long userID);
char *getNotificationString (ChatBot *bot, Post *post);

#endif /* Notifications.h */

