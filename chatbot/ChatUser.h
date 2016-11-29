//
//  ChatUser.h
//  chatbot
//
//  Created on 5/4/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef ChatUser_h
#define ChatUser_h

#include <stdio.h>

typedef struct _ChatUser {
    char *name;
    unsigned long userID;
    int isModerator;
    int isRoomOwner;
}ChatUser;

typedef struct _SOUser {
    char *username;
    unsigned long userID;
    unsigned userRep;
}SOUser;

ChatUser *createUser(unsigned long userID, const char *username);
void deleteUser(ChatUser *user);

#endif /* ChatUser_h */
