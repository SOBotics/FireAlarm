//
//  Post.c
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "Post.h"
#include "ChatBot.h"
#include "Client.h"
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

Post *createPost(const char *title, const char *body, unsigned long postID, unsigned char isAnswer, unsigned long userID) {
    Post *p = malloc(sizeof(Post));
    p->title = malloc(strlen(title) + 1);
    strcpy(p->title, title);
    
    p->body = malloc(strlen(body) + 1);
    strcpy(p->body, body);
    
    p->postID = postID;
    p->isAnswer = isAnswer;
    p->userID = userID;
    
    return p;
}

void deletePost(Post *p) {
    free(p->title);
    free(p->body);
    free(p);
}

int getCloseVotesByID (ChatBot *bot, unsigned long postID)
{
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    
    unsigned max = 256;
    char request [max];
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL,
                                   "api.stackexchange.com/2.2/filters/create"
                                   "?unsafe=false&key="API_KEY
                                   ));
    checkCURL(curl_easy_perform(curl));
        
    cJSON *json = cJSON_Parse(buffer.data);
    free(buffer.data);
    buffer.data = NULL;
        
    cJSON *items = cJSON_GetObjectItem(json, "items");
    char *filter = cJSON_GetObjectItem(cJSON_GetArrayItem(items, 0), "filter")->valuestring;
    
    snprintf (request, max,
              "https://api.stackexchange.com/2.2/questions/%lu?order=desc&sort=activity&site=stackoverflow&filter=%s",
              postID, filter);
              
    free (filter);
              
    curl_easy_setopt(curl, CURLOPT_URL, request);
    
    checkCURL(curl_easy_perform(curl));
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""));
    
    
    pthread_mutex_unlock(&bot->room->clientLock);
    
    cJSON *json = cJSON_Parse(buffer.data);
    
    free(buffer.data);
    
    if (!json || cJSON_GetObjectItem(json, "error_id")) {
        if (json) {
            cJSON_Delete(json);
        }
        puts("Error fetching post!");
        return 0;
    }
    
    cJSON *backoff;
    if ((backoff = cJSON_GetObjectItem(json, "backoff"))) {
        char *str;
        asprintf(&str, "Recieved backoff: %d", backoff->valueint);
        postMessage(bot->room, str);
        free(str);
    }
    
    int cvCount = cJSON_GetObjectItem (json, "close_vote_count")->valueint;
    
    cJSON_Delete (json);
    
    return cvCount;
}

int isPostClosed (ChatBot *bot, unsigned long userID)
{
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    
    unsigned max = 256;
    char request [max];
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL,
                                   "api.stackexchange.com/2.2/filters/create"
                                   "?unsafe=false&key="API_KEY
                                   ));
    checkCURL(curl_easy_perform(curl));
        
    cJSON *json = cJSON_Parse(buffer.data);
    free(buffer.data);
    buffer.data = NULL;
        
    cJSON *items = cJSON_GetObjectItem(json, "items");
    char *filter = cJSON_GetObjectItem(cJSON_GetArrayItem(items, 0), "filter")->valuestring;
    
    snprintf (request, max,
              "https://api.stackexchange.com/2.2/questions/%lu?order=desc&sort=activity&site=stackoverflow&filter=%s",
              postID, filter);
              
    free (filter);
              
    curl_easy_setopt(curl, CURLOPT_URL, request);
    
    checkCURL(curl_easy_perform(curl));
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""));
    
    
    pthread_mutex_unlock(&bot->room->clientLock);
    
    cJSON *json = cJSON_Parse(buffer.data);
    
    free(buffer.data);
    
    if (!json || cJSON_GetObjectItem(json, "error_id")) {
        if (json) {
            cJSON_Delete(json);
        }
        puts("Error fetching post!");
        return 0;
    }
    
    cJSON *backoff;
    if ((backoff = cJSON_GetObjectItem(json, "backoff"))) {
        char *str;
        asprintf(&str, "Recieved backoff: %d", backoff->valueint);
        postMessage(bot->room, str);
        free(str);
    }
    
    char *closedReason = cJSON_GetObjectItem (json, "closed_reason")->valuestring;
    
    cJSON_Delete (json);
    
    if (closedReason == NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
