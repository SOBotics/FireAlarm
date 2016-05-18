//
//  Client.c
//  chatbot
//
//  Created by Jonathan Keller on 4/29/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "Client.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void checkCurlError(CURLcode code, const char *func, const char *file, int line) {
    if (code) {
        fprintf(stderr, "CURL error: %d\nfile: %s\nline: %d\nfunction: %s\n", code, file, line, func);
        exit(EXIT_FAILURE);
    }
}

size_t curlCallback(char *data, size_t inSize, size_t nmemb, OutBuffer *outBuf) {
    size_t size = inSize * nmemb;
    size_t where = 0; //The index in outBuf->data to write to.
    if (outBuf->data == NULL) {
        size = size + 1;
        outBuf->size = size;
        outBuf->data = malloc(size);
    } else {
        where = outBuf->size - 1;
        size = outBuf->size + size;
        outBuf->data = realloc(outBuf->data, size);
    }
    
    memcpy(outBuf->data + where, data, inSize * nmemb);
    outBuf->data[size-1] = 0;
    
    return inSize * nmemb;
}

#ifdef DEBUG

int curlDebug(CURL *curl, curl_infotype type, char *data, size_t size, void *usrptr) {
    if (type != CURLINFO_TEXT) {
        return 0;
    }
    //null terminate the data
    char buf[size+1];
    memcpy(buf, data, size);
    buf[size+1] = 0;
    
    printf("curl: %s\n", data);
    
    return 0;
}

#endif

Client *createClient(const char *host, const char *cookiefile) {
    Client *c = malloc(sizeof(Client));
    c->fkey = NULL;
    c->host = malloc(strlen(host) + 1);
    strcpy(c->host, host);
    
    CURL *curl = curl_easy_init();
    c->curl = curl;
    //Configure CURL
    
    //Uncomment this to pass all requests through mitmproxy, for debugging.
    //checkCURL(curl_easy_setopt(curl, CURLOPT_PROXY, "https://127.0.0.1:8080"));
    
#ifdef DEBUG
    //checkCURL(curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, curlDebug));
    //checkCURL(curl_easy_setopt(curl, CURLOPT_VERBOSE, 1));
#endif
    
    char cookiebuf[PATH_MAX];
    realpath(cookiefile, cookiebuf);
    
    //Enable cookies.
    checkCURL(curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiebuf));
    checkCURL(curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiebuf));
    
    
    //Enable SSL.
    checkCURL(curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY));
    
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    
    
    //Specify callback data
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback));
    
    OutBuffer buf;
    buf.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf));
    
    //check if we are currently logged in by seeing if stackexchange.com/logout redirects
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, "https://stackexchange.com/users/logout"));
    checkCURL(curl_easy_perform(curl));
    
    long http_code = 0;
    checkCURL(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code));
    
    c->isLoggedIn = (http_code == 200);
    
    
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    return c;
}

void getFkey(Client *client, char *data) {
    //Find the fkey
    char *fkeyLocation = strstr(data, "name=\"fkey\"");
    if (fkeyLocation == NULL) {
        client->fkey = NULL;
        return;
    }
    fkeyLocation = strstr(fkeyLocation, "value=");
    
    if (fkeyLocation == NULL) {
        client->fkey = NULL;
        return;
    }
    
    while (*(fkeyLocation++) != '"');   //point it to after the opening quote
    int fkeySize = 8;
    char *fkey = malloc(fkeySize + 1);
    int pos;
    for (pos = 0; fkeyLocation[pos] != '"'; pos++) {
        if (fkeyLocation[pos] == 0) {
            client->fkey = NULL;
            return;
        }
        if (pos >= fkeySize) {
            fkeySize *= 2;
            fkey = realloc(fkey, fkeySize + 1);
        }
        fkey[pos] = fkeyLocation[pos];
    }
    fkey[pos] = 0;
    client->fkey = fkey;
}

