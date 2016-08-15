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

#include "Privileges.h"
#include "ChatRoom.h"
#include "ChatBot.h"
#include "cJSON.h"
#include "commands.h"
#include "Filter.h"
#undef Client_h
#include "Client.h"

#define SAVE_INTERVAL 60
//long postMessage = 1;

void unrecognizedCommand(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    char *str;
    char *message;
    //char messageString [256];
    //char subString [127];

    asprintf (&str, "%s", command->message->content);
    asprintf(&message, "Unrecognized command `%s`.", command->message->content);
    /*sprintf (subString, "%s Did you want to type in", message);

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
    }*/


    postReply(bot->room, message, command->message);

    free(str);
    free (message);
    return;
}

void webSocketOpened(WebSocket *ws) {
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
        int falsePositives = cJSON_GetObjectItem(filter, "falsePositives")->valueint;
        if (falsePositives == -1) {
            falsePositives = 25 - truePositives;
            if (falsePositives < 0) {
                falsePositives = 0;
            }
        }
        filters[i] = createFilter(desc, expr, type, truePositives, falsePositives);
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

    if (!file)
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
        cJSON_AddItemToObject (object, "user_id", cJSON_CreateNumber (user->userID));
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

void wsClosed(WebSocket *socket) {
    //postMessage(((ChatBot*)(socket->user))->room, "Websocket disconnected! Reboot the bot to reconnect the websocket. (cc @NobodyNada)");
    postMessage(((ChatBot*)(socket->user))->room, "@FireAlarm reboot");
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
    PrivUser **users = loadPrivUsers();
    PrivRequest **requests = loadPrivRequests();
    Modes *modes = createMode (1, 1, 1, 1);
    Notify **notify = loadNotifications ();

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
        createCommand("recent reports ...", 0, printLatestReports),
        createCommand("latest reports ...", 0, printLatestReports),
        createCommand("post info *", 0, postInfo),
        createCommand("tr", 1, truePositiveRespond),
        createCommand("fr", 1, falsePositiveRespond),
        createCommand("cp *", 0, checkPostCallback),
        createCommand("pinfo *", 0, postInfo),
        createCommand("change threshold *", 2, modifyFilterThreshold),
        createCommand("check threshold", 0, checkThreshold),
        createCommand("test post *", 0, testPostCallback),
        createCommand("privilege user * *", 2, addUserPriv),
        createCommand("unprivilege user * *", 2, removeUserPriv),
        createCommand("priv user * *", 2, addUserPriv),
        createCommand("unpriv user *", 2, removeUserPriv),
        createCommand("is privileged ...", 0, isPrivileged),
        createCommand("membership ...", 0, printPrivUser),
        createCommand("request privileges *", 0, requestPriv),
        createCommand("request priv *", 0, requestPriv),
        createCommand("approve privilege request *", 2, approvePrivRequest),
        createCommand("approve priv request *", 2, approvePrivRequest),
        createCommand("reject privilege request *", 2, rejectPrivRequest),
        createCommand("reject priv request *", 2, rejectPrivRequest),
        createCommand("pending privilege requests", 0, printPrivRequests),
        createCommand("pending priv requests", 0, printPrivRequests),
        createCommand("amiprivileged", 0, amiPrivileged),
        createCommand("mode * * ...", 2, changeMode),
        createCommand("check mode", 0, printCurrentMode),
        createCommand("opt in *", 0, optIn),
        createCommand("opt out *", 0, optOut),
        createCommand("notify me *", 0, notifyMe),
        createCommand("unnotify me *", 0, unnotifyMe),
        createCommand("say ...", 0, say),
        createCommand("aminotified", 0, amINotified),
        createCommand("is notified *", 0, isNotified),
        createCommand("notified users", 0, printNotifiedUsers),
        createCommand("unclosed t", 0, printUnclosedTP),
        createCommand("unclosed tp", 0, printUnclosedTP),
        createCommand("unclosed true", 0, printUnclosedTP),
        createCommand("api quota", 0, apiQuota),
        NULL
    };
    ChatBot *bot = createChatBot(room, NULL, commands, loadReports(), filters, users, requests, modes, notify);


    WebSocket *socket = createWebSocketWithClient(client);
    socket->user = bot;
    socket->openCallback = webSocketOpened;
    socket->recieveCallback = wsRecieved;
    socket->closeCallback = wsClosed;
    connectWebSocket(socket, "qa.sockets.stackexchange.com", "/");

    puts("Fire Alarm started.");
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
            savePrivUsers(bot->privUsers, bot->numOfPrivUsers);
            saveReports(bot->latestReports, bot->reportsUntilAnalysis);
            savePrivRequests(bot->privRequests, bot->totalPrivRequests);
            saveNotifications (bot->notify, bot->totalNotifications);
            saveTime = time(NULL) + SAVE_INTERVAL;
        }
    }

    leaveRoom(bot->room);

    puts("Saving data...");
    saveFilters(bot->filters, bot->filterCount);
    savePrivUsers(bot->privUsers, bot->numOfPrivUsers);
    saveReports(bot->latestReports, bot->reportsUntilAnalysis);
    savePrivRequests(bot->privRequests, bot->totalPrivRequests);
    saveNotifications (bot->notify, bot->totalNotifications);

    puts("Waiting (to allow networking to finish)...");
    sleep(5);   //give background threads a bit of time

    curl_easy_cleanup(client->curl);

    if (reboot) {
        execv(argv[0], (char*const*)argv);  //Reload the program.
    }

    return 0;
}
