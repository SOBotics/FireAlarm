//
//  ChatRoom.c
//  chatbot
//
//  Created on 4/29/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "ChatRoom.h"
#include "ChatBot.h"
#include <stdlib.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>
#include <curl/curl.h>
#include <string.h>
#include "cJSON.h"
#include "Client.h"

typedef enum {
    MessagePosted = 1,
    MessageEdited = 2,
    UserEntered = 3,
    UserLeft = 4,
    RoomNameChanged = 5,
    MessageStarred = 6,
    DebugMessage = 7,
    UserMentioned = 8,
    MessageFlagged = 9,
    MessageDeleted = 10,
    FileAdded = 11,
    ModeratorFlag = 12,
    UserSettingsChanged = 13,
    GlobalNotification = 14,
    AccessLevelChanged = 15,
    UserNotification = 16,
    Invitation = 17,
    MessageReply = 18,
    MessageMovedOut = 19,
    MessageMovedIn = 20,
    TimeBreak = 21,
    FeedTicker = 22,
    UserSuspended = 29,
    UserMerged = 30,
    UsernameChanged = 34
}EventType;

ChatRoom *createChatRoom(Client *client, unsigned roomID) {
    ChatRoom *r = malloc(sizeof(ChatRoom));
    r->client = client;
    r->roomID = roomID;
    r->users = NULL;
    r->userCount = 0;
    r->lastPostTimestamp = 0;
    r->pendingMessageLinkedList = NULL;
    pthread_mutex_init(&r->clientLock, NULL);
    pthread_mutex_init(&r->pendingMessageLock, NULL);
    return r;
}

void getInfoForUsers(ChatRoom *r, ChatUser **users, unsigned userCount) {
    printf("Getting information for %d user%s...\n", userCount, userCount == 1 ? "" : "s");
    const int maxUrlLength = 256;
    char url[maxUrlLength];
    snprintf(url, maxUrlLength,
             "chat.%s/user/info",
             r->client->host
             );
    
    CURL *curl = r->client->curl;
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, url));
    
    char *userIDs = malloc(1);
    *userIDs = 0;
    
    for (int i = 0; i < userCount; i++) {
        ChatUser *user = users[i];
        if (user->userID == 0) {
            continue;
        }
        char *newUser;
        asprintf(&newUser, "%lu%s", user->userID,
                 (i == (userCount - 1) || (i == (userCount - 2) && users[userCount-1]->userID == 0) ? "" : ","));
        
        userIDs = realloc(userIDs, strlen(userIDs) + strlen(newUser) + 1);
        strcat(userIDs, newUser);
        
        free(newUser);
    }
    
    char *postFields;
    asprintf(&postFields, "ids=%s&roomId=%d", userIDs, r->roomID);
    free(userIDs);
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, postFields));
    free(postFields);
    
    OutBuffer buf;
    buf.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf));
    checkCURL(curl_easy_perform(curl));
    
    cJSON *json = cJSON_Parse(buf.data);
    free(buf.data);
    
    cJSON *userDatas;
    if (json) {
        if ((userDatas = cJSON_GetObjectItem(json, "users"))) {
            for (int i = 0; i < cJSON_GetArraySize(userDatas); i++) {
                cJSON *userData = cJSON_GetArrayItem(userDatas, i);
                ChatUser *user = getUserByID(r, cJSON_GetObjectItem(userData, "id")->valueint);
                
                user->isModerator = (cJSON_GetObjectItem(userData, "is_moderator")->type == cJSON_True);
                user->isRoomOwner = (cJSON_GetObjectItem(userData, "is_owner")->type == cJSON_True);
                if (user->isModerator || user->isRoomOwner) {
                    printf("%s is a %s!\n", user->name, user->isModerator ? "moderator" : "room owner");
                }
            }
        }
        cJSON_Delete(json);
    }
}

void addUserToRoom(ChatRoom *r, ChatUser *u) {
    printf("Registering user \"%s\" (%lu)\n", u->name, u->userID);
    r->users = realloc(r->users, (++r->userCount) * sizeof(ChatUser *));
    r->users[r->userCount - 1] = u;
}

