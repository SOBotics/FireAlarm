//
//  ChatBot.c
//  chatbot
//
//  Created on 5/5/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zlib.h>
#include <ctype.h>

#include "ChatBot.h"
#include "ChatMessage.h"
#include "cJSON.h"
#include "misc_functions.h"
#include "Client.h"
#include "Filter.h"
//#include "Logs.h"

#define REPORT_HEADER "Potentially bad question"
//#define THRESHOLD 1000

long THRESHOLD = 1000;

static void loadNullReports(Report **reports) {
    for (int i = 0; i < REPORT_MEMORY; i++) {
        reports[i] = NULL;
    }
}

typedef struct {
    char *text;
    unsigned trueOccurences;
    unsigned falseOccurences;
}Word;

void analyzeReports(ChatBot *bot) {
    puts("Analyzing reports...");
    Report **reports = bot->latestReports;
    //First, string together all of the true and false positive posts.
    size_t trueLen = 0;
    size_t falseLen = 0;
    char *truePositives = NULL;
    char *falsePositives = NULL;
    for (int i = 0; i < REPORT_MEMORY; i++) {
        Report *report = reports[i];
        if (!report || report->confirmation == -1) {
            continue;
        }
        const char *body = report->post->body;
        if (report->confirmation) {
            trueLen += strlen(body) + 2;
            if (truePositives) {
                truePositives = realloc(truePositives, trueLen);
            }
            else {
                truePositives = calloc(1, trueLen);   //calloc zero-fills
            }
            strcat(truePositives, " ");
            strcat(truePositives, body);
        }
        else {
            falseLen += strlen(body) + 2;
            if (falsePositives) {
                falsePositives = realloc(falsePositives, falseLen);
            }
            else {
                falsePositives = calloc(1, falseLen);   //calloc zero-fills
            }
            strcat(falsePositives, " ");
            strcat(falsePositives, body);
        }
    }

    //Now, count word frequency.
    Word *words = NULL;
    size_t wordCount = 0;
    unsigned char inWord = 0;

    const unsigned maxWordLength = 256;
    char currentWord[maxWordLength];
    size_t currentWordLen = 0;

    char *pos = truePositives;
    for (signed char searchingTruePositives = 1; searchingTruePositives > -1; searchingTruePositives--) {
        while (*pos) {
            if (inWord) {
                if (isalpha(*pos)) {
                    currentWord[currentWordLen++] = tolower(*pos);
                }
                else {  //we're at the end of a word!
                    currentWord[currentWordLen] = 0;
                    unsigned char foundWord = 0;
                    //check if this word is in the list
                    for (int i = 0; i < wordCount; i++) {
                        if (!strcmp(words[i].text, currentWord)) {
                            //this word is in the list.
                            //increment it's count
                            if (searchingTruePositives) {
                                words[i].trueOccurences++;
                            }
                            else {
                                words[i].falseOccurences++;
                            }
                            foundWord = 1;
                        }
                    }
                    if (!foundWord) {
                        words = realloc(words, ++wordCount * sizeof(Word));
                        words[wordCount-1].text = malloc(currentWordLen + 1);
                        strcpy(words[wordCount-1].text, currentWord);
                        words[wordCount-1].trueOccurences = searchingTruePositives;
                        words[wordCount-1].falseOccurences = !searchingTruePositives;
                    }
                    currentWordLen = 0;
                    inWord = 0;
                }
            }
            else {
                switch (*pos) {
                    case '<':   //skip HTML tags
                        if (strstr(pos, "<pre><code>") == pos) {
                            pos = strstr(pos, "</code></pre>");
                            break;  //Completely skip over code.
                        }
                        pos = strchr(pos, '>');
                        break;
                    default:
                        if (isalpha(*pos)) {
                            inWord = 1;
                            currentWordLen = 1;
                            currentWord[0] = tolower(*pos);
                        }
                        break;
                }
            }
            pos++;
        }
        pos = falsePositives;
    }
    free(truePositives);
    free(falsePositives);

    puts("Filter additions:");
    puts("            Word    TP rate      TPs     FPs\n");
    for (unsigned i = 0; i < wordCount; i++) {
        Word word = words[i];
        unsigned trueOccurences = word.trueOccurences;
        unsigned falseOccurences = word.falseOccurences;
        float totalOccurences = trueOccurences + falseOccurences;
        float trueRate = trueOccurences / totalOccurences;
        const char *text = word.text;
        if (totalOccurences > 5 && trueRate > 0.7) {
            unsigned char matchesExistingFilter = 0;
            for (int i = 0; i < bot->filterCount; i++) {
                if (strstr(bot->filters[i]->filter, text)) {
                    matchesExistingFilter = 1;
                    break;
                }
            }
            if (!matchesExistingFilter && strlen(text) > 2) {
                Filter *newFilter = createFilter(text, text, 0, trueOccurences, falseOccurences, 0);
                bot->filters = realloc(bot->filters, ++bot->filterCount * sizeof(Filter*));
                bot->filters[bot->filterCount - 1] = newFilter;
                printf("%16s\t%4f\t%4d\t%4d\n", text, trueRate, trueOccurences, falseOccurences);
            }
        }
    }

    /*//Now coming to analyzing the tag filter
    char **tagsCaught;
    int j = 0;
    unsigned trueOccurences = 0;
    unsigned falseOccurences = 0;
    unsigned totalOccurences = 0;
    int i;
    for (i = 0; i < bot->filterCount; i ++)
    {
        if (bot->filters [i]->type == 3)
        {
            tagsCaught [j] = bot->filters [i]->filter;
            j++;
        }
    }

    //Now looking for patterns of more bad tags which can be added to the list
    j = 0;
    int k = 0;
    int totalNewTags = 0;
    char **newTags;
    int tps [50];
    int fps [50];
    int tpRate [50];
    for (i = 0; i < REPORT_MEMORY; i ++)
    {
        Report *report = reports [i];

        char **postTags = getTagsByID (bot, report->post->postID);

        for (j = 0; j < 5; j ++)
        {
            char *currentTag = postTags [i];

            if (!isTagProgrammingRelated (currentTag) && !isTagInFilter (bot, currentTag))
            {
                for (k = 0; k < REPORT_MEMORY; k ++)
                {
                    Report *currentReport = reports [k];

                    if (currentReport->messageID != report->messageID)
                    {
                        char **currentPostTags = getTagsByID (bot, currentReport->post->postID);

                        if (postHasTags (bot, currentReport->post, currentTag))
                        {
                            if (currentReport->confirmation)
                            {
                                trueOccurences ++;
                            }
                            else if (!currentReport->confirmation)
                            {
                                falseOccurences ++;
                            }
                        }
                    }
                }
            }

            if ((trueOccurences - falseOccurences) >= 15)
            {
                trueOccurences = 0;
                falseOccurences = 0;
                newTags [totalNewTags] = currentTag;
                tps [totalNewTags] = trueOccurences;
                fps [totalNewTags] = falseOccurences;
                tpRate [totalNewTags] = trueOccurences / falseOccurences;
                totalNewTags ++;
            }
            if (trueOccurences > 0 || falseOccurences > 0)
            {
                trueOccurences = 0;
                falseOccurences = 0;
            }
        }
    }

    Filter **filters = bot->filters;
    char *desc = malloc (sizeof (char) * 50);
    puts("            Tags    TP rate      TPs     FPs\n");
    for (i = 0; i < totalNewTags; i ++)
    {
        sprintf (desc, "[tag:%s]", newTags [i]);

        filters = realloc (filters, ++bot->filterCount * sizeof (Filter *));
        filters [bot->filterCount - 1] = createFilter (desc, newTags [i], 3, tps [i], fps [i]);

        printf("%16s\t%4f\t%4d\t%4d\n", newTags [i], tpRate [i], tps [i], fps [i]);
    }

    free (desc);*/
    return;
}

