//
//  Api.c
//  chatbot
//
//  Created by Ashish Ahuja on 11/05/16.
//  Copyright © 2016 Fortunate-MAN (Ashish Ahuja). All rights reserved.
//
//  Note: Some of the code here has been taken from https://github.com/akheron/jansson/blob/2.9/doc/github_commits.c
//

//#include "Api.h"
#include "ChatBot.h"

Backoff *createBackoff (unsigned isBackoff, long int backoffTime, long long backoffAt, unsigned isError)
{
    Backoff *b = malloc (sizeof (Backoff));

    b->isBackoff = isBackoff;
    b->backoffTime = backoffTime;
    b->backoffAt = backoffAt;
    b->isError = isError;

    return b;
}

ApiCaller *createApiCaller (char *apiFilter, char *apiKey, Backoff *backoff, int apiQuota)
{
    ApiCaller *c = malloc (sizeof (ApiCaller));

    c->apiFilter = malloc (strlen (apiFilter) + 1);
    strcpy (c->apiFilter, apiFilter);

    c->apiKey = malloc (strlen (apiKey) + 1);
    strcpy (c->apiKey, apiKey);

    c->backoff = backoff;
    c->apiQuota = apiQuota;

    return c;
}

char *createApiFilter (ChatBot *bot, char *filter)
{
    char *request;
    asprintf (&request, "api.stackexchange.com/2.2/filters/create?include=%s&unsafe=false&key=%s", filter, bot->api->apiKey);
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;

    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));

    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, request));
    free (request);
    checkCURL(curl_easy_perform(curl));

    cJSON *json = cJSON_Parse(buffer.data);
    free(buffer.data);
    buffer.data = NULL;

    cJSON *items = cJSON_GetObjectItem(json, "items");
    char *apiFilter = cJSON_GetObjectItem(cJSON_GetArrayItem(items, 0), "filter")->valuestring;
    cJSON_Delete (json);
    pthread_mutex_unlock (&bot->room->clientLock);

    return apiFilter;
}

cJSON *makeApiCall (ChatBot *bot, char *request)
{
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;

    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));

    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

    curl_easy_setopt(curl, CURLOPT_URL, request);

    checkCURL(curl_easy_perform(curl));

    checkCURL(curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""));

    pthread_mutex_unlock(&bot->room->clientLock);

    cJSON *json = cJSON_Parse(buffer.data);

    free(buffer.data);

    return json;
}

cJSON *SE_apiGET (ChatBot *bot, char *request)
{
    ApiCaller *caller = bot->api;
    Backoff *backoff = caller->backoff;

    //Backoff and error checking
    if (backoff->isError)
    {
        return NULL;
    }
    if (backoff->isBackoff)
    {
        long long currentTime = (long long) time (NULL);
        //Checking if the backoff is over
        /*if ((currentTime - backoff->backoffAt) > backoff->backoffTime)
        {
            backoff->isBackoff = 0;
        }
        else
        {
            return NULL;
        }*/
        postMessage (bot->room, "Blocked by backoff!");
        return NULL;
    }

    //Now we check if the api filter is NULL or not. If it is NULL, we set the filter to "default"
    if (caller->apiFilter == NULL)
    {
        fputs ("Api Filter is NULL! Setting the filter to default...", stderr);
        strcpy (caller->apiFilter, "default");
    }

    //Now get the json from the api
    cJSON *json = makeApiCall (bot, request);

    char *jsonInStr = cJSON_Print (json);

    //Checking if we violated a backoff
    if (!strstr (jsonInStr, "quota_remaining") || strstr (jsonInStr, "error_id"))
    {
        backoff->isError = 1;
        free (jsonInStr);
        char *message;
        asprintf (&message, "The `quota_remaining` property was not in the API response. **Error (cc @ashish)** Violation of backoff at %s. Backing off from all requests permanently."
                  ,getCurrentUTCTime ());
        fputs (message, stderr);
        postMessage (bot->room, message);
        free (message);
        return NULL;
    }
    free (jsonInStr);

    //Checking if we got a backoff
    cJSON *apiBackoff;
    if ((apiBackoff = cJSON_GetObjectItem (json, "backoff")))
    {
        backoff->isBackoff = 1;
        backoff->backoffTime = apiBackoff->valueint;
        backoff->backoffAt = (long long) time (NULL);
        char *message;
        asprintf (&message, "Backoff received for %d seconds on request `%s` at %s. (cc @ashish)", apiBackoff->valueint, request, getCurrentUTCTime());
        puts (message);
        postMessage (bot->room, message);
        free (message);
        return NULL;
    }

    return json;
}

int getApiQuota (ChatBot *bot)
{
    char *request;
    asprintf (&request, "api.stackexchange.com/2.2/info?site=stackoverflow&filter=%s&key=%s", bot->api->apiFilter, bot->api->apiKey);
    cJSON *json = SE_apiGET (bot, request);
    if (json == NULL)
    {
        return -1;
    }
    //Updating the latest api quota
    bot->api->apiQuota = cJSON_GetObjectItem (json, "quota_remaining")->valueint;

    int quota = cJSON_GetObjectItem (json, "quota_remaining")->valueint;
    cJSON_Delete (json);
    return quota;
}