void refreshUsers(ChatRoom *r) {
    pthread_mutex_lock(&r->clientLock);
    puts("Refreshing user list...");
    for (int i = 0; i < r->userCount; i++) {
        deleteUser(r->users[i]);
    }
    r->userCount = 0;
    ChatUser *console = createUser(0, "Console");
    console->isModerator = 1;
    console->isRoomOwner = 1;
    addUserToRoom(r, console);
    
    const int maxUrlLength = 256;
    char url[maxUrlLength];
    url[0] = 0;
    snprintf(url, maxUrlLength,
             "chat.%s/rooms/pingable/%d",
             r->client->host,
             r->roomID
             );
    
    CURL *curl = r->client->curl;
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, url));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    checkCURL(curl_easy_perform(curl));
    cJSON *json = cJSON_Parse(buffer.data);
    free(buffer.data);
    
    pthread_mutex_unlock(&r->clientLock);
    
    const int userCount = cJSON_GetArraySize(json);
    const time_t requestTime = time(NULL);
    for (int i = 0; i < userCount; i++) {
        cJSON *userData = cJSON_GetArrayItem(json, i);
        unsigned long userID = cJSON_GetArrayItem(userData, 0)->valueint;
        const char *username = cJSON_GetArrayItem(userData, 1)->valuestring;
        unsigned long lastSeen = cJSON_GetArrayItem(userData, 2)->valueint;
        //unsigned long lastTalked = cJSON_GetArrayItem(userData, 3)->valueint;
        //If this user has been seen less than 200 seconds ago, register them.
        if ((requestTime - lastSeen) < 200) {
            addUserToRoom(r, createUser(userID, username));
        }
    }
    cJSON_Delete(json);
    
    getInfoForUsers(r, r->users, r->userCount);
}

void webSocketRecieved(WebSocket *socket, char *data, size_t len);

void enterChatRoom(ChatRoom *room) {
    printf("Joining chat room %d...\n", room->roomID);
    pthread_mutex_lock(&room->clientLock);
    room->lastUpdateTimestamp = connectClientToRoom(room->client, room->roomID);
    
    
    pthread_mutex_unlock(&room->clientLock);
    refreshUsers(room);
    room->lastEventTime = time(NULL);
}

unsigned userIDToIndex(ChatRoom *r, unsigned long id) {
    for (int i = 0; i < r->userCount; i++) {
        if (r->users[i]->userID == id) return i;
    }
    return -1;
}

ChatUser *getUserByID(ChatRoom *r, unsigned long id) {
    int i = userIDToIndex(r, id);
    if (i != -1) {
        return r->users[i];
    }
    return NULL;
}

void removeUserByID(ChatRoom *r, unsigned long id) {
    int i = userIDToIndex(r, id);
    if (i == -1) {
        return;
    }
    r->userCount--;
    printf("Deleting user %s.\n", r->users[i]->name);
    for (deleteUser(r->users[i]); i < r->userCount; i++) {
        r->users[i] = r->users[i+1];
    }
    r->users = realloc(r->users, r->userCount * sizeof(ChatUser *));
}

int isUserInRoom (ChatRoom *r, long userID)
{
    if (getUserByID (r, userID) == NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

typedef struct _PendingMessage {
    char *text;
    struct _PendingMessage *next;
}PendingMessage;

void postMessage(ChatRoom *r, const char *text) {
    
    //if (!postMessage)
    //{
    //    return;
    //}
    //^^^^^^^^^^^^^^^^^^what's that supposed to do?
    
    //queue the message to avoid throttling
    pthread_mutex_lock(&r->pendingMessageLock);
    
    PendingMessage *new = malloc(sizeof(PendingMessage));
    new->text = malloc(strlen(text) + 1);
    strcpy(new->text, text);
    new->next = NULL;
    
    if (r->pendingMessageLinkedList) {
        PendingMessage *last = r->pendingMessageLinkedList;
        while (last->next) {
            last = last->next;
        }
        last->next = new;
    }
    else {
        r->pendingMessageLinkedList = new;
    }
    
    pthread_mutex_unlock(&r->pendingMessageLock);
}

void postReply(ChatRoom *r, const char *text, ChatMessage *message) {
    
    //if (!postMessage)
    //{
    //    return;
    //}
    //^^^^^^^^^^^^^^^^^^ huh?
    
    const size_t bufSize = strlen(text) + 16;
    char buf[bufSize];
    if (message->user->userID) {
        snprintf(buf, bufSize,
                 ":%lu %s",
                 message->id, text
                 );
    }
    else {
        snprintf(buf, bufSize,
                 "@Console %s",
                 text
                 );
    }
    postMessage(r, buf);
}

void handleQueuedMessages(ChatRoom *r) {
    if (time(NULL) - r->lastPostTimestamp < 2) {
        return;
    }
    pthread_mutex_lock(&r->pendingMessageLock);
    
    PendingMessage *m;
    if ((m = r->pendingMessageLinkedList)) {
        PendingMessage *m = r->pendingMessageLinkedList;
        char *unescapedText = m->text;
        
        
        pthread_mutex_lock(&r->clientLock);
        printf("%s\n", unescapedText);
        
        CURL *curl = r->client->curl;
        char *text = curl_easy_escape(curl, unescapedText, 0);
        const size_t maxRequestLength = strlen(text) + 256;
        char request[maxRequestLength];
        request[0] = 0;
        snprintf(request, maxRequestLength,
                 "fkey=%s&text=%s",
                 r->client->fkey, text
                 );
        curl_free((void*)text);
        
        char url[maxRequestLength];
        url[0] = 0;
        snprintf(url, maxRequestLength,
                 "chat.%s/chats/%d/messages/new",
                 r->client->host,
                 r->roomID
                 );
        
        OutBuffer buf;
        buf.data = NULL;
        checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf));
        checkCURL(curl_easy_setopt(curl, CURLOPT_POST, 1L));
        checkCURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request));
        checkCURL(curl_easy_setopt(curl, CURLOPT_URL, url));
        