static Report **parseReports(ChatBot *bot, cJSON *json) {
    Report **reports = malloc(REPORT_MEMORY * sizeof(reports));
    bot->reportsUntilAnalysis = REPORT_MEMORY;
    if (json == NULL) {
        loadNullReports(reports);
        return reports;
    }

    cJSON *array = cJSON_GetObjectItem(json, "latestReports");

    if (cJSON_GetArraySize(array) != REPORT_MEMORY) {
        fputs("Report file doesn't have enough reports!  Ignoring report file.\n", stderr);
        loadNullReports(reports);
        cJSON_Delete(json);
        return reports;
    }

    bot->reportsUntilAnalysis = cJSON_GetObjectItem(json, "reportsUntilAnalysis")->valueint;

    for (int i = 0; i < REPORT_MEMORY; i++) {
        cJSON *data = cJSON_GetArrayItem(array, i);
        if (data->type == cJSON_NULL) {
            reports[i] = NULL;
            continue;
        }

        unsigned long messageID = cJSON_GetObjectItem(data, "messageID")->valueint;
        unsigned long postID = cJSON_GetObjectItem(data, "postID")->valueint;
        unsigned long userID = cJSON_GetObjectItem(data, "userID")->valueint;
        unsigned char isAnswer = cJSON_GetObjectItem(data, "isAnswer")->type == cJSON_True;
        int confirmation = cJSON_GetObjectItem(data, "confirmation")->valueint;
        const char *title = cJSON_GetObjectItem(data, "title")->valuestring;
        const char *body = cJSON_GetObjectItem(data, "body")->valuestring;

        Report *report = malloc(sizeof(Report));

        report->messageID = messageID;
        report->post = createPost(title, body, postID, isAnswer, userID);
        report->confirmation = confirmation;;

        reports[i] = report;
    }

    cJSON_Delete(json);
    return reports;
}

