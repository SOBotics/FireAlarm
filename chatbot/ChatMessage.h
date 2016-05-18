//
//  ChatMessage.h
//  chatbot
//
//  Created by Jonathan Keller on 5/4/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef ChatMessage_h
#define ChatMessage_h

#include <stdio.h>
#include "ChatUser.h"

typedef struct _ChatMessage {
    unsigned long id;
    char *content;
    ChatUser *user;
}ChatMessage;

ChatMessage *createChatMessage(const char *text, unsigned long messageID, ChatUser *user);
void deleteChatMessage(ChatMessage *message);

#endif /* ChatMessage_h */
