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
#define LWS_MAX_SMP ${LWS_MAX_SMP}
#define LWS_MAX_EXTENSIONS_ACTIVE 2


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


  struct lws_context {
        time_t last_timeout_check_s;
	time_t last_ws_ping_pong_check_s;
	time_t time_up;
	struct lws_plat_file_ops fops;
	//struct lws_context_per_thread pt[LWS_MAX_SMP];
	int count_wsi_allocated;
	int count_cgi_spawned;
	unsigned int options;
	unsigned int fd_limit_per_thread;
	unsigned int timeout_secs;
	unsigned int pt_serv_buf_size;
	int max_http_header_data;

	/*
	 * set to the Thread ID that's doing the service loop just before entry
	 * to poll indicates service thread likely idling in poll()
	 * volatile because other threads may check it as part of processing
	 * for pollfd event change.
	 */
	volatile int service_tid;
	int service_tid_detected;

	short max_http_header_pool;
	short count_threads;
	short plugin_protocol_count;
	short plugin_extension_count;
	short server_string_len;
	unsigned short ws_ping_pong_interval;
	void *user_space;

	unsigned int being_destroyed:1;
	unsigned int requested_kill:1;
	unsigned int protocol_init_done:1;
  };

  struct lws {

	/* structs */
	/* members with mutually exclusive lifetimes are unionized */

	union u {
		struct lws_http_mode_related *http;
#ifdef LWS_USE_HTTP2
		struct _lws_http2_related http2;
#endif
		struct lws_header_related *hdr;
		struct lws_websocket_related *ws;
	} u;

	/* lifetime members */

#if defined(LWS_USE_LIBEV) || defined(LWS_USE_LIBUV)
	struct lws_io_watcher w_read;
#endif
#if defined(LWS_USE_LIBEV)
	struct lws_io_watcher w_write;
#endif
	time_t pending_timeout_limit;

	/* pointers */

	struct lws_context *context;
	struct lws_vhost *vhost;
	struct lws *parent; /* points to parent, if any */
	struct lws *child_list; /* points to first child */
	struct lws *sibling_list; /* subsequent children at same level */
#ifdef LWS_WITH_CGI
	struct lws_cgi *cgi; /* wsi being cgi master have one of these */
#endif
	const struct lws_protocols *protocol;
	struct lws **same_vh_protocol_prev, *same_vh_protocol_next;
	struct lws *timeout_list;
	struct lws **timeout_list_prev;
#ifdef LWS_WITH_ACCESS_LOG
	struct lws_access_log access_log;
#endif
	void *user_space;
	/* rxflow handling */
	unsigned char *rxflow_buffer;
	/* truncated send handling */
	unsigned char *trunc_alloc; /* non-NULL means buffering in progress */

#if defined (LWS_WITH_ESP8266)
	void *premature_rx;
	unsigned short prem_rx_size, prem_rx_pos;
#endif

#ifndef LWS_NO_EXTENSIONS
	const struct lws_extension *active_extensions[LWS_MAX_EXTENSIONS_ACTIVE];
	void *act_ext_user[LWS_MAX_EXTENSIONS_ACTIVE];
#endif
#ifdef LWS_OPENSSL_SUPPORT
	SSL *ssl;
#if !defined(LWS_USE_POLARSSL) && !defined(LWS_USE_MBEDTLS)
	BIO *client_bio;
#endif
	struct lws *pending_read_list_prev, *pending_read_list_next;
#endif
#ifdef LWS_WITH_HTTP_PROXY
	struct lws_rewrite *rw;
#endif
#ifdef LWS_LATENCY
	unsigned long action_start;
	unsigned long latency_start;
#endif
	/* pointer / int */
	lws_sockfd_type sock;

	/* ints */
	int position_in_fds_table;
	int rxflow_len;
	int rxflow_pos;
	unsigned int trunc_alloc_len; /* size of malloc */
	unsigned int trunc_offset; /* where we are in terms of spilling */
	unsigned int trunc_len; /* how much is buffered */
#ifndef LWS_NO_CLIENT
	int chunk_remaining;
#endif
	unsigned int cache_secs;

	unsigned int hdr_parsing_completed:1;
	unsigned int http2_substream:1;
	unsigned int listener:1;
	unsigned int user_space_externally_allocated:1;
	unsigned int socket_is_permanently_unusable:1;
	unsigned int rxflow_change_to:2;
	unsigned int more_rx_waiting:1; /* has to live here since ah may stick to end */
	unsigned int conn_stat_done:1;
	unsigned int cache_reuse:1;
	unsigned int cache_revalidate:1;
	unsigned int cache_intermediaries:1;
	unsigned int favoured_pollin:1;
	unsigned int sending_chunked:1;
	unsigned int already_did_cce:1;
#if defined(LWS_WITH_ESP8266)
	unsigned int pending_send_completion:3;
	unsigned int close_is_pending_send_completion:1;
#endif
#ifdef LWS_WITH_ACCESS_LOG
	unsigned int access_log_pending:1;
#endif
#ifndef LWS_NO_CLIENT
	unsigned int do_ws:1; /* whether we are doing http or ws flow */
	unsigned int chunked:1; /* if the clientside connection is chunked */
	unsigned int client_rx_avail:1;
	unsigned int client_http_body_pending:1;
#endif
#ifdef LWS_WITH_HTTP_PROXY
	unsigned int perform_rewrite:1;
#endif
#ifndef LWS_NO_EXTENSIONS
	unsigned int extension_data_pending:1;
#endif
#ifdef LWS_OPENSSL_SUPPORT
	unsigned int use_ssl:3;
	unsigned int upgraded:1;
#endif
#ifdef _WIN32
	unsigned int sock_send_blocking:1;
#endif
#ifdef LWS_OPENSSL_SUPPORT
	unsigned int redirect_to_https:1;
#endif

	/* chars */
#ifndef LWS_NO_EXTENSIONS
	unsigned char count_act_ext;
#endif
	unsigned char ietf_spec_revision;
	char mode; /* enum connection_mode */
	char state; /* enum lws_connection_states */
	char state_pre_close;
	char lws_rx_parse_state; /* enum lws_rx_parse_state */
	char rx_frame_type; /* enum lws_write_protocol */
	char pending_timeout; /* enum pending_timeout */
	char pps; /* enum lws_pending_protocol_send */
	char tsi; /* thread service index we belong to */
	char protocol_interpret_idx;
#ifdef LWS_WITH_CGI
	char cgi_channel; /* which of stdin/out/err */
	char hdr_state;
#endif
#ifndef LWS_NO_CLIENT
	char chunk_parser; /* enum lws_chunk_parser */
#endif
#if defined(LWS_WITH_CGI) || !defined(LWS_NO_CLIENT)
	char reason_bf; /* internal writeable callback reason bitfield */
#endif
};

Client *createClient(const char *host, const char *cookiefile);
void loginWithEmailAndPassword(Client *client, const char* email, const char* password);
unsigned long connectClientToRoom(Client *client, unsigned roomID); //Connects to a chat room.  Returns the current timestamp.
void serviceWebsockets(Client *client);
void addWebsocket(Client *client, WebSocket *ws);
void checkCurlError(CURLcode code, const char *func, const char *file, int line);
unsigned sendDataOnWebsocket(struct lws *socket, void *data, size_t len);




typedef void (*WebSocketOpenCallback)(struct _WebSocket *ws);
typedef void (*WebSocketClosedCallback)(struct _WebSocket *ws);
typedef void (*WebSocketRecieveCallback)(struct _WebSocket *ws, char *data, size_t len);


typedef struct _WebSocket {
    Client *client;
    struct lws *ws;
    void *user;
    unsigned char isSetUp;
    WebSocketOpenCallback openCallback;
    WebSocketRecieveCallback recieveCallback;
    WebSocketClosedCallback closeCallback;
}WebSocket;

WebSocket *createWebSocketWithClient(Client *client);
void connectWebSocket(WebSocket *socket, const char *host, const char *path);


#endif /* Client_h */
