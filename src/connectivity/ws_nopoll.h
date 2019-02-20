#include "nopoll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ConnectInfo {
    int sockfd;
    const char *host;
    const char *port;
	const char *origin;
	const char *get_url;
};

typedef struct ConnectInfo ConnectInfo_t;

int ws_nopoll_read(void *handle, char *buffer, int len, int timeout_ms);

int ws_nopoll_write(void *handle, const char *buffer, int len, int timeout_ms);

int ws_nopoll_destroy(void *handle);

noPollConn *ws_nopoll_connect(const int debug_on, const char *host, const char *port, const char *get_url, const int tls_on, const char *ca_crt);

noPollConn *ws_nopoll_listener(const int debug_on, const char *host, const char *port, const int tls_on, const char *server_crt, const char *private_key);

int ws_nopoll_set_on_ready(void *handle, noPollActionHandler on_ready);

int ws_nopoll_set_on_msg(void *handle, noPollOnMessageHandler on_message);

noPollConn *ws_nopoll_accept(void *handle);

int ws_nopoll_loop_wait(void *handle);

int ws_nopoll_get_conn_info(void *handle, ConnectInfo_t *connInfo);