ChatBot *createChatBot(
                       ChatRoom *room,
                       ChatRoom *roomPostTrue,
                       Command **commands,
                       cJSON *latestReports,
                       Filter **filters,
                       PrivUser **users,
                       PrivRequest **requests,
                       Modes *modes,
                       Notify **notify,
                       Log **logs
                       ) {
    ChatBot *c = malloc(sizeof(ChatBot));
    c->room = room;
    c->roomPostTrue = roomPostTrue;
    c->commands = commands;
    c->runningCommands = NULL;
    c->apiFilter = NULL;
    c->runningCommandCount = 0;
    c->modes = modes;
    c->stopAction = ACTION_NONE;
    pthread_mutex_init(&c->runningCommandsLock, NULL);
    pthread_mutex_init(&c->detectorLock, NULL);

    c->filters = NULL;
    c->filterCount = 0;
    c->numOfPrivUsers = 0;
    c->privUsers = NULL;
    c->privRequests = NULL;
    c->totalPrivRequests = 0;
    c->notify = NULL;
    c->totalNotifications = 0;
    c->log = NULL;
    c->totalLogs = 0;

    c->reportsWaiting = -1;

    while (*(filters++)) {
        c->filters = realloc(c->filters, ++c->filterCount * sizeof(Filter*));
        c->filters[c->filterCount-1] = *(filters - 1);
    }

    while (*(notify++))
    {
        c->notify = realloc (c->notify, ++c->totalNotifications * sizeof (Notify*));
        c->notify [c->totalNotifications - 1] = *(notify - 1);
    }

    while (*(requests++))
    {
        c->privRequests = realloc (c->privRequests, ++c->totalPrivRequests * sizeof (PrivRequest*));
        c->privRequests[c->totalPrivRequests-1] = *(requests - 1);
    }

    while (*(users++)) {
        c->privUsers = realloc(c->privUsers, ++c->numOfPrivUsers * sizeof(PrivUser*));
        c->privUsers[c->numOfPrivUsers-1] = *(users - 1);
    }

    while (*(logs++)) {
        c->log = realloc(c->log, ++c->totalLogs * sizeof(Log*));
        c->log[c->totalLogs-1] = *(logs - 1);
    }

    Report **reports = parseReports(c, latestReports);
    for (int i = 0; i < REPORT_MEMORY; i++) {
        c->latestReports[i] = reports[i];
    }
    free(reports);

    return c;
}

void runCommand(ChatBot *bot, ChatMessage *message, char *command) {
    //Get the space-separated components of this command.
    char **components = NULL;
    size_t componentCount = 0;

    char *component;
    while ((component = strsep(&command, " "))) {
        //add command to components
        components = realloc(components, (++componentCount) * sizeof(char*));
        components[componentCount-1] = malloc(strlen(component) + 1);
        strcpy(components[componentCount-1], component);
    };
    pthread_mutex_lock(&bot->runningCommandsLock);
    RunningCommand *c = launchCommand(message, (int)componentCount, components, bot->commands, bot);
    bot->runningCommands = realloc(bot->runningCommands, ++bot->runningCommandCount * sizeof(RunningCommand *));
    bot->runningCommands[bot->runningCommandCount-1] = c;
    pthread_mutex_unlock(&bot->runningCommandsLock);
}

