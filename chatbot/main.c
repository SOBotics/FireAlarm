//
//  main.c
//  chatbot
//
//  Created on 4/29/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <limits.h>

#include "Client.h"
#include "ChatRoom.h"
#include "ChatBot.h"
#include "cJSON.h"
#include "commands.h"
#include "Filter.h"

#define SAVE_INTERVAL 60

void unrecognizedCommand(RunningCommand *c, void *ctx) {
    ChatBot *bot = ctx;
    char *str;
    asprintf(&str, "Unrecognized command \"%s\".", c->message->content);
    postReply(bot->room, str, c->message);
    free(str);
}

void webSocektOpened(WebSocket *ws) {
    puts("Websocket opened");
    sendDataOnWebsocket(ws->ws, "155-questions-active", 0);
}

void wsRecieved(WebSocket *ws, char *data, size_t len) {
    cJSON *json = cJSON_Parse(data);
    cJSON *post = cJSON_Parse(cJSON_GetObjectItem(json, "data")->valuestring);
    if (strcmp(cJSON_GetObjectItem(post, "apiSiteParameter")->valuestring, "stackoverflow")) {
        //if this isn't SO
        cJSON_Delete(json);
        cJSON_Delete(post);
        return;
    }
    ChatBot *bot = (ChatBot*)ws->user;
    Post *p = getPostByID(bot, cJSON_GetObjectItem(post, "id")->valueint);
    if (p != NULL) {
        checkPost(bot, p);
    }
    else {
        printf("Got a null post: %d\n", cJSON_GetObjectItem(post, "id")->valueint);
    }
    cJSON_Delete(json);
    cJSON_Delete(post);
}

