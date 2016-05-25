//
//  Client.h
//  chatbot
//
//  Created on 4/29/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef Client_h
#define Client_h

#include <stdio.h>
#include <pthread.h>
#include <curl/curl.h>
#include <libwebsockets.h>

typedef struct _WebSocket WebSocket;


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
    struct lws_context *wsContext;
    WebSocket **sockets;
    size_t socketCount;
    
}Client;

Client *createClient(const char *host, const char *cookiefile);
void loginWithEmailAndPassword(Client *client, const char* email, const char* password);
unsigned long connectClientToRoom(Client *client, unsigned roomID); //Connects to a chat room.  Returns the current timestamp.
void serviceWebsockets(Client *client);
void addWebsocket(Client *client, WebSocket *ws);
void checkCurlError(CURLcode code, const char *func, const char *file, int line);
unsigned sendDataOnWebsocket(struct lws *socket, void *data, size_t len);




typedef void (*WebSocketOpenCallback)(struct _WebSocket *ws);
typedef void (*WebSocketRecieveCallback)(struct _WebSocket *ws, char *data, size_t len);


typedef struct _WebSocket {
    Client *client;
    struct lws *ws;
    void *user;
    unsigned char isSetUp;
    WebSocketOpenCallback openCallback;
    WebSocketRecieveCallback recieveCallback;
}WebSocket;

WebSocket *createWebSocketWithClient(Client *client);
void connectWebSocket(WebSocket *socket, const char *host, const char *path);


#endif /* Client_h */