void prepareCommand(ChatBot *bot, ChatMessage *message, char *messageText) {
    char *command = strchr(messageText, ' ');
    if (command) {
        while (*(++command) == ' ');
        if (*command && bot->stopAction == ACTION_NONE) {
            runCommand(bot, message, command);
            return;
        }
    }
}

Report *reportWithMessage(ChatBot *bot, unsigned long messageID) {
    for (int i = 0; i < REPORT_MEMORY; i++) {
        if (bot->latestReports[i]) {
            if (messageID == bot->latestReports[i]->messageID) {
                return bot->latestReports[i];
            }
        }
    }
    return NULL;
}

void processMessage(ChatBot *bot, ChatMessage *message) {
    char *messageText = malloc(strlen(message->content) + 1);
    strcpy(messageText, message->content);
    if (strcasestr(messageText, "@fir") == messageText) {
        //detects message containg @Fir
        prepareCommand(bot, message, messageText);

    }
    else if (bot->reportsWaiting != -1 && strstr(messageText, REPORT_HEADER)) {
        bot->latestReports[bot->reportsWaiting--]->messageID = message->id;
        deleteChatMessage(message);
    }
    else if (message->replyID && reportWithMessage(bot, message->replyID)) {
        prepareCommand(bot, message, messageText);
    }
    else {
        deleteChatMessage(message);
    }
    free(messageText);
}

Post *getPostByID(ChatBot *bot, unsigned long postID) {
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;

    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));

    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

    if (bot->apiFilter == NULL) {
        checkCURL(curl_easy_setopt(curl, CURLOPT_URL,
                                   "api.stackexchange.com/2.2/filters/create"
                                   "?include=post.title;post.body;question.tags;user.user_id;question.closed_reason&unsafe=false&key="API_KEY
                                   ));
        checkCURL(curl_easy_perform(curl));

        cJSON *json = cJSON_Parse(buffer.data);
        free(buffer.data);
        buffer.data = NULL;

        cJSON *items = cJSON_GetObjectItem(json, "items");
        char *filter = cJSON_GetObjectItem(cJSON_GetArrayItem(items, 0), "filter")->valuestring;
        bot->apiFilter = malloc(strlen(filter) + 1);
        strcpy(bot->apiFilter, filter);
        cJSON_Delete(json);
    }



    unsigned max = 256;
    char request[max];
    snprintf(request, max,
             "https://api.stackexchange.com/posts/%lu?site=stackoverflow&filter=%s&key="API_KEY,
             postID, bot->apiFilter
             );
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
        //sleep (backoff->valueint);
        free(str);
        //return getPostByID (bot, postID);
    }

    cJSON *postJSON = cJSON_GetArrayItem(cJSON_GetObjectItem(json, "items"), 0);
    if (postJSON == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    //puts(cJSON_Print(postJSON));


    char *title = cJSON_GetObjectItem(postJSON, "title")->valuestring;
    char *body = cJSON_GetObjectItem(postJSON, "body")->valuestring;
    char *type = cJSON_GetObjectItem(postJSON, "post_type")->valuestring;
    unsigned userID = cJSON_GetObjectItem(cJSON_GetObjectItem(postJSON, "owner"), "user_id")->valueint;

    Post *p = createPost(title, body, postID, strcmp(type, "answer") == 0, userID);

    cJSON_Delete(json);
    return p;
}

