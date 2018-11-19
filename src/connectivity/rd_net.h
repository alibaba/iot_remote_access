#ifndef RD_NET_H
#define RD_NET_H
#include "ws_nopoll.h"

typedef enum {
    RD_NET_WEBSOCKET = 0,
    RD_NET_MQTT,
    RD_NET_WSF,
} RD_NET_TYPE_T;

struct Network;
typedef struct Network Network_t;
typedef struct Network *pNetwork_t;

typedef int (*ActionHandler) (void *ctx, void *conn, void *user_data);

typedef void (*MessageHandler) (void *ctx, void *conn, void *msg, void *user_data);

struct Network {
    RD_NET_TYPE_T type;
    const char *host;
    const char *port;
    const char *get_url;    
    const char *ca_crt;
    const char *pri_key;    
    int tls_on;
	int debug_on;
    void *handle;
};

int rd_net_read(pNetwork_t pNetwork, void *connect, char *buffer, int len, int timeout_ms);
int rd_net_write(pNetwork_t pNetwork, void *connect, const char *buffer, int len, int timeout_ms);
int rd_net_get_conn_info (pNetwork_t pNetwork, void *connect, ConnectInfo_t *connInfo);
int rd_net_destroy(pNetwork_t pNetwork);
int rd_net_connect(pNetwork_t pNetwork);
int rd_net_listener(pNetwork_t pNetwork);
int rd_net_set_on_msg(pNetwork_t pNetwork, MessageHandler on_message);
int rd_net_set_on_ready(pNetwork_t pNetwork, ActionHandler on_ready);
int rd_net_loop_wait (pNetwork_t pNetwork);
void *rd_net_accept(pNetwork_t pNetwork);
int rd_net_init(pNetwork_t pNetwork, RD_NET_TYPE_T type, const int debug_on,
                  const char *host, const char *port, const char *get_url,
                  const int tls_on, const char *ca_crt, const char *pri_key);

#endif /* RD_NET_H */

