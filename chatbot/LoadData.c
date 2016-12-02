//
//  LoadData.c
//  chatbot
//
//  Created by Ashish Ahuja on 10/22/16.
//  Copyright © 2016 Fortunate-MAN (Ashish Ahuja). All rights reserved.
//

#include "LoadData.h"
//#include "ChatBot.h"
//#include "Privileges.h"

typedef struct _PrivUser {
    long userID;
    int privLevel;  // 1 if member, 2 if bot owner.
}PrivUser;

typedef struct  _PrivRequest {
    long userID;
    int groupType;  // 0 if user wants to join membeers, 1 if user wants to join bot owners.
}PrivRequest;

ApiCaller *loadApiCaller ()
{
    puts ("Loading Api Caller...");
    FILE *file = fopen ("api.json", "r");
    if (!file || isFileEmpty (file))
    {
        fputs ("Could not read from ~/.chatbot/api.json. Exiting the program...", stderr);
        exit (EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buf = malloc(size+1);
    fread(buf, size, 1, file);
    buf[size] = 0;

    cJSON *json = cJSON_Parse (buf);
    free (buf);

    //First load backoff data
    unsigned isBackoff = cJSON_GetObjectItem (json, "backoff_isbackoff")->valueint;
    long int backoffTime = cJSON_GetObjectItem (json, "backoff_backofftime")->valueint;
    long long backoffAt = cJSON_GetObjectItem (json, "backoff_backoffat")->valueint;
    unsigned isError = cJSON_GetObjectItem (json, "backoff_iserror")->valueint;

    Backoff *backoff = createBackoff (isBackoff, backoffTime, backoffAt, isError);

    //Now loading the remaining data
    char *apiFilter = cJSON_GetObjectItem (json, "api_filter")->valuestring;
    char *apiKey = cJSON_GetObjectItem (json, "api_key")->valuestring;

    ApiCaller *api = createApiCaller (apiFilter, apiKey, backoff, -1);

    cJSON_Delete (json);
    fclose (file);

    return api;
}

void saveApiCaller (ApiCaller *caller)
{
    FILE *file = fopen ("api.json", "w");
    if (!file)
    {
        fputs ("Could not save to 'api.json'!", stderr);
        return;
    }

    cJSON *container = cJSON_CreateObject();

    cJSON_AddItemToObject(container, "backoff_isbackoff", cJSON_CreateNumber(caller->backoff->isBackoff));
    cJSON_AddItemToObject (container, "backoff_backofftime", cJSON_CreateNumber(caller->backoff->backoffTime));
    cJSON_AddItemToObject(container, "backoff_backoffat", cJSON_CreateNumber(caller->backoff->backoffAt));
    cJSON_AddItemToObject (container, "backoff_iserror", cJSON_CreateNumber (caller->backoff->isError));
    cJSON_AddItemToObject(container, "api_filter", cJSON_CreateString(caller->apiFilter));
    cJSON_AddItemToObject(container, "api_key", cJSON_CreateString (caller->apiKey));

    char *str = cJSON_Print (container);
    cJSON_Delete (container);

    fwrite(str, strlen(str), 1, file);

    fclose(file);

    free(str);

    return;
}

Log **loadLogs ()
{
    puts ("Loading Logs...");
    FILE *file = fopen ("logs.json", "r");
    if (!file || isFileEmpty (file))
    {
        fputs ("Could not read from ~/.chatbot/logs.json. Returning an empty list...", stderr);
        Log **logs = malloc (sizeof (Log*));
        *logs = NULL;
        return logs;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buf = malloc(size+1);
    fread(buf, size, 1, file);
    buf[size] = 0;

    cJSON *json = cJSON_Parse (buf);
    free (buf);

    unsigned total = cJSON_GetArraySize(json);
    Log **logs = malloc (sizeof (Log*) * (total + 1));

    for (unsigned i = 0; i < total; i ++)
    {
        cJSON *log = cJSON_GetArrayItem (json, i);

        unsigned type = cJSON_GetObjectItem (log, "type")->valueint;
        char *funcCaller = cJSON_GetObjectItem (log, "caller")->valuestring;
        char *location = cJSON_GetObjectItem (log, "location")->valuestring;
        size_t size = (size_t) cJSON_GetObjectItem (log, "size")->valueint;
        unsigned key = cJSON_GetObjectItem (log, "key")->valueint;
        char *time = cJSON_GetObjectItem (log, "time")->valuestring;
        char *message = cJSON_GetObjectItem (log, "message")->valuestring;

        logs [i] = createLog (type, funcCaller, location, size, key, message, time);
    }

    logs [total] = NULL;
    cJSON_Delete (json);
    fclose (file);

    return logs;
}

void saveLogs (Log **logs, unsigned totalLogs)
{
    FILE *file = fopen ("logs.json", "w");

    if (!file)
    {
        fputs ("Failed to open ~/.chatbot/logs.json/!", stderr);
        return;
    }

    if (logs == NULL)
        return;

    cJSON *json = cJSON_CreateArray ();

    for (unsigned i = 0; i < totalLogs; i ++)
    {
        Log *log = logs [i];
        cJSON *object = cJSON_CreateObject ();

        cJSON_AddItemToObject (object, "type", cJSON_CreateNumber (log->type));
        cJSON_AddItemToObject (object, "caller", cJSON_CreateString (log->funcCaller));
        cJSON_AddItemToObject (object, "location", cJSON_CreateString (log->location));
        cJSON_AddItemToObject (object, "size", cJSON_CreateNumber (log->size));
        cJSON_AddItemToObject (object, "key", cJSON_CreateNumber (log->key));
        cJSON_AddItemToObject (object, "time", cJSON_CreateString (log->time));
        cJSON_AddItemToObject (object, "message", cJSON_CreateString (log->message));

        cJSON_AddItemToArray (json, object);
    }

    char *str = cJSON_Print (json);
    cJSON_Delete (json);

    fwrite(str, strlen(str), 1, file);

    fclose(file);

    free(str);

    return;
}

Filter **loadFilters() {
    puts("Loading filters...");
    FILE *file = fopen("filters.json", "r");
    if (!file || isFileEmpty (file)) {
        puts("Could not read from ~/.chatbot/filters.json.  Creating a skeleton filter list...");
        Filter **filters = malloc(sizeof(Filter*) * 2);
        filters[0] = createFilter(
                                  "Example filter",
                                  "Regular expression to test against ^",   //^ is so this doesn't match anything
                                  FILTER_REGEX,
                                  0,
                                  0,
                                  0
                                  );

        filters[1] = NULL;
        return filters;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buf = malloc(size+1);
    fread(buf, size, 1, file);
    buf[size] = 0;

    fclose(file);

    cJSON *json = cJSON_Parse(buf);
    free(buf);

    unsigned filterCount = cJSON_GetArraySize(json);
    Filter **filters = malloc(sizeof(Filter*) * (filterCount + 1));

    for (int i = 0; i < filterCount; i++) {
        cJSON *filter = cJSON_GetArrayItem(json, i);
        const char *desc = cJSON_GetObjectItem(filter, "description")->valuestring;
        const char *expr = cJSON_GetObjectItem(filter, "expression")->valuestring;
        FilterType type = cJSON_GetObjectItem(filter, "type")->valueint;
        unsigned truePositives = cJSON_GetObjectItem(filter, "truePositives")->valueint;
        int falsePositives = cJSON_GetObjectItem(filter, "falsePositives")->valueint;
        unsigned isDisabled = cJSON_GetObjectItem (filter, "isDisabled")->valueint;
        if (falsePositives == -1) {
            falsePositives = 25 - truePositives;
            if (falsePositives < 0) {
                falsePositives = 0;
            }
        }
        filters[i] = createFilter(desc, expr, type, truePositives, falsePositives, isDisabled);
    }
    filters[filterCount] = NULL;

    cJSON_Delete(json);

    return filters;
}

PrivRequest **loadPrivRequests ()
{
    puts ("Loading Privilege Requests...");
    FILE *file = fopen ("privRequest.json", "r");

    if (!file)
    {
        puts ("privRequest.json does not exist. Returning an empty list...");
        PrivRequest **list = malloc(sizeof(PrivRequest*));
        *list = NULL;
        return list;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buf = malloc(size+1);
    fread(buf, size, 1, file);
    buf[size] = 0;

    cJSON *json = cJSON_Parse (buf);
    free (buf);

    unsigned total = cJSON_GetArraySize(json);
    PrivRequest **requests = malloc (sizeof (PrivRequest*) * (total + 1));

    for (int i = 0; i < total; i ++)
    {
        cJSON *request = cJSON_GetArrayItem(json, i);

        long userID = cJSON_GetObjectItem (request, "user_id")->valueint;
        int groupType = cJSON_GetObjectItem (request, "group_type")->valueint;

        requests [i] = createPrivRequest (userID, groupType);
    }

    requests[total] = NULL;

    cJSON_Delete (json);
    fclose (file);

    return requests;
}

Notify **loadNotifications ()
{
    puts ("Loading Notifications...");
    FILE *file = fopen ("notifications.json", "r");

    if (!file || isFileEmpty(file))
    {
        puts ("notifications.json does not exist. Returning an empty list...");
        Notify **list = malloc(sizeof(Notify*));
        *list = NULL;
        return list;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buf = malloc(size+1);
    fread(buf, size, 1, file);
    buf[size] = 0;

    cJSON *json = cJSON_Parse (buf);
    free (buf);

    unsigned total = cJSON_GetArraySize(json);
    Notify **notify = malloc (sizeof (Notify*) * (total + 1));

    for (int i = 0; i < total; i ++)
    {
        cJSON *n = cJSON_GetArrayItem(json, i);

        long userID = cJSON_GetObjectItem (n, "user_id")->valueint;
        int type = cJSON_GetObjectItem (n, "type")->valueint;

        notify [i] = createNotification (type, userID);
    }

    notify[total] = NULL;

    cJSON_Delete (json);
    fclose (file);

    return notify;
}

void saveNotifications (Notify **notify, unsigned totalNotifications)
{
    FILE *file = fopen ("notifications.json", "w");

    cJSON *json = cJSON_CreateArray();

    for (int i = 0; i < totalNotifications; i ++)
    {
        Notify *n = notify [i];
        cJSON *object = cJSON_CreateObject();

        cJSON_AddItemToObject (object, "user_id", cJSON_CreateNumber (n->userID));
        cJSON_AddItemToObject (object, "type", cJSON_CreateNumber (n->type));

        cJSON_AddItemToArray(json, object);
    }

    char *str = cJSON_Print(json);
    cJSON_Delete(json);

    fwrite(str, strlen(str), 1, file);

    fclose (file);
    free (str);

    return;
}

void savePrivRequests (PrivRequest **requests, unsigned totalRequests)
{
    FILE *file = fopen ("privRequest.json", "w");

    cJSON *json = cJSON_CreateArray();

    for (int i = 0; i < totalRequests; i ++)
    {
        PrivRequest *request = requests [i];
        cJSON *object = cJSON_CreateObject();

        cJSON_AddItemToObject (object, "user_id", cJSON_CreateNumber (request->userID));
        cJSON_AddItemToObject (object, "group_type", cJSON_CreateNumber (request->groupType));

        cJSON_AddItemToArray(json, object);
    }

    char *str = cJSON_Print(json);
    cJSON_Delete(json);

    fwrite(str, strlen(str), 1, file);

    fclose (file);
    free (str);

    return;
}

PrivUser **loadPrivUsers ()
{
    puts ("Loading Privileged Users...");
    FILE *file = fopen ("privUsers.json", "r");

    if (!file)
    {
        puts ("privUsers.json does not exist. Creating skeleton file...");
        PrivUser **users = malloc(sizeof(PrivUser*) * 3);

        users [0] = createPrivUser (3476191, 3);    //NobodyNada
        users [1] = createPrivUser (5735775, 3);    //Ashish Ahuja ツ
        users [2] = NULL;
        return users;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buf = malloc(size+1);
    fread(buf, size, 1, file);
    buf[size] = 0;

    cJSON *json = cJSON_Parse(buf);
    free(buf);

    unsigned privUsersCount = cJSON_GetArraySize(json);
    PrivUser **users = malloc(sizeof(PrivUser*) * (privUsersCount + 1));

    for (int i = 0; i < privUsersCount; i ++)
    {
        cJSON *user = cJSON_GetArrayItem(json, i);

        long userID = cJSON_GetObjectItem (user, "user_id")->valueint;
        int privLevel = cJSON_GetObjectItem (user, "priv_level")->valueint;
        users [i] = createPrivUser (userID, privLevel);
    }
    users [privUsersCount] = NULL;

    cJSON_Delete(json);
    return users;
}

void savePrivUsers (PrivUser **users, unsigned privUsersCount)
{
    cJSON *json = cJSON_CreateArray();

    for (int i = 0; i < privUsersCount; i ++)
    {
        PrivUser *user = users [i];
        cJSON *object = cJSON_CreateObject();
        cJSON_AddItemToObject (object, "user_id", cJSON_CreateNumber (users [i]->userID));
        cJSON_AddItemToObject (object, "priv_level", cJSON_CreateNumber (user->privLevel));

        cJSON_AddItemToArray(json, object);
    }

    char *str = cJSON_Print(json);
    cJSON_Delete(json);

    FILE *file = fopen ("privUsers.json", "w");

    if (!file)
    {
        fputs ("Failed to open privUsers.json!", stderr);
        return;
    }

    fwrite(str, strlen(str), 1, file);

    fclose (file);
    free (str);

    return;
}

void saveFilters(Filter **filters, unsigned filterCount) {
    cJSON *json = cJSON_CreateArray();
    for (int i = 0; i < filterCount; i++) {
        Filter *filter = filters[i];
        cJSON *object = cJSON_CreateObject();

        cJSON_AddItemToObject(object, "description", cJSON_CreateString(filter->desc));
        cJSON_AddItemToObject(object, "expression", cJSON_CreateString(filter->filter));
        cJSON_AddItemToObject(object, "type", cJSON_CreateNumber(filter->type));
        cJSON_AddItemToObject(object, "truePositives", cJSON_CreateNumber(filter->truePositives));
        cJSON_AddItemToObject(object, "falsePositives", cJSON_CreateNumber(filter->falsePositives));
        cJSON_AddItemToObject(object, "isDisabled", cJSON_CreateNumber (filter->isDisabled));

        cJSON_AddItemToArray(json, object);
    }


    char *str = cJSON_Print(json);
    cJSON_Delete(json);

    FILE *file = fopen("filters.json", "w");
    if (!file) {
        fputs("Failed to open filter file!", stderr);
        return;
    }

    fwrite(str, strlen(str), 1, file);

    fclose(file);

    free(str);
}

cJSON *loadReports() {
    puts("Loading recent reports...");
    FILE *file = fopen("reports.json", "r");

    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buf = malloc(size+1);
    fread(buf, size, 1, file);
    buf[size] = 0;

    fclose(file);

    cJSON *json = cJSON_Parse(buf);
    free(buf);

    return json;
}

void saveReports(Report *reports[], int reportsUntilAnalysis) {
    cJSON *container = cJSON_CreateObject();
    cJSON *json = cJSON_CreateArray();

    for (int i = 0; i < REPORT_MEMORY; i++) {
        if (reports[i] == NULL) {
            cJSON_AddItemToArray(json, cJSON_CreateNull());
            continue;
        }
        cJSON *item = cJSON_CreateObject();
        cJSON_AddItemToObject(item, "messageID", cJSON_CreateNumber(reports[i]->messageID));
        cJSON_AddItemToObject(item, "confirmation", cJSON_CreateNumber(reports[i]->confirmation));
        cJSON_AddItemToObject (item, "likelihood", cJSON_CreateNumber (reports [i]->likelihood));
        cJSON_AddItemToObject (item, "body_length", cJSON_CreateNumber (reports [i]->bodyLength));

        Post *post = reports[i]->post;
        cJSON_AddItemToObject(item, "postID", cJSON_CreateNumber(post->postID));
        cJSON_AddItemToObject(item, "title", cJSON_CreateString(post->title));
        cJSON_AddItemToObject(item, "body", cJSON_CreateString(post->body));
        cJSON_AddItemToObject(item, "userID", cJSON_CreateNumber(post->owner->userID));
        cJSON_AddItemToObject (item, "userRep", cJSON_CreateNumber (post->owner->userRep));
        cJSON_AddItemToObject(item, "username", cJSON_CreateString(post->owner->username));
        cJSON_AddItemToObject(item, "isAnswer", cJSON_CreateBool(post->isAnswer));

        cJSON_AddItemToArray(json, item);
    }

    cJSON_AddItemToObject(container, "latestReports", json);
    cJSON_AddItemToObject(container, "reportsUntilAnalysis", cJSON_CreateNumber(reportsUntilAnalysis));

    char *str = cJSON_Print(container);
    cJSON_Delete(container);

    FILE *file = fopen("reports.json", "w");
    if (!file) {
        fputs("Failed to open reports file!", stderr);
        return;
    }

    fwrite(str, strlen(str), 1, file);

    fclose(file);
    free(str);
}
