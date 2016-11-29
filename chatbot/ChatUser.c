//
//  ChatUser.c
//  chatbot
//
//  Created on 5/4/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "ChatUser.h"
#include <stdlib.h>
#include <string.h>

ChatUser *createUser(unsigned long id, const char *name) {
    ChatUser *u = malloc(sizeof(ChatUser));

    u->userID = id;
    u->name = malloc(strlen(name) + 1);
    strcpy(u->name, name);

    u->isModerator = 0;
    u->isRoomOwner = 0;

    return u;
}

SOUser *createSOUser (unsigned long userID, char *username, unsigned userRep)
{
    SOUser *u = malloc (sizeof (SOUser));

    u->userID = userID;
    u->username = malloc (strlen (username) + 1);
    strcpy (u->username, username);
    u->userRep = userRep;

    return u;
}

void deleteUser(ChatUser *u) {
    free(u->name);
    free(u);
}