unsigned int checkPost(ChatBot *bot, Post *post) {

    if (!bot->modes->reportMode)
    {
        return 0;
    }
    if (post == NULL)
    {
        puts ("\nNULL post!\n");
        return 0;
    }
    /*else if (post->body == NULL)
    {
        puts ("Post->body is NULL!");
        return 0;
    }*/
    //else if (post->isAnswer == 1)
   /* {
        puts ("Checking answer :p\n");
        return 0;
    }*/
    //printf ("Checking post: %lu\n", post->postID);

    unsigned likelihood = 0;
    unsigned bodyLength = 1;
    char *messageBuf = malloc(sizeof(char));
    *messageBuf = 0;




        for (int i = 0; i < bot->filterCount; i++) {
            unsigned start, end;
            if (postMatchesFilter(bot, post, bot->filters[i], &start, &end)) {

                const char *desc = bot->filters[i]->desc;
                messageBuf = realloc(messageBuf, strlen(messageBuf) + strlen(desc) + 16);

                snprintf(messageBuf + strlen(messageBuf), strlen(desc) + 16,
                         "%s%s", (likelihood ? ", " : ""), desc);
                //If this not the first match, start it with a comma and space.

                const float truePositives = bot->filters[i]->truePositives;
                likelihood += (truePositives / (truePositives + bot->filters[i]->falsePositives)) * 1000;
                //likelihood = 1001;
            }
        }


    if (likelihood > THRESHOLD && (recentlyReported (post->postID, bot) == 0)) {
        //puts ("Preparing report...\n");
        char *notifString = getNotificationString(bot, post);
        //puts ("Completed line 576.");
        const size_t maxMessage = strlen(messageBuf) + strlen(post->title) + strlen(REPORT_HEADER) + strlen (notifString) + 256;
        //puts ("Completed line 578.");
        char *message = malloc(maxMessage);
       // puts ("Completed line 580.");
        //char *notif = getNotificationString(bot, post);
        //puts ("Completed line 582.");
        snprintf(message, maxMessage,
                 REPORT_HEADER " (%s): [%s](http://stackoverflow.com/%s/%lu) (likelihood %d) %s",
                 messageBuf, post->title, post->isAnswer ? "a" : "q", post->postID, likelihood, notifString);
        //puts ("Completed preparing report.");
        //free(notif);
        //if (notifString != NULL)
            //free (notifString);
        //puts ("Posting report...");
        postMessage(bot->room, message);
            //puts ("Posted report.");

        if (bot->latestReports[REPORT_MEMORY-1]) {
            free(bot->latestReports[REPORT_MEMORY-1]->post);
            free(bot->latestReports[REPORT_MEMORY-1]);
        }
        int i = REPORT_MEMORY;
        while(--i) {
            bot->latestReports[i] = bot->latestReports[i-1];
        }
        Report *report = malloc(sizeof(Report));
        report->post = post;
        report->confirmation = -1;
        report->likelihood = likelihood;
        bot->latestReports[0] = report;
        bot->reportsWaiting++;
        bot->reportsUntilAnalysis--;
        if (bot->reportsUntilAnalysis == 0) {
            bot->reportsUntilAnalysis = REPORT_MEMORY;
            analyzeReports(bot);
        }
        free(message);
        free (messageBuf);
        return 0;
    }
    else {
        deletePost(post);
        free (messageBuf);
        return 1;
    }
    free (messageBuf);
}

Post **getPosts (ChatBot *bot, int *total)
{
    pthread_mutex_lock(&bot->room->clientLock);
    CURL *curl = bot->room->client->curl;

    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));

    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

    if (bot->apiFilter == NULL) {
        checkCURL(curl_easy_setopt(curl, CURLOPT_URL,
                                   "api.stackexchange.com/2.2/filters/create"
                                   "?include=post.title;post.body;question.tags;user.user_id;question.closed_reason;user.display_name&unsafe=false&key="API_KEY
                                   ));
        checkCURL(curl_easy_perform(curl));

        cJSON *JSON = cJSON_Parse(buffer.data);
        free(buffer.data);
        buffer.data = NULL;

        cJSON *items = cJSON_GetObjectItem(JSON, "items");
        char *filter = cJSON_GetObjectItem(cJSON_GetArrayItem(items, 0), "filter")->valuestring;
        bot->apiFilter = malloc(strlen(filter) + 1);
        strcpy(bot->apiFilter, filter);
        cJSON_Delete(JSON);
    }

    unsigned max = 256;
    char request[max];
    snprintf(request, max,
             "https://api.stackexchange.com/2.2/posts?pagesize=50&order=desc&sort=activity&filter=%s&site=stackoverflow&key="API_KEY,
             bot->apiFilter
             );
    curl_easy_setopt(curl, CURLOPT_URL, request);



    checkCURL(curl_easy_perform(curl));

    checkCURL(curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""));


    pthread_mutex_unlock(&bot->room->clientLock);

    cJSON *json = cJSON_Parse(buffer.data);

    free(buffer.data);
    //puts (cJSON_Print (json));

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
        sleep (backoff->valueint + 5);
        free(str);
    }

    cJSON *item = cJSON_GetObjectItem (json, "items");
    /*if (postsJSON == NULL)
    {
        printf ("postJSON is NULL!");
        cJSON_Delete (json);
        return NULL;
    }*/

    unsigned totalPosts = cJSON_GetArraySize(item);
    //unsigned totalPosts = 10;
    Post **posts = malloc(sizeof(Post*) * (50 + 1));
    posts = NULL;
    unsigned i;

    for (i = 0; i < 49; i ++)
    {
        //cJSON *postJSON = cJSON_GetArrayItem (postsJSON, i);
        /*char *title = cJSON_GetObjectItem(cJSON_GetArrayItem (item, i), "title")->valuestring;
        char *body = cJSON_GetObjectItem(cJSON_GetArrayItem (item, i), "body")->valuestring;
       // printf ("'body' is %s\n", body);
        char *type = cJSON_GetObjectItem(cJSON_GetArrayItem (item, i), "post_type")->valuestring;
        //unsigned userID = cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem (item, i), "owner"), "user_id")->valueint;
        */
        cJSON *current = cJSON_GetArrayItem (item, i);
        if (current == NULL)
        {
            total = totalPosts;

    cJSON_Delete (json);
    cJSON_Delete (item);

    return posts;
        }
        unsigned long postID = cJSON_GetObjectItem (cJSON_GetArrayItem (item, i), "post_id")->valueint;
        totalPosts = i;
        /*cJSON *current = cJSON_GetArrayItem (item, i);
        unsigned userID = cJSON_GetObjectItem (cJSON_GetObjectItem (current, "owner"), "user_id")->valueint;

        posts [i] = createPost(title, body, postID, strcmp(type, "answer") == 0, userID);*/
        posts [i] = getPostByID (bot, postID);
        if (posts [i] == NULL)
        {
            total = totalPosts;

            cJSON_Delete (json);
            cJSON_Delete (item);

            return posts;
        }
    }
    total = totalPosts;

    cJSON_Delete (json);
    cJSON_Delete (item);

    return posts;
}

