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

void deleteUser(ChatUser *u) {
    free(u->name);
    free(u);
}

char *getUsernameByID (ChatBot *bot, long userID)
{
    pthread_mutex_lock(&bot->room->clientLock);
    const int maxUrlLength = 256;
    char url[maxUrlLength];
    url[0] = 0;
    snprintf(url, maxUrlLength,
             "chat.%s/rooms/pingable/%d",
             bot->room->client->host,
             bot->room->roomID
             );
    
    CURL *curl = bot->room->client->curl;
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, url));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    checkCURL(curl_easy_perform(curl));
    cJSON *json = cJSON_Parse(buffer.data);
    free(buffer.data);
    
    pthread_mutex_unlock(&bot->room->clientLock);
    
    const int userCount = cJSON_GetArraySize(json);
    
    for (int i = 0; i < userCount; i ++)
    {
        cJSON *item = cJSON_GetArrayItem(json, i);
        unsigned long user_id = cJSON_GetArrayItem(item, 0)->valueint;
        
        if (user_id == userID)
        {
            const char *username = cJSON_GetArrayItem(item, 1)->valuestring;
            char *result = malloc(strlen(username) + 1);
            strcpy(result, username);
            cJSON_Delete (json);
            return result;
        }
    }
    
    cJSON_Delete (json);
    return NULL;
}

int isUserInPingableList (ChatBot *bot, long userID)
{
    if (getUsernameByID (bot, userID) == NULL)
    {
        return 0;
    }
    else 
    {
        return 1;
    }
}
