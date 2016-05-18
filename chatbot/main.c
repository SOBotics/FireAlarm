//
//  main.c
//  chatbot
//
//  Created by Jonathan Keller on 4/29/16.
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

void unrecognizedCommand(RunningCommand *c, void *ctx) {
    ChatBot *bot = ctx;
    char *str;
    asprintf(&str, "Unrecognized command \"%s\"", c->message->content);
    postReply(bot->room, str, c->message);
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
    ChatRoom *room = createChatRoom(client, 41570);
    enterChatRoom(room);
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
        NULL
    };
    ChatBot *bot = createChatBot(room, commands);
    
    puts("Started.");
    
    for (;;) {
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
