#include <string.h>
#include "rd_net.h"
#include "simple_log.h"

int rd_net_read(pNetwork_t pNetwork, void *connect, char *buffer, int len, int timeout_ms)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        if (NULL != connect) {
            return ws_nopoll_read(connect, buffer, len, timeout_ms);
        } else {
            return ws_nopoll_read(pNetwork->handle, buffer, len, timeout_ms);
        }
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_write(pNetwork_t pNetwork, void *connect, const char *buffer, int len, int timeout_ms)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        if (NULL != connect) {
            return ws_nopoll_write(connect, buffer, len, timeout_ms);
        } else {
            return ws_nopoll_write(pNetwork->handle, buffer, len, timeout_ms);
        }
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_destroy(pNetwork_t pNetwork)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        return ws_nopoll_destroy(pNetwork->handle);
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_connect(pNetwork_t pNetwork)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        pNetwork->handle = (void *)ws_nopoll_connect(pNetwork->debug_on, pNetwork->host, pNetwork->port, 
                                pNetwork->get_url, pNetwork->tls_on, pNetwork->ca_crt);
        if (NULL != pNetwork->handle) {
            return 0;
        }
        return -1;
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_listener(pNetwork_t pNetwork)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        pNetwork->handle = (void *)ws_nopoll_listener(pNetwork->debug_on, pNetwork->host, pNetwork->port, 
                                pNetwork->tls_on, pNetwork->ca_crt, pNetwork->pri_key);
        if (NULL != pNetwork->handle) {
            return 0;
        }
        return -1;
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_set_on_msg(pNetwork_t pNetwork, MessageHandler on_message)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        return ws_nopoll_set_on_msg(pNetwork->handle, (noPollOnMessageHandler)on_message);
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_set_on_ready(pNetwork_t pNetwork, ActionHandler on_ready)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        return ws_nopoll_set_on_ready(pNetwork->handle, (noPollActionHandler)on_ready);
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

void *rd_net_accept(pNetwork_t pNetwork)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        return ws_nopoll_accept(pNetwork->handle);
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return NULL;
    }
}

int rd_net_get_conn_info (pNetwork_t pNetwork, void *connect, ConnectInfo_t *connInfo)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        if (NULL != connect) {
            return ws_nopoll_get_conn_info(connect, connInfo);
        } else {
            return ws_nopoll_get_conn_info(pNetwork->handle, connInfo);
        }
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_loop_wait (pNetwork_t pNetwork)
{
    if (RD_NET_WEBSOCKET == pNetwork->type) {
        return ws_nopoll_loop_wait(pNetwork->handle);
    } else {
        log_debug("unsupport connect type: %d ", pNetwork->type);
        return -1;
    }
}

int rd_net_init(pNetwork_t pNetwork, RD_NET_TYPE_T type, const int debug_on, 
                  const char *host, const char *port, const char *get_url,
                  const int tls_on, const char *ca_crt, const char *pri_key)
{
    pNetwork->type = type;
    pNetwork->debug_on = debug_on;
    pNetwork->host = host;
    pNetwork->port = port;
    pNetwork->get_url = get_url;
    pNetwork->tls_on = tls_on;
    pNetwork->ca_crt = ca_crt;
    pNetwork->pri_key = pri_key;
    pNetwork->handle = NULL;
    return 0;
}

