//
//  users_commands.h
//  chatbot
//
//  Created by Jonathan Keller on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef users_commands_h
#define users_commands_h

void listPingable(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
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
    char *reply;
    asprintf(&reply, "%d pingable users", userCount);
    postReply(bot->room, reply, command->message);
    free(reply);
    
    char *messageString = malloc(64 * userCount);
    for (int i = 0; i < userCount; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        unsigned long userID = cJSON_GetArrayItem(item, 0)->valueint;
        const char *username = cJSON_GetArrayItem(item, 1)->valuestring;
        snprintf(messageString + strlen(messageString), 64,
                 "    %s (%lu)\n",
                 username,
                 userID
                 );
    }
    postMessage(bot->room, messageString);
}

#endif /* users_commands_h */
