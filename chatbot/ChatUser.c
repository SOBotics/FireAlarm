//
//  ChatUser.c
//  chatbot
//
//  Created by Jonathan Keller on 5/4/16.
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
    
    return u;
}

void deleteUser(ChatUser *u) {
    free(u->name);
    free(u);
}