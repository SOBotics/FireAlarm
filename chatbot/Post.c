//
//  Post.c
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "Post.h"
#include <string.h>
#include <stdlib.h>

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
    
    snprintf (request, max
              "https://api.stackexchange.com/2.2/questions/%lu?order=desc&sort=activity&site=stackoverflow&filter=!5-dm_.B4Dkg%29AM2phiJMbbo4oPnNg.YWfy%2838d",
              postID);
              
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
        return NULL;
    }
    
    cJSON *backoff;
    if ((backoff = cJSON_GetObjectItem(json, "backoff"))) {
        char *str;
        asprintf(&str, "Recieved backoff: %d", backoff->valueint);
        postMessage(bot->room, str);
        free(str);
    }
    
    int cvCount = cJSON_GetObjectItem (postJSON, "close_vote_count")->valueint;
    
    cJSON_Delete (json);
    
    return cvCount;
}