void confirmPost(ChatBot *bot, Post *post, unsigned char confirmed) {
    for (int i = 0; i < bot->filterCount; i++) {
        Filter *filter = bot->filters[i];
        if (postMatchesFilter(bot, post, filter, NULL, NULL)) {
            if (confirmed) {
                filter->truePositives++;
            }
            else {
                filter->falsePositives++;
            }
            printf("Increased %s positive count of %s.\n", confirmed ? "true" : "false", filter->desc);
        }
    }
}

StopAction runChatBot(ChatBot *c) {
    ChatMessage **messages = processChatRoomEvents(c->room);
    if (messages != NULL) {
        ChatMessage *message;
        for (int i = 0; (message = messages[i]); i++) {
            processMessage(c, message);
        }
        free(messages);
    }

    //clean up old commands
    for (int i = 0; i < c->runningCommandCount; i++) {
        if (c->runningCommands[i]->finished) {
            //delete the command
            c->runningCommandCount--;
            int j = i;
            for (deleteRunningCommand(c->runningCommands[j]); j < c->runningCommandCount; j++) {
                c->runningCommands[i] = c->runningCommands[i+1];
            }
            c->runningCommands = realloc(c->runningCommands, c->runningCommandCount * sizeof(RunningCommand*));
        }
    }
    if (c->stopAction != ACTION_NONE) {
        if (c->room->pendingMessageLinkedList == NULL && (c->runningCommandCount == 0) && c->reportsWaiting == -1) {
            return c->stopAction;
        }
    }

    return ACTION_NONE;
}

void testPost (ChatBot *bot, Post *post, RunningCommand *command)
{
    unsigned int likelihood = 0;
    char *messageBuf = malloc (sizeof (char));

    *messageBuf = 0;

    for (int i = 0; i < bot->filterCount; i++) {
        unsigned start, end;
        if (postMatchesFilter(bot, post, bot->filters[i], &start, &end)) {

            const char *desc = bot->filters[i]->desc;
            messageBuf = realloc(messageBuf, strlen(messageBuf) + strlen(desc) + 16);

            snprintf(messageBuf + strlen(messageBuf), strlen(desc) + 16,
                     "%s%s", (likelihood ? ", " : ""), desc);
            //If this not the first match, start it with a comma and space.

            const float truePositives = bot->filters[i]->truePositives;
            likelihood += (truePositives / (truePositives + bot->filters[i]->falsePositives)) * 1000;
        }
    }

    if (likelihood >= THRESHOLD)
    {
        const size_t maxMessage = strlen(messageBuf) + 256;
        char *message = malloc (maxMessage);

        sprintf (message, "Yes, that post crosses the threshold which currently is %ldd. The post's likelihood is %d.",
                 THRESHOLD, likelihood);

        postReply (bot->room, message, command->message);

        free (message);
    }
    else if (likelihood < THRESHOLD)
    {
        const size_t maxMessage = strlen (messageBuf) + 256;
        char *message = malloc (maxMessage);

        sprintf (message, "No, that post doesn't cross the threshold which currently is %ld. The post's likelihood is %d",
                 THRESHOLD, likelihood);

        postReply (bot->room, message, command->message);

        free (message);
    }

    free (messageBuf);

    return;
}

