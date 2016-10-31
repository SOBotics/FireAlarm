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
#include "Logs.h"
#include "LoadData.h"

#define SAVE_INTERVAL 60
#define REPO_URL "https://git.io/vPis7"
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


    //postReply(bot->room, message, command->message);

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
    //printf ("\n%s\n", data);
    cJSON *post = cJSON_Parse(cJSON_GetObjectItem(json, "data")->valuestring);
    if (strcmp(cJSON_GetObjectItem(post, "apiSiteParameter")->valuestring, "stackoverflow")) {
        //if this isn't SO
        cJSON_Delete(json);
        cJSON_Delete(post);
        return;
    }
    ChatBot *bot = (ChatBot*)ws->user;
    Post *p = getPostByID(bot, cJSON_GetObjectItem(post, "id")->valueint);
    if (p != NULL && !p->isAnswer) {
        printf ("checking post: %lu", p->postID);
        checkPost(bot, p);
    }
    else {
        printf("Got a null post: %d\n", cJSON_GetObjectItem(post, "id")->valueint);
    }
    cJSON_Delete(json);
    cJSON_Delete(post);
}

void wsClosed(WebSocket *socket) {
    //postMessage(((ChatBot*)(socket->user))->room, "Websocket disconnected! Reboot the bot to reconnect the websocket. (cc @NobodyNada)");
    postMessage(((ChatBot*)(socket->user))->room, "@FireAlarm reboot");
}

int main(int argc, const char * argv[]) {
    // insert code here...
    reboot:

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

            //Get the password
            char *password = getpass("Password: ");
            loginWithEmailAndPassword(client, email, password);
            //Overwrite the password so it doesn't stay in memory
            memset(password, 0, strlen(password));
        }
        else {
            loginWithEmailAndPassword(client, env_email, env_pass);
            memset(env_pass, 0, strlen(env_pass));
        }
    }

    //ChatRoom *roomPostTrue = createChatRoom (client, 111347); //773 is room number of LQPHQ

    ChatRoom *room = createChatRoom(client, 123602); // 111347 is SOBotics; 123602 is FA Dev

    enterChatRoom(room);
    //enterChatRoom (roomPostTrue);


    Filter **filters = loadFilters();
    PrivUser **users = loadPrivUsers();
    PrivRequest **requests = loadPrivRequests();
    Modes *modes = createMode (1, 1, 1, 1);
    Notify **notify = loadNotifications ();
    Log **logs = loadLogs ();

    Command *commands[] = {
        createCommand("I can put anything I want here; the first command runs when no other commands match", 0, unrecognizedCommand),
        createCommand("test1 ...", 0, test1Callback),
        createCommand("test1 test2 ...", 0, testVarCallback),
        createCommand("test1 * test3", 0, testArgCallback),
        createCommand("test", 0, test2Callback),
        createCommand("running commands", 0, listCommands),
        createCommand("stop", 2, stopBot),
        createCommand("reboot", 1, rebootBot),
        createCommand("kill", 2, forceStopBot),
        createCommand("check post *", 0, checkPostCallback),
        createCommand("tp ...", 1, truePositive),
        createCommand("fp ...", 1, falsePositive),
        createCommand("t ...", 1, truePositive),
        createCommand("f ...", 1, falsePositive),
        createCommand("help", 0, help),
        createCommand("alive", 0, aliveCheck),
        createCommand("commands", 0, commandList),
        createCommand("command", 0, commandList),
        createCommand("tpr ...", 1, truePositiveRespond),
        createCommand("fpr ...", 1, falsePositiveRespond),
        createCommand("stats ...", 0, statistics),
        createCommand("recent reports ...", 0, printLatestReports),
        createCommand("latest reports ...", 0, printLatestReports),
        createCommand("post info *", 0, postInfo),
        createCommand("tr ...", 1, truePositiveRespond),
        createCommand("fr ...", 1, falsePositiveRespond),
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
        createCommand("opt in", 0, optIn),
        createCommand("opt out", 0, optOut),
        createCommand("notify me", 0, notifyMe),
        createCommand("unnotify me", 0, unnotifyMe),
        createCommand("say ...", 0, say),
        createCommand("aminotified", 0, amINotified),
        createCommand("is notified *", 0, isNotified),
        createCommand("notified users", 0, printNotifiedUsers),
        createCommand("unclosed t", 0, printUnclosedTP),
        createCommand("unclosed tp", 0, printUnclosedTP),
        createCommand("unclosed true", 0, printUnclosedTP),
        createCommand("api quota", 0, printApiQuota),
        createCommand("filter modify threshold *", 2, modifyFilterThreshold),
        createCommand("filter add keyword * * *", 2, addKeywordToFilter),
        createCommand("filter view keywords", 0, printKeywordsInFilter),
        createCommand("filter view keyword", 0, printKeywordsInFilter),
        createCommand("filter modify keyword * * *", 2, modifyKeywordFilter),
        createCommand("filter info * *", 0, filterInfo),
        createCommand("filter view reports * ... ...", 0, printReportsByFilter),
        createCommand("filter view filters", 0, printFilters),
        createCommand("filter disable *", 2, disableFilter),
        createCommand("filter enable *", 2, enableFilter),
        createCommand("error logs", 0, printErrorLogs),
        createCommand("clear error logs", 2, clearErrorLogs),
        NULL
    };
    ChatBot *bot = createChatBot(room, NULL, commands, loadReports(), filters, users, requests, modes, notify, logs);


    WebSocket *socket = createWebSocketWithClient(client);
    socket->user = bot;
    socket->openCallback = webSocketOpened;
    socket->recieveCallback = wsRecieved;
    socket->closeCallback = wsClosed;
    connectWebSocket(socket, "qa.sockets.stackexchange.com", "/");

    puts("Fire Alarm started.");
    postMessage (bot->room, "[Fire Alarm](https://github.com/NobodyNada/chatbot) started.");
    //registerError (bot, "main.c/main", "test error", "main");
    //registerError (bot, "main.c/main", "test error 2", "main");

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
    //leaveRoom(bot->roomPostTrue);

    puts("Saving data...");
    saveFilters(bot->filters, bot->filterCount);
    savePrivUsers(bot->privUsers, bot->numOfPrivUsers);
    saveReports(bot->latestReports, bot->reportsUntilAnalysis);
    savePrivRequests(bot->privRequests, bot->totalPrivRequests);
    saveNotifications (bot->notify, bot->totalNotifications);
    saveLogs (bot->log, bot->totalLogs);

    puts("Waiting (to allow networking to finish)...");
    sleep(5);   //give background threads a bit of time

    curl_easy_cleanup(client->curl);

    if (reboot) {
        execv(argv[0], (char*const*)argv);  //Reload the binaries which will restart the program.
    }

    return 0;
}
