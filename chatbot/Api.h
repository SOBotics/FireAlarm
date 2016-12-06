//
//  Api.h
//  chatbot
//
//  Created by Ashish Ahuja on 11/05/16.
//  Copyright © 2016 Fortunate-MAN (Ashish Ahuja). All rights reserved.
//

#ifndef Api_h

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "cJSON.h"
#include "Client.h"
#include "Privileges.h"
//#include "ChatBot.h"

#define BUFFER_SIZE  (512 * 1024) //512 KB

typedef struct {
    unsigned isBackoff;
    long int backoffTime;
    long long backoffAt;
    unsigned isError;
}Backoff;

typedef struct {
    char *apiFilter;
    char *apiKey;
    Backoff *backoff;
    int apiQuota;
}ApiCaller;

//https://github.com/akheron/jansson/blob/2.9/doc/github_commits.c#L30-L34
struct write_result
{
    char *data;
    int pos;
};

Backoff *createBackoff (unsigned isBackoff, long int backoffTime, long long backoffAt, unsigned isError);
ApiCaller *createApiCaller (char *apiFilter, char *apiKey, Backoff *backoff, int apiQuota);
char *createApiFilter (ChatBot *bot, char *filter);
cJSON *makeApiCall (ChatBot *bot, char *request);
cJSON *SE_apiGET (ChatBot *bot, char *request);
int getApiQuota (ChatBot *bot);
Post *getPostByID (ChatBot *bot, unsigned long postID);
unsigned getUserRepByID (ChatBot *bot, unsigned long userID);
static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream);
char *makeGHApiCall (ChatBot *bot, const char *request);
cJSON *GH_apiGET (ChatBot *bot, const char *request);
char *getLatestCommit (ChatBot *bot);

#endif /* Api.h */
