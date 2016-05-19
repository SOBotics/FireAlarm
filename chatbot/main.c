//
//  main.c
//  chatbot
//
//  Created on 4/29/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>

#include "Client.h"
#include "ChatRoom.h"
#include "ChatBot.h"
#include "cJSON.h"
#include "commands.h"
#include "Filter.h"

void unrecognizedCommand(RunningCommand *c, void *ctx) {
    ChatBot *bot = ctx;
    char *str;
    asprintf(&str, "Unrecognized command \"%s\"", c->message->content);
    postReply(bot->room, str, c->message);
    free(str);
}

void webSocektOpened(WebSocket *ws) {
    puts("Websocket opened");
    sendDataOnWebsocket(ws->ws, "155-questions-active", 0);
}

void wsRecieved(WebSocket *ws, char *data, size_t len) {
    //cJSON *json = cJSON_Parse(data);
    //puts(cJSON_Print(cJSON_Parse(cJSON_GetObjectItem(json, "data")->valuestring)));
    //ChatBot *bot = (ChatBot*)ws->user;
    
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
    
    
    //Determine where to save cookies.
    //Get the user's home directory.
    //http://stackoverflow.com/a/26696759/3476191
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    //Append the cookie location.
    const char *cookieName = "/.chatbot_cookies";
    char cookiePath[strlen(homedir) + strlen(cookieName) + 1];
    cookiePath[0] = 0;
    strcpy(cookiePath, homedir);
    strcat(cookiePath, cookieName);
    char resolvedPath[PATH_MAX];
    realpath(cookiePath, resolvedPath);
    
    Client *client = createClient("stackoverflow.com", cookiePath);
    if (!client->isLoggedIn) {
        char *env_email, *env_pass;
        if (
            (env_email = getenv("ChatBotEmail")) == NULL ||
            (env_pass = getenv("ChatBotPass")) == NULL
            ) {
            //Get the email from the user
            printf("Email address: ");
            char email[_PASSWORD_LEN];
            fgets(email, _PASSWORD_LEN, stdin);
            email[_PASSWORD_LEN-1] = 0;   //make sure it's null terminated
            
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
    
    
    Filter *filters[] = {
        createFilter("test filter", ".*", FILTER_REGEX),
        NULL
    };
    
    Command *commands[] = {
        createCommand("I can put anything I want here; the first command runs when no other commands match", unrecognizedCommand),
        createCommand("test1", test1Callback),
        createCommand("test1 test2 ...", testVarCallback),
        createCommand("test1 * test3", testArgCallback),
        createCommand("test2 test3", test2Callback),
        createCommand("running commands", listCommands),
        createCommand("pingable users", listPingable),
        createCommand("stop", stopBot),
        createCommand("reboot", rebootBot),
        createCommand("kill", forceStopBot),
        createCommand("check post *", checkPostCallback),
        NULL
    };
    ChatBot *bot = createChatBot(room, commands, filters);
    
    
    WebSocket *socket = createWebSocketWithClient(client);
    socket->user = bot;
    socket->openCallback = webSocektOpened;
    socket->recieveCallback = wsRecieved;
    connectWebSocket(socket, "qa.sockets.stackexchange.com", "/");
    
    puts("Started.");
    
    
    for (;;) {
        serviceWebsockets(client);
        StopAction action = runChatBot(bot);
        if (action == ACTION_STOP) {
            break;
        }
        else if (action == ACTION_REBOOT) {
            curl_easy_cleanup(client->curl);
            execv(argv[0], (char*const*)argv);  //Load this program into the current process.
        }                                       //This will cause the bot to restart.
    }
    
    curl_easy_cleanup(client->curl);
    
    return 0;
}