int recentlyReported (long postID, ChatBot *bot)
{
    Report **reports = bot->latestReports;

    for (unsigned int i = 0; i < REPORT_MEMORY; ++ i)
    {
        if (reports[i] == NULL) {
            continue;
        }

        Post *post = reports [i]->post;

        if (post->postID == postID)
        {
            return 1;
        }
    }

    return 0;
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

int isValidUserID (ChatBot *bot, long userID)
{
    if (isUserInPingableList (bot, userID))
    {
        return 1;
    }
    else if (!getUsernameByID (bot, userID) || getUsernameByID (bot, userID) == NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

char *getUsernameByID (ChatBot *bot, unsigned long userID)
{
    /*if (isUserInPingableList (bot, userID))
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
    else if (!isUserInPingableList(bot, userID))
    {*/
        pthread_mutex_lock(&bot->room->clientLock);
        CURL *curl = bot->room->client->curl;

        checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
        OutBuffer buffer;
        buffer.data = NULL;
        checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));

        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

        unsigned max = 256;
        char request [max];

        snprintf (request, max,
                  "http://api.stackexchange.com/2.2/users/%lu?order=desc&sort=reputation&site=stackoverflow&key="API_KEY, userID);

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

        cJSON *userJSON = cJSON_GetArrayItem(cJSON_GetObjectItem(json, "items"), 0);
        if (userJSON == NULL) {
            cJSON_Delete(json);
            return NULL;
        }

        char *username = cJSON_GetObjectItem (userJSON, "display_name")->valuestring;

        cJSON_Delete (json);

        if (username == NULL)
        {
            return NULL;
        }
        return username;

}

char **getTagsByID (ChatBot *bot, unsigned long postID)
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

    snprintf (request, max,
              "https://api.stackexchange.com/2.2/questions/%lu?order=desc&sort=activity&site=stackoverflow&filter=default",
              postID);

    printf ("API request is %s\n", request);

    curl_easy_setopt(curl, CURLOPT_URL, request);

    checkCURL(curl_easy_perform(curl));

    checkCURL(curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""));


    pthread_mutex_unlock(&bot->room->clientLock);

    cJSON *json = cJSON_Parse(buffer.data);
    puts (buffer.data);

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
    cJSON *tagJSON = cJSON_GetArrayItem (json, "tags");
    //unsigned size = cJSON_GetArraySize (tagJSON);
    unsigned size = 3;
    char **tags;
    for (unsigned i = 0; i < size; i ++)
    {
        tags [i] = malloc (sizeof (char) * 40);
        //tags [i] = cJSON_GetObjectItem (tagJSON, i)->valuestring;
        strcpy (tags [i], cJSON_GetObjectItem (tagJSON, i)->valuestring);
    }

    //char **tags = cJSON_GetArrayItem (json, "tags");

    //char **tags = cJSON_GetObjectItem (json, "tags")->valuestring;
    //strcpy (tags [0], "java");

    //cJSON_Delete (json);
    return tags;
}

Filter *getFilterByTag (ChatBot *bot, char *tag)
{
    if (!isTagInFilter (bot, tag))
    {
        return NULL;
    }

    Filter **filters = bot->filters;

    for (int i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == 3)
        {
            if (strcmp (filter->filter, tag) == 0)
            {
                return filter;
            }
        }
    }

    return NULL;
}

