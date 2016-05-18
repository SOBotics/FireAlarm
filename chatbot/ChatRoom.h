//
//  ChatRoom.h
//  chatbot
//
//  Created by Jonathan Keller on 4/29/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef ChatRoom_h
#define ChatRoom_h

#include <stdio.h>
#include "Client.h"
#include "ChatMessage.h"
#include "ChatUser.h"
#include "Command.h"

typedef struct _ChatRoom {
    Client *client;
    unsigned roomID;
    unsigned long lastUpdateTimestamp;
    unsigned long lastPostTimestamp;
    unsigned userCount;
    time_t lastEventTime;
    ChatUser **users;
    struct _PendingMessage *pendingMessageLinkedList;
    pthread_mutex_t pendingMessageLock;
    pthread_mutex_t clientLock;
}ChatRoom;

ChatRoom *createChatRoom(Client *client, unsigned roomID);
void enterChatRoom(ChatRoom *room);
ChatMessage **processChatRoomEvents(ChatRoom *room);    //Returns a NULL-terminated list of chat messages.
                                                        //The list must be freed when done.
ChatUser *getUserByID(ChatRoom *r, unsigned long id);
void postMessage(ChatRoom *r, const char *text);
void postReply(ChatRoom *r, const char *text, ChatMessage *message);

#endif /* ChatRoom_h */