void loginWithEmailAndPassword(Client *client, const char *email, const char *password) {
    if (client->isLoggedIn) {
        fputs("already logged in\n", stderr);
        exit(EXIT_FAILURE);
    }
    puts("Logging in...");
    CURL *curl = client->curl;
    
    OutBuffer buffer;
    buffer.data = NULL;
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    
    //Log in to Stack Exchange.
    checkCURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                               "from=https%3A%2F%2Fstackexchange.com%2Fusers%2Flogin%23log-in"));
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL,
                               "https://stackexchange.com/users/signin")
              );
    
    checkCURL(curl_easy_perform(curl));
    //getFkey(client, buffer.data);
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, buffer.data));
    free(buffer.data);
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    checkCURL(curl_easy_perform(curl));
    getFkey(client, buffer.data);
    
    //Make a buffer for POST data.
    const size_t maxPostLength = 256;
    char postBuffer[maxPostLength];
    postBuffer[0] = 0;
    
    char *escapedEmail = curl_easy_escape(curl, email, (int)strlen(email));
    char *escapedPassword = curl_easy_escape(curl, password, (int)strlen(password));
    snprintf(postBuffer,
             maxPostLength - 1,
             "email=%s&password=%s&affId=11&fkey=%s",
             escapedEmail,
             escapedPassword,
             client->fkey
             );
    
    //Overwrite the password with zeroes.
    memset(escapedPassword, 0, strlen(escapedPassword));
    curl_free(escapedEmail);
    curl_free(escapedPassword);
    
    postBuffer[maxPostLength-1] = 0;
    
    
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 0));
    checkCURL(curl_easy_setopt(curl, CURLOPT_POST, 1));
    checkCURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBuffer));
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, "https://openid.stackexchange.com/affiliate/form/login/submit"));
    
    free(buffer.data);
    buffer.data = NULL;
    checkCURL(curl_easy_perform(curl));
    
    
    //Overwrite the buffer with zeroes since it contains the password.
    memset(postBuffer, 0, maxPostLength);
    
    //Get the authenticate link.
    char *authLink = malloc(strlen(buffer.data) + 1);
    char *authLink_orig = authLink; //so we can free it
    strcpy(authLink, buffer.data);
    free(buffer.data);
    buffer.data = NULL;
    
    authLink = strstr(authLink, "<a");
    authLink = strchr(authLink, '"') + 1;
    *(strchr(authLink, '"')) = 0;
    
    //Follow the link.
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L));
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, authLink));
    free(authLink_orig);
    checkCURL(curl_easy_perform(curl));
    
    free(buffer.data);
    buffer.data = NULL;
    
    if (!strstr(client->host, "stackexchange")) {   //If we're not at stackexchange.com,
        //Log into host.
        //Make a buffer for POST data.
        const size_t maxPostLength = 256;
        char postBuffer[maxPostLength] = {0};
        
        snprintf(postBuffer, maxPostLength,
                 "https://%s/users/login",
                 client->host
                 );
        checkCURL(curl_easy_setopt(curl, CURLOPT_URL, postBuffer));
        postBuffer[0] = 0;
        
        
        char *escapedEmail = curl_easy_escape(curl, email, (int)strlen(email));
        char *escapedPassword = curl_easy_escape(curl, password, (int)strlen(password));
        snprintf(postBuffer,
                 maxPostLength - 1,
                 "email=%s&password=%s",
                 escapedEmail,
                 escapedPassword
                 );
        
        //Overwrite the password with zeroes.
        memset(escapedPassword, 0, strlen(escapedPassword));
        curl_free(escapedEmail);
        curl_free(escapedPassword);
        
        postBuffer[maxPostLength-1] = 0;
        
        
        checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 0));
        checkCURL(curl_easy_setopt(curl, CURLOPT_POST, 1));
        checkCURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBuffer));
        
        checkCURL(curl_easy_perform(curl));
        
        
        //Overwrite the buffer with zeroes since it contains the password.
        memset(postBuffer, 0, maxPostLength);
    }
}

void processClientEvents(Client *client) {
    
}

unsigned long connectClientToRoom(Client *client, unsigned roomID) {
    CURL *curl = client->curl;
    
    OutBuffer buffer;
    buffer.data = NULL;
    checkCURL(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer));
    
    //Get the chat fkey.
    checkCURL(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1));
    const size_t maxLength = 256;
    char request[maxLength];
    snprintf(request, maxLength,
             "chat.%s/chats/join/favorite",
             client->host
             );
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, request));
    checkCURL(curl_easy_perform(curl));
    
    free(client->fkey);
    getFkey(client, buffer.data);
    free(buffer.data);
    buffer.data = NULL;
    if (client->fkey == NULL) {
        fputs("Could not find fkey!\n", stderr);
        exit(EXIT_FAILURE);
    }
    
    
    //get the timestamp
    
    const size_t maxPostLength = 256;
    char postBuffer[maxPostLength];
    
    snprintf(postBuffer, maxPostLength - 1, "roomid=%d&fkey=%s", roomID, client->fkey);
    checkCURL(curl_easy_setopt(curl, CURLOPT_POST, 1));
    checkCURL(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBuffer));
    
    
    
    
    char urlBuffer[maxPostLength];
    
    snprintf(urlBuffer, maxPostLength, "chat.%s/chats/%d/events", client->host, roomID);
    checkCURL(curl_easy_setopt(curl, CURLOPT_URL, urlBuffer));
    
    
    curl_easy_perform(curl);
    
    cJSON *json = cJSON_Parse(buffer.data);
    free(buffer.data);
    
    unsigned long time = cJSON_GetObjectItem(json, "time")->valueint;
    cJSON_Delete(json);
    return time;
}