Filter **getTagsCaughtInPost (ChatBot *bot, Post *post)
{
    Filter **filters = bot->filters;
    char **tags = getTagsByID (bot, post->postID);
    char **tagsCaught;

    int k = 0;
    int i;
    for (i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == 3)
        {
            for (int j = 0; j < 5; j ++)
            {
                if (strcmp (tags [j], filter->filter) == 0)
                {
                    tagsCaught [k] = filter->filter;
                    k ++;
                }
            }
        }
    }

    Filter **filtersCaught;

    for (i = 0; i < 5; i ++)
    {
        filtersCaught [i] = getFilterByTag (bot, tagsCaught [i]);
    }

    return filtersCaught;
}

void editFilter (ChatBot *bot, Post *post, int confirm)
{
    Filter **filtersCaught = getTagsCaughtInPost (bot, post);

    for (int i = 0; i < 5; i ++)
    {
        Filter *filter = filtersCaught [i];

        if (confirm)
        {
            filter->truePositives ++;
        }
        else if (!confirm)
        {
            filter->falsePositives ++;
        }
    }

    return;
}

int isValidTag (ChatBot *bot, char *tag)
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

    snprintf (request, max,
              "https://api.stackexchange.com/2.2/tags/%s/info?order=desc&sort=popular&site=stackoverflow&filter=default",
              tag);

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

    /*cJSON *tagJSON = cJSON_GetArrayItem(cJSON_GetObjectItem(json, "items"), 0);
    if (tagJSON == NULL) {
        cJSON_Delete(json);
        return 0;
    }*/

    cJSON_Delete (json);
    return 1;
}

unsigned isKeywordInFilter (ChatBot *bot, char *keyword)
{
    Filter **filters = bot->filters;
    int i;

    for (i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == 0)
        {
            if (strcmp (filter->filter, keyword) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

Filter *getFilterByKeyword (ChatBot *bot, char *keyword)
{
    Filter **filters = bot->filters;
    int i;

    for (i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == 0)
        {
            if (strcmp (filter->filter, keyword) == 0)
            {
                return filter;
            }
        }
    }

    return NULL;
}

/*unsigned isTagInFilter (ChatBot *bot, char *tag)
{
    Filter **filters = bot->filters;
    int i;

    for (i = 0; i < bot->filterCount; i ++)
    {
        Filter *filter = filters [i];

        if (filter->type == 3)
        {
            if (strcmp (filter->filter, tag) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}*/

Report **getReportsByFilter (ChatBot *bot, unsigned filterType, unsigned totalReports)
{
    Report **reports = bot->latestReports;
    Filter **filters = bot->filters;
    Report **reportsDetected;
    unsigned i, j, k = 0;

    for (i = 0; i < REPORT_MEMORY; i ++)
    {
        Post *post = reports [i]->post;

        for (j = 0; j < bot->filterCount; j ++)
        {
            if (postMatchesFilter (bot, post, filters [j], NULL, NULL) && filters [i]->type == filterType)
            {
                reportsDetected = realloc (reportsDetected, ++k * sizeof (Report*));
                reportsDetected [k - 1] = reports [i];
            }
        }
    }

    return reportsDetected;
}

int apiQuota (ChatBot *bot)
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

    if (bot->apiFilter == NULL) {
        checkCURL(curl_easy_setopt(curl, CURLOPT_URL,
                                   "api.stackexchange.com/2.2/filters/create"
                                   "?include=post.title;post.body;question.tags;user.user_id;question.closed_reason;user.display_name&unsafe=false&key="API_KEY
                                   ));
        checkCURL(curl_easy_perform(curl));

        cJSON *json = cJSON_Parse(buffer.data);
        free(buffer.data);
        buffer.data = NULL;

        cJSON *items = cJSON_GetObjectItem(json, "items");
        char *filter = cJSON_GetObjectItem(cJSON_GetArrayItem(items, 0), "filter")->valuestring;
        bot->apiFilter = malloc(strlen(filter) + 1);
        strcpy(bot->apiFilter, filter);
        cJSON_Delete(json);
    }

    snprintf (request, max,
              "api.stackexchange.com/2.2/info?site=stackoverflow&filter=%s&key="API_KEY,
              bot->apiFilter);

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
        return -1;
    }

    cJSON *backoff;
    if ((backoff = cJSON_GetObjectItem(json, "backoff"))) {
        char *str;
        asprintf(&str, "Recieved backoff: %d", backoff->valueint);
        postMessage(bot->room, str);
        free(str);
    }

    int apiQuota = cJSON_GetObjectItem (json, "quota_remaining")->valueint;
    cJSON_Delete (json);

    return apiQuota;
}
