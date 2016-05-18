//
//  Client.h
//  chatbot
//
//  Created by Jonathan Keller on 4/29/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef Client_h
#define Client_h

#include <stdio.h>
#include <curl/curl.h>

//Pass this to the CURL callback.
typedef struct _OutBuffer {
    size_t size;
    char *data;
}OutBuffer;

#define checkCURL(code) checkCurlError(code, __PRETTY_FUNCTION__, __FILE__, __LINE__)

typedef struct _Client {
    
    int isLoggedIn;
    CURL *curl;
    char *fkey;
    char *host;
    
}Client;

Client *createClient(const char *host, const char *cookiefile);
void loginWithEmailAndPassword(Client *client, const char* email, const char* password);
unsigned long connectClientToRoom(Client *client, unsigned roomID); //Connects to a chat room.  Returns the current timestamp.
void processClientEvents(Client *client);
void checkCurlError(CURLcode code, const char *func, const char *file, int line);


#endif /* Client_h */
