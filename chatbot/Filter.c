//
//  LQDetector.c
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include <string.h>
#include <stdlib.h>

#include "Filter.h"

Filter *createFilter(const char *desc, const char *filter, FilterType type, unsigned truePositives, unsigned falsePositives) {
    Filter *f = malloc(sizeof(Filter));
    f->desc = malloc(strlen(desc) + 1);
    strcpy(f->desc, desc);
    f->filter = malloc(strlen(filter) + 1);
    strcpy(f->filter, filter);
    
    f->type = type;
    f->truePositives = truePositives;
    f->falsePositives = falsePositives;
    
    int error;
    if (type == FILTER_REGEX && (error = regcomp(&f->regex, f->filter, REG_ICASE))) {
        const unsigned max = 1024;
        char msg[max];
        regerror(error, &f->regex, msg, max);
        fprintf(stderr, "Error compiling regex %s: %s\n", filter, msg);
        exit(EXIT_FAILURE);
    }
    return f;
}

static unsigned char matchRegexFilter(Post *post, Filter *f, unsigned *outStart, unsigned *outEnd) {
    regmatch_t match;
    int error = regexec(&f->regex, post->body, 1, &match, 0);
    if (error == REG_NOMATCH) {
        return 0;
    }
    if (error) {
        const unsigned max = 1024;
        char msg[max];
        regerror(error, &f->regex, msg, max);
        fprintf(stderr, "Error executing regex %s: %s\n", f->filter, msg);
        exit(EXIT_FAILURE);
    }
    if (outStart) *outStart = (unsigned)match.rm_so;
    if (outEnd) *outEnd = (unsigned)match.rm_eo;
    return 1;
}

unsigned char postMatchesFilter(ChatBot *bot, Post *post, Filter *filter, unsigned *outStart, unsigned *outEnd) {
    switch (filter->type) {
        case FILTER_TEXT:
            ;char *start = strstr(post->body, filter->filter);
            if (start) {
                if (outStart) *outStart = (unsigned)(start - post->body);
                if (outEnd) *outEnd = (unsigned)((start - post->body) + strlen(filter->filter));
                return 1;
            }
            return 0;
        case FILTER_REGEX:
            return matchRegexFilter(post, filter, outStart, outEnd);
        case FILTER_SHORTBODY:
            return strlen(post->body) < 500;
        case FILTER_TAG:
            return matchTagFilter (bot, post, filter);
        default:
            fprintf(stderr, "Invalid filter type %d\n", filter->type);
            exit(EXIT_FAILURE);
    }
}

unsigned postMatchesTagFilter (ChatBot *bot, Post *post)
{
    char *tagList;
    
    asprintf (&tagList,
              "licensing; copyright; ownership; blogs; open-source; jobs; apple; driver; drivers; privacy; spam; protection; search-engine; stack-overflow");
              
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    
    unsigned max = 256;
    char request[max];
    snprintf(request, max,
             "https://api.stackexchange.com/2.2/search?site=stackoverflow&order=desc&sort=activity&tagged=%s&key="API_KEY, tagList);
    
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
    
    unsigned totalPosts = cJSON_GetArraySize (json);
    
    for (int i = 0; i < totalPosts; i ++)
    {
        if (cJSON_GetObjectItem (json, "question_id")->valueint == post->postID)
        {
            cJSON_Delete (json);
            return 1;
        }
    }
    
    cJSON_Delete (json);
    return 0;
}

unsigned matchTagFilter (ChatBot *bot, Post *post, Filter *filter)
{
    char **tags = getTagsByID (bot, post->postID);
    
    for (int i = 0; i < 5; i ++)
    {
        if (strcmp (tags [i], filter->filter) == 0)
        {
            return 1;
        }
    }
    
    return 0;
}