Post *getPostByID (ChatBot *bot, unsigned long postID)
{
    char *request;
    asprintf (&request, "api.stackexchange.com/questions/%lu?site=stackoverflow&filter=%s&key=%s", postID, bot->api->apiFilter, bot->api->apiKey);
    cJSON *json = SE_apiGET (bot, request);
    free (request);
    if (json == cJSON_NULL || json == NULL)
    {
        return NULL;
    }
    //Updating the latest api quota
    bot->api->apiQuota = cJSON_GetObjectItem (json, "quota_remaining")->valueint;

    cJSON *postJSON = cJSON_GetArrayItem(cJSON_GetObjectItem(json, "items"), 0);
    if (postJSON == NULL || postJSON == cJSON_NULL)
    {
        cJSON_Delete (json);
        return NULL;
    }

    char *title = cJSON_GetObjectItem(postJSON, "title")->valuestring;
    char *body = cJSON_GetObjectItem(postJSON, "body")->valuestring;
    char *type = "question";
    unsigned long userID = 0;
    unsigned userRep = 0;
    char *username = NULL;

    //Checking if OP is deleted
    if (strcmp (cJSON_GetObjectItem (cJSON_GetObjectItem (postJSON, "owner"), "user_type")->valuestring, "does_not_exist") == 0)
    {
        puts ("OP is deleted!");
        userID = 0;
    }
    else
    {
        userID = cJSON_GetObjectItem(cJSON_GetObjectItem(postJSON, "owner"), "user_id")->valueint;
        userRep = cJSON_GetObjectItem (cJSON_GetObjectItem (postJSON, "owner"), "reputation")->valueint;
        username = cJSON_GetObjectItem(cJSON_GetObjectItem(postJSON, "owner"), "display_name")->valuestring;
    }

    SOUser *user = createSOUser(userID, username, userRep);

    Post *p = createPost(title, body, postID, strcmp(type, "answer") == 0, user);

    cJSON_Delete(json);
    return p;
}

unsigned getUserRepByID (ChatBot *bot, unsigned long userID)
{
    char *request;
    asprintf (&request, "api.stackexchange.com/2.2/users/%lu?site=stackoverflow&filter=default&key=%s", userID, bot->api->apiKey);
    cJSON *json = SE_apiGET (bot, request);
    free (request);
    if (json == NULL)
    {
        return 1;
    }

    //Updating the latest api quota
    bot->api->apiQuota = cJSON_GetObjectItem (json, "quota_remaining")->valueint;

    cJSON *userJSON = cJSON_GetArrayItem(cJSON_GetObjectItem(json, "items"), 0);
    if (userJSON == NULL) {
        cJSON_Delete(json);
        return 1;
    }

    unsigned userRep = cJSON_GetObjectItem (userJSON, "reputation")->valueint;
    cJSON_Delete (json);
    return userRep;
}

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
    struct write_result *result = (struct write_result *)stream;

    if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
    {
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

char *makeGHApiCall (ChatBot *bot, const char *request)
{
    pthread_mutex_lock (&bot->room->clientLock);
    CURL *curl = NULL;
    curl = curl_easy_init();
    CURLcode status;
    struct curl_slist *headers = NULL;
    long code;

    char *data = malloc (BUFFER_SIZE);
    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, request);

    /* GitHub commits API v3 requires a User-Agent header */
    headers = curl_slist_append(headers, "User-Agent: FireAlarm");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    status = curl_easy_perform(curl);
    if (status != 0)
    {
        fprintf (stderr, "error: unable to request data from %s:\n", request);
        return NULL;
    }

    //curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    pthread_mutex_unlock (&bot->room->clientLock);
    /*if(code != 200)
    {
        char *error;
        asprintf (&error, "Error: Github server responded with code %ld!", code);
        fprintf(stderr, "%s\n", error);
        postMessage (bot->room, error);
        free (error);
        return NULL;
    }*/
    curl_easy_cleanup (curl);
    curl_slist_free_all (headers);

    /* zero-terminate the result */
    data[write_result.pos] = '\0';

    return data;
}

cJSON *GH_apiGET (ChatBot *bot, const char *request)
{
    char *data = makeGHApiCall(bot, request);
    if (!data)
    {
        return NULL;
    }

    cJSON *json = cJSON_Parse(data);
    free (data);
    puts (cJSON_Print (json));

    return json;
}

char *getLatestCommit (ChatBot *bot)
{
    char url [256];
    snprintf (url, 256, "api.github.com/repos/NobodyNada/FireAlarm/commits");
    cJSON *json = GH_apiGET (bot, url);

    if (json == NULL)
    {
        puts ("json is null!");
        return NULL;
    }

    if (cJSON_GetArraySize (json) == 0)
    {
        fputs ("array size is 0!", stderr);
        return NULL;
    }
    cJSON *data = cJSON_GetArrayItem (json, 0);

    char *sha = cJSON_GetObjectItem (data, "sha")->valuestring;
    puts ("putting sha..");
    puts (sha);
    return sha;
}
