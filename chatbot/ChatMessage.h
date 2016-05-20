//
//  ChatMessage.h
//  chatbot
//
//  Created on 5/4/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef ChatMessage_h
#define ChatMessage_h

#include <stdio.h>
#include "ChatUser.h"

typedef struct _ChatMessage {
    unsigned long id;
    unsigned long replyID;  //If this message is a reply, the message that it is a reply to.  0 otherwise.
    char *content;
    ChatUser *user;
}ChatMessage;

ChatMessage *createChatMessage(const char *text, unsigned long messageID, unsigned long replyID, ChatUser *user);
void deleteChatMessage(ChatMessage *message);

#endif /* ChatMessage_h */