#ifndef DEBUG
        checkCURL(curl_easy_perform(curl));
        long http_code = 0;
        checkCURL(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code));
        if (http_code == 200) {
            r->pendingMessageLinkedList = m->next;
            free(unescapedText);
            free(m);
        }
        else {
            puts("Failed to post message; will try again soon.");
        }
        
        free(buf.data);
#else
        
        r->pendingMessageLinkedList = m->next;
        free(unescapedText);
        free(m);
#endif
        
        
        r->lastPostTimestamp = time(NULL);
        pthread_mutex_unlock(&r->clientLock);
    }
    pthread_mutex_unlock(&r->pendingMessageLock);
}

ChatMessage *processChatEvent(ChatRoom *r, cJSON *event) {
    r->lastEventTime = time(NULL);
    
    int eventType = cJSON_GetObjectItem(event, "event_type")->valueint;
    
    ChatUser *user = NULL;
    
    switch (eventType) {
        case MessagePosted:
        case MessageEdited:
        case UserEntered:
            ;
            //A user posted, edited, or entered.  Register him if needed.
            unsigned long userID = cJSON_GetObjectItem(event, "user_id")->valueint;
            user = getUserByID(r, userID);
            
            if (user == NULL) {
                const char *username = cJSON_GetObjectItem(event, "user_name")->valuestring;
                user = createUser(userID, username);
                addUserToRoom(r, user);
                getInfoForUsers(r, &user, 1);
            }
            break;
        case UserLeft:
            if (cJSON_GetObjectItem(event, "room_id")->valueint == r->roomID) {
                //A user left.
                printf("%s left the room.\n", cJSON_GetObjectItem(event, "user_name")->valuestring);
                removeUserByID(r, cJSON_GetObjectItem(event, "user_id")->valueint);
            }
            break;
        case UserMentioned:
        case MessageReply:
            break;
        default:
            printf("New event type: %d\n", eventType);
    }
    
    if (eventType == 1 || eventType == 2) {
        //A user posted or edited.
        const char *content = cJSON_GetObjectItem(event, "content")->valuestring;
        unsigned messageID = cJSON_GetObjectItem(event, "message_id")->valueint;
        cJSON *parent = cJSON_GetObjectItem(event, "parent_id");
        unsigned replyID = parent ? parent->valueint : 0;
        ChatMessage *m = createChatMessage(content, messageID, replyID, user);
        printf(
               "%s: %s\n",
               user->name,
               content
               );
        return m;
        //parseCommand(content);
    }
    return NULL;
}

