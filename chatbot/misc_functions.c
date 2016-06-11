//
//  misc_functions.c
//  chatbot
//
//  Created by Ashish Ahuja on 28/5/16.
//  Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

#include <ctype.h>

void lowercase (char *str)
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }
    
    return;
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
            cJSON_Delete (json);
            return username;
        }
    }
    
    cJSON_Delete (json);
    return 1;
}
