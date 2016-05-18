//
//  WebSocket.h
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef WebSocket_h
#define WebSocket_h

#include <stdio.h>
#include <libwebsockets.h>
#include "Client.h"


typedef struct _WebSocket WebSocket;

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

#endif /* WebSocket_h */