ChatMessage **processChatRoomEvents(ChatRoom *room) {
    //Calculate the time interval between chat room polls.
    time_t timeDifference = time(NULL) - room->lastEventTime;
    unsigned interval = (timeDifference / 60.0) * 1000;
    if      (interval < 1000) interval = 1000;
    else if (interval > 5000) interval = 5000;
    
    struct pollfd fd;
    fd.fd = STDIN_FILENO;
    fd.events = POLLIN;
    int stdinAvailable = poll(&fd, 1, interval);
    
    if (stdinAvailable == -1) {
        fprintf(stderr, "Failed to poll stdin!\n");
    }
    else if (stdinAvailable) {
        ChatMessage **messageBuffer = malloc(sizeof(ChatMessage*) * 2);
        int maxStdinMessageSize = 256;
        char stdinBuf[maxStdinMessageSize];
        fgets(stdinBuf, maxStdinMessageSize, stdin);
        stdinBuf[strlen(stdinBuf) - 1] = 0; //remove the newline
        ChatMessage *message = createChatMessage(stdinBuf, 0, 0, room->users[0]);
        messageBuffer[0] = message;
        messageBuffer[1] = NULL;
        return messageBuffer;
    }
    
    handleQueuedMessages(room);
    
    
    pthread_mutex_lock(&room->clientLock);
    const int maxRequestLength = 256;
    char roomIDBuffer[maxRequestLength/4];
    char postBuffer[maxRequestLength];
    postBuffer[0] = 0;
    
    OutBuffer buffer;
    buffer.data = NULL;
    snprintf(
             roomIDBuffer, maxRequestLength/4,
             "r%d",
             room->roomID);
    snprintf(
             postBuffer, maxRequestLength,
             "fkey=%s&%s=%lu",
             room->client->fkey, roomIDBuffer, room->lastUpdateTimestamp);
    char urlBuffer[maxRequestLength];
    snprintf(
             urlBuffer, maxRequestLength,
             "chat.%s/events",
             room->client->host
             );
    checkCURL(curl_easy_setopt(room->client->curl, CURLOPT_URL, urlBuffer));
    checkCURL(curl_easy_setopt(room->client->curl, CURLOPT_POST, 1L));
    checkCURL(curl_easy_setopt(room->client->curl, CURLOPT_POSTFIELDS, postBuffer));
    checkCURL(curl_easy_setopt(room->client->curl, CURLOPT_WRITEDATA, &buffer));
    
    curl_easy_perform(room->client->curl);
    
    cJSON *json = cJSON_Parse(buffer.data);
    free(buffer.data);
    pthread_mutex_unlock(&room->clientLock);
    if (!json) {
        return NULL;
    }
    //puts(cJSON_Print(json));
    cJSON *roomData = cJSON_GetObjectItem(json, roomIDBuffer);
    cJSON *events = cJSON_GetObjectItem(roomData, "e");
    cJSON *time = cJSON_GetObjectItem(roomData, "t");
    if (time) {
        room->lastUpdateTimestamp = time->valueint;
    }
    int eventCount = events ? cJSON_GetArraySize(events) : 0;
    
    
    ChatMessage **messageBuffer = NULL;
    size_t messageBufferSize = 0;
    for (int i = 0; i < eventCount; i++) {
        cJSON *event = cJSON_GetArrayItem(events, i);
        ChatMessage *message = processChatEvent(room, event);
        if (message) {
            messageBuffer = realloc(messageBuffer, (++messageBufferSize) * sizeof(ChatMessage *));
            messageBuffer[messageBufferSize-1] = message;
        }
    }
    messageBuffer = realloc(messageBuffer, ++messageBufferSize * sizeof(ChatMessage *));
    messageBuffer[messageBufferSize - 1] = NULL;
    
    cJSON_Delete(json);
    
    return messageBuffer;
}

void leaveRoom(ChatRoom *r) {
    pthread_mutex_lock(&r->clientLock);
    
    const unsigned size = 256;
    char postBuffer[size];
    snprintf(postBuffer, size,
             "fkey=%s",
             r->client->fkey
             );
    checkCURL(curl_easy_setopt(r->client->curl, CURLOPT_COPYPOSTFIELDS, postBuffer));
    snprintf(postBuffer, size,
             "chat.%s/chats/leave/%d",
             r->client->host, r->roomID
             );
    checkCURL(curl_easy_setopt(r->client->curl, CURLOPT_URL, postBuffer));
    
    OutBuffer buf;
    buf.data = NULL;
    checkCURL(curl_easy_setopt(r->client->curl, CURLOPT_WRITEDATA, &buf));
    
    checkCURL(curl_easy_perform(r->client->curl));
    
    free(buf.data);
    
    
    pthread_mutex_unlock(&r->clientLock);
}