Filter **loadFilters() {
    puts("Loading filters...");
    FILE *file = fopen("filters.json", "r");
    if (!file) {
        puts("Could not read from ~/.chatbot/filters.json.  Creating a skeleton filter list.");
        Filter **filters = malloc(sizeof(Filter*) * 2);
        filters[0] = createFilter(
                                  "Example filter",
                                  "Regular expression to test against ^",   //^ is so this doesn't match anything
                                  FILTER_REGEX,
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
        unsigned falsePositives = cJSON_GetObjectItem(filter, "falsePositives")->valueint;
        filters[i] = createFilter(desc, expr, type, truePositives, falsePositives);
    }
    filters[filterCount] = NULL;
    
    cJSON_Delete(json);
    
    return filters;
}

void wsClosed(WebSocket *socket) {
    postMessage(((ChatBot*)(socket->user))->room, "Websocket disconnected! Reboot the bot to reconnect websocket.");
}

void saveFilters(Filter **filters, unsigned filterCount) {
    puts("Saving filters...");
    cJSON *json = cJSON_CreateArray();
    for (int i = 0; i < filterCount; i++) {
        Filter *filter = filters[i];
        cJSON *object = cJSON_CreateObject();
        
        cJSON_AddItemToObject(object, "description", cJSON_CreateString(filter->desc));
        cJSON_AddItemToObject(object, "expression", cJSON_CreateString(filter->filter));
        cJSON_AddItemToObject(object, "type", cJSON_CreateNumber(filter->type));
        cJSON_AddItemToObject(object, "truePositives", cJSON_CreateNumber(filter->truePositives));
        cJSON_AddItemToObject(object, "falsePositives", cJSON_CreateNumber(filter->falsePositives));
        
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
    puts("Saving recent reports...");
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
        
        Post *post = reports[i]->post;
        cJSON_AddItemToObject(item, "postID", cJSON_CreateNumber(post->postID));
        cJSON_AddItemToObject(item, "title", cJSON_CreateString(post->title));
        cJSON_AddItemToObject(item, "body", cJSON_CreateString(post->body));
        cJSON_AddItemToObject(item, "userID", cJSON_CreateNumber(post->userID));
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

int main(int argc, const char * argv[]) {
    // insert code here...
    puts("Starting...");
#ifdef DEBUG
    puts("Debug mode is active.  Messages will not be posted to the chat room.");
#endif
    if (curl_global_init(CURL_GLOBAL_ALL)) {
        fputs("Error initializing libcurl!", stderr);
        exit(EXIT_FAILURE);
    }
    
    
    //Get the chatbot directory path.
    //http://stackoverflow.com/a/26696759/3476191
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    //Append the directory location.
    const char *dirName = "/.chatbot";
    char dirPath[strlen(homedir) + strlen(dirName) + 1];
    dirPath[0] = 0;
    strcpy(dirPath, homedir);
    strcat(dirPath, dirName);
    char resolvedPath[PATH_MAX];
    realpath(dirPath, resolvedPath);
    
    struct stat st;
    if (stat(resolvedPath, &st) == -1) {
        mkdir(resolvedPath, 0700);
    }
    
    chdir(resolvedPath);
    
    Client *client = createClient("stackoverflow.com", "cookies");
    if (!client->isLoggedIn) {
        char *env_email, *env_pass;
        if (
            (env_email = getenv("ChatBotEmail")) == NULL ||
            (env_pass = getenv("ChatBotPass")) == NULL
            ) {
            //Get the email from the user
            printf("Email address: ");
            const unsigned maxEmailLen = 128;
            char email[maxEmailLen];
            fgets(email, maxEmailLen, stdin);
            email[maxEmailLen-1] = 0;   //make sure it's null terminated
            
            char *password = getpass("Password: ");
            loginWithEmailAndPassword(client, email, password);
            //overwrite the password so it doesn't stay in memory
            memset(password, 0, strlen(password));
        }
        else {
            loginWithEmailAndPassword(client, env_email, env_pass);
            memset(env_pass, 0, strlen(env_pass));
        }
    }
    ChatRoom *room = createChatRoom(client, 68414);
    
    enterChatRoom(room);
    
    
    Filter **filters = loadFilters();
    
    Command *commands[] = {
        createCommand("I can put anything I want here; the first command runs when no other commands match", unrecognizedCommand),
        createCommand("test1", test1Callback),
        createCommand("test1 test2 ...", testVarCallback),
        createCommand("test1 * test3", testArgCallback),
        createCommand("test2 test3", test2Callback),
        createCommand("running commands", listCommands),
        createCommand("stop", stopBot),
        createCommand("reboot", rebootBot),
        createCommand("kill", forceStopBot),
        createCommand("check post *", checkPostCallback),
        createCommand("tp", truePositive),
        createCommand("fp", falsePositive),
        createCommand("t", truePositive),
        createCommand("f", falsePositive),
        createCommand("help", help),
        createCommand("alive", aliveCheck),
        createCommand("commands", commandList),
        createCommand("command", commandList),
        createCommand("tpr", truePositiveRespond),
        createCommand("fpr", falsePositiveRespond),
        createCommand("stats *", statistics),
        createCommand("recent reports *", printLatestReports),
        createCommand("latest reports *", printLatestReports),
        createCommand("post info *", postInfo),
        createCommand("tr", truePositiveRespond),
        createCommand("fr", falsePositiveRespond),
        createCommand("cp *", checkPostCallback),
        createCommand("pinfo *", postInfo),
        createCommand("change threshold *", changeThreshold),
        createCommand("check threshold *", checkThreshold),
        NULL
    };
    ChatBot *bot = createChatBot(room, commands, loadReports(), filters);
    
    
    WebSocket *socket = createWebSocketWithClient(client);
    socket->user = bot;
    socket->openCallback = webSocektOpened;
    socket->recieveCallback = wsRecieved;
    socket->closeCallback = wsClosed;
    connectWebSocket(socket, "qa.sockets.stackexchange.com", "/");
    
    puts("Started.");
    postMessage (bot->room, "[Fire Alarm](https://github.com/NobodyNada/chatbot) started.");
    
    unsigned char reboot = 0;
    time_t saveTime = time(NULL) + SAVE_INTERVAL;
    for (;;) {
        serviceWebsockets(client);
        StopAction action = runChatBot(bot);
        if (action == ACTION_STOP) {
            break;
        }
        else if (action == ACTION_REBOOT) {
            reboot = 1;
            break;
        }
        if (time(NULL) > saveTime) {
            saveFilters(bot->filters, bot->filterCount);
            saveReports(bot->latestReports, bot->reportsUntilAnalysis);
            saveTime = time(NULL) + SAVE_INTERVAL;
        }
    }
    
    leaveRoom(bot->room);
    
    saveFilters(bot->filters, bot->filterCount);
    saveReports(bot->latestReports, bot->reportsUntilAnalysis);
    
    
    
    curl_easy_cleanup(client->curl);
    
    if (reboot) {
        execv(argv[0], (char*const*)argv);  //Reload the program.
    }
    
    return 0;
}
