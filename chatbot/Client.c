//
//  Client.c
//  chatbot
//
//  Created on 4/29/16.
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
    c->sockets = NULL;
    c->socketCount = 0;
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
    addWebsocket(c, socket);
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

WebSocket *webSocketWithLWS(Client *c, struct lws *ws) {
    for (int i = 0; i < c->socketCount; i++) {
        if (c->sockets[i]->ws == ws) {
            return c->sockets[i];
        }
    }
    
    return NULL;
}

int websocketCallback(struct lws *ws,
                      enum lws_callback_reasons reason,
                      void *user,
                      void *in, size_t len) {
    char *data;
    Client *c = NULL;
    if (ws) {
        c = (Client *)lws_context_user(lws_get_context(ws));
    }
    WebSocket *socket = NULL;
    if (user) {
        socket = *(WebSocket**)user;
    }
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            puts("Connection established");
            for (int i = 0; i < c->socketCount; i++) {
                if (c->sockets[i]->isSetUp == 0) {
                    socket = c->sockets[i];
                    socket->isSetUp = 1;
                    break;
                }
            }
            *(WebSocket**)user = socket;
            socket->ws = ws;
            if (socket->openCallback != NULL) {
                socket->openCallback(socket);
            }
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            data = malloc((strlen(in) + 1) * sizeof(char));
            strcpy(data, in);
            
            if (socket->recieveCallback != NULL) {
                socket->recieveCallback(socket, data, strlen(in));
            }
            
            free(data);
            break;
        case LWS_CALLBACK_CLOSED:
            puts("Connection closed.");
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            puts("Connection error");
            break;
        default:
            break;
    }
    return 0;
}

void setupWebsocketContext(Client *c) {
    puts("Starting libwebsockets...");
    struct lws_context_creation_info info;
    struct lws_protocols *protocols = malloc(sizeof(struct lws_protocols) * 3);
    
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.iface = NULL;
    info.protocols = protocols;
    info.extensions = NULL;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.extensions = lws_get_internal_extensions();
    info.gid = -1;
    info.uid = -1;
    info.options = 0;
    info.user = c;
    
    
    
    protocols[0].name = "http-only";
    protocols[1].name = "";
    protocols[2].name = "";
    
    protocols[0].callback = websocketCallback;
    protocols[1].callback = websocketCallback;
    protocols[2].callback = NULL;
    
    protocols[0].id = 0;
    protocols[1].id = 0;
    protocols[2].id = 0;
    
    protocols[0].per_session_data_size = sizeof(WebSocket*);
    protocols[1].per_session_data_size = sizeof(WebSocket*);
    protocols[2].per_session_data_size = sizeof(WebSocket*);
    
    protocols[0].user = c;
    protocols[1].user = c;
    protocols[2].user = c;
    
    protocols[0].rx_buffer_size = 0;
    protocols[1].rx_buffer_size = 0;
    protocols[2].rx_buffer_size = 0;
    
    
    
    c->wsContext = lws_create_context(&info);
    if (c->wsContext == NULL) {
        fputs("Failed to create websocket context!\n", stderr);
        exit(EXIT_FAILURE);
    }
    
    //lws_set_proxy(c->wsContext, "localhost:8080");
}

void serviceWebsockets(Client *client) {
    lws_service(client->wsContext, 50);
}

///Sends data across the websocket.
///@param socket: The websocket to send data to.
///@param data: The data to send.
///@param len: The size in bytes of the data.  If len is 0, strlen(buf) bytes are sent.
unsigned sendDataOnWebsocket(struct lws *socket, void *data, size_t len) {
    if (len == 0) {
        len = strlen(data);
    }
    unsigned char *buf = malloc(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING);
    memcpy(buf + LWS_SEND_BUFFER_PRE_PADDING, data, len);
    
    int ret = lws_write(socket, buf + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);
    
    free(buf);
    return ret;
}

void addWebsocket(Client *client, WebSocket *ws) {
    client->sockets = realloc(client->sockets, ++client->socketCount * sizeof(WebSocket*));
    client->sockets[client->socketCount - 1] = ws;
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
    
    setupWebsocketContext(client);
    //client->ws = connectWebsocket(client, "qa.sockets.stackexchange.com", "/");
    //sendDataOnWebsocket(client->ws, "155-questions-active", 0);
    
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

