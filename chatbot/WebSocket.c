//
//  WebSocket.c
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "WebSocket.h"

WebSocket *createWebSocketWithClient(Client *client) {
    WebSocket *s = malloc(sizeof(WebSocket));
    
    s->openCallback = NULL;
    s->recieveCallback = NULL;
    s->ws = NULL;
    s->user = NULL;
    s->client = client;
    s->isSetUp = 0;
    
    return s;
}

void connectWebSocket(WebSocket *socket, const char *host, const char *path) {
    Client *c = socket->client;
    struct lws *ws = lws_client_connect(
                                        c->wsContext,
                                        host,
                                        80,
                                        0,
                                        path,
                                        host,
                                        NULL,
                                        NULL,
                                        -1
                                        );
    
    
    
    if (ws == NULL) {
        fputs("Failed to create websocket!\n", stderr);
        exit(EXIT_FAILURE);
    }
    
    
}