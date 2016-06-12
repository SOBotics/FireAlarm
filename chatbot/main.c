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
#include "Privileges.h"

#define SAVE_INTERVAL 60

void unrecognizedCommand(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    char *str;
    char *message;
    char messageString [256];
    char subString [127];
    
    asprintf (&str, "%s", command->message->content);
    asprintf(&message, "Unrecognized command `%s`.", command->message->content);
    sprintf (subString, "%s Did you want to type in", message);
    
    if (strcasestr (str, "to") == str || strcasestr (str, "tl") == str || strcasestr (str, "t[") == str || strcasestr (str, "t{") == str)
    {
        sprintf (messageString, "%s `tp`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "fo") == str || strcasestr (str, "fl") == str || strcasestr (str, "f[") == str || strcasestr (str, "f{") == str)
    {
        sprintf (messageString, "%s `fp`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "rp") == str || strcasestr (str, "yp") == str)
    {
        sprintf (messageString, "%s `tp`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "gp") == str || strcasestr (str, "dp") == str || strcasestr (str, "vp") == str)
    {
        sprintf (messageString, "%s `fp`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "che") == str || strcasestr (str, "ch ost") == str)
    {
        sprintf (messageString, "%s `check post`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "tes") == str || strcasestr (str, "te ost") == str)
    {
        sprintf (messageString, "%s `test post`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "run") == str || strcasestr (str, "ru and") == str)
    {
        sprintf (messageString, "%s `running commands`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "stat") == str || strcasestr (str, "sta") == str)
    {
        sprintf (messageString, "%s `stats`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "rec") == str || strcasestr (str, "lat") == str)
    {
        sprintf (messageString, "%s `recent reports` or `latest reports`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "stp") == str || strcasestr (str, "st p") == str)
    {
        sprintf (messageString, "%s `stop`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "ki") == str)
    {
        sprintf (messageString, "%s `kill`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "reb") == str || strcasestr (str, "re o") || strcasestr (str, "re t") == str)
    {
        sprintf (messageString, "%s `reboot`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "hel") == str || strcasestr (str, "hep") || strcasestr (str, "he p") == str)
    {
        sprintf (messageString, "%s `help`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "ali") == str)
    {
        sprintf (messageString, "%s `alive`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "com") == str)
    {
        sprintf (messageString, "%s `commands` or `command`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "pri us") == str)
    {
        sprintf (messageString, "%s `privilege user`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "is priv") == str)
    {
        sprintf (messageString, "%s `is privileged`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    else if (strcasestr (str, "unp us") == str)
    {
        sprintf (messageString, "%s `unprivilege user`?", subString);
        postReply (bot->room, messageString, command->message);
        free (str);
        free (message);
        return;
    }
    
        
    postReply(bot->room, message, command->message);
    
    free(str);
    free (message);
    return;
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

PrivRequest **loadPrivRequest ()
{
    puts ("Loading Privilege Requests...");
    FILE *file = fopen ("privRequest.json", "r");
    
    if (!file)
    {
        puts ("privRequest.json does not exist. Returning NULL...");
        return NULL;
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
        char *username = cJSON_GetObjectItem (request, "user_name")->valuestring;
        int groupType = cJSON_GetObjectItem (request, "group_type")->valueint;
        
        requests [i] = createPrivRequest (userID, username, groupType);
    }
    
    if (total < 1)
    {
        cJSON_Delete (json);
        return NULL;
    }
    
    cJSON_Delete (json);
    
    return requests;
}

PrivUsers **loadPrivUsers ()
{
    puts ("Loading Privileged Users...");
    FILE *file = fopen ("privUsers.json", "r");
    
    if (!file)
    {
        puts ("privUsers.json does not exist. Creating skeleton file...");
        PrivUsers **users = malloc(sizeof(Filter*) * 3);
        
        users [0] = createPrivUsers (3476191, "NobodyNada", 2);        // User ID of NobodyNada
        users [1] = createPrivUsers (5735775, "Ashish Ahuja ツ", 2);   // User ID of Ashish Ahuja
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
    PrivUsers **users = malloc(sizeof(PrivUsers*) * (privUsersCount + 1));
    
    for (int i = 0; i < privUsersCount; i ++)
    {
        cJSON *user = cJSON_GetArrayItem(json, i);
        
        long userID = cJSON_GetObjectItem (user, "user_id")->valueint;
        char *username = cJSON_GetObjectItem (user, "user_name")->valuestring;
        int privLevel = cJSON_GetObjectItem (user, "priv_level")->valueint;
        users [i] = createPrivusers (userID, username, privLevel);
    }
    users [privUsersCount] = NULL;
    
    return users;
}

void savePrivUsers (PrivUsers **users, unsigned privUsersCount)
{
    puts ("Saving Privileged users...");
    cJSON *json = cJSON_CreateArray();
    
    for (int i = 0; i < privUsersCount; i ++)
    {
        PrivUsers *user = users [i];
        cJSON *object = cJSON_CreateObject();
        cJSON_AddItemToObject (object, "user_id", cJSON_CreateNumber (user->userID));
        cJSON_AddItemToObject (object, "user_name", cJSON_CreateString (user->username));
        CJSON_AddItemToObject (object, "priv_level", cJSON_CreateNumber (user->privLevel));
        
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
    
    //ChatRoom *roomPostTrue = createChatRoom (client, 773); //773 is room number of LQPHQ
    
    ChatRoom *room = createChatRoom(client, 68414); // 68414 is room number of SOCVR Testing Facility
    
    enterChatRoom(room);
    //enterChatRoom (roomPostTrue);
    
    
    Filter **filters = loadFilters();
    PrivUsers **users = loadPrivUsers();
    
    Command *commands[] = {
        createCommand("I can put anything I want here; the first command runs when no other commands match", 0, unrecognizedCommand),
        createCommand("test1", 0, test1Callback),
        createCommand("test1 test2 ...", 0, testVarCallback),
        createCommand("test1 * test3", 0, testArgCallback),
        createCommand("test2 test3", 0, test2Callback),
        createCommand("running commands", 0, listCommands),
        createCommand("stop", 2, stopBot),
        createCommand("reboot", 1, rebootBot),
        createCommand("kill", 2, forceStopBot),
        createCommand("check post *", 0, checkPostCallback),
        createCommand("tp", 1, truePositive),
        createCommand("fp", 1, falsePositive),
        createCommand("t", 1, truePositive),
        createCommand("f", 1, falsePositive),
        createCommand("help", 0, help),
        createCommand("alive", 0, aliveCheck),
        createCommand("commands", 0, commandList),
        createCommand("command", 0, commandList),
        createCommand("tpr", 1, truePositiveRespond),
        createCommand("fpr", 1, falsePositiveRespond),
        createCommand("stats ...", 0, statistics),
        createCommand("recent reports *", 0, printLatestReports),
        createCommand("latest reports *", 0, printLatestReports),
        createCommand("post info *", 0, postInfo),
        createCommand("tr", 1, truePositiveRespond),
        createCommand("fr", 1, falsePositiveRespond),
        createCommand("cp *", 0, checkPostCallback),
        createCommand("pinfo *", 0, postInfo),
        createCommand("change threshold *", 2, changeThreshold),
        createCommand("check threshold *", 0, checkThreshold),
        createCommand("test post *", 0, testPostCallback),
        createCommand("privilege user * *", 2, addUserPriv),
        createCommand("unprivilege user *", 2, removeUserPriv),
        createCommand("priv user * *", 2, addUserPriv),
        createCommand("unpriv user *", 2, removeUserPriv),
        createCommand("is privileged ...", 0, isPrivileged),
        createCommand("membership ...", 0, printPrivUser),
        NULL
    };
    ChatBot *bot = createChatBot(room, NULL, commands, loadReports(), filters, users);
    
    
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
    savePrivUsers(bot->privUsers, bot->numOfPrivUsers);
    saveReports(bot->latestReports, bot->reportsUntilAnalysis);
    
    
    
    curl_easy_cleanup(client->curl);
    
    if (reboot) {
        execv(argv[0], (char*const*)argv);  //Reload the program.
    }
    
    return 0;
}
