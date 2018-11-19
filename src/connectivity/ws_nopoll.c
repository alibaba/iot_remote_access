#include "ws_nopoll.h"
#include "simple_log.h"

int ws_nopoll_read(void *handle, char *buffer, int len, int timeout_ms)
{
    noPollConn *conn = (noPollConn *)handle;
	int bytes_read = 0;

    #if 1
	bytes_read = nopoll_conn_read(conn, buffer, len, nopoll_false, 100);
	#else
	noPollMsg *msg = NULL;
	int size = 0;

	msg = nopoll_conn_get_msg(conn);
	if (NULL != msg) {
        size = nopoll_msg_get_payload_size(msg);
        if (size >= len) {
            size = len - 1;
        }
        if (size > 0) {
            memset(buffer, 0x0, len);
            memcpy(buffer, nopoll_msg_get_payload(msg), size);
        }
	}
	bytes_read = size;
	#endif
	return bytes_read;
}

int ws_nopoll_write(void *handle, const char *buffer, int len, int timeout_ms)
{
    noPollConn *conn = (noPollConn *)handle;
    int tries = 0;
    int bytes_written = 0;

    if ((NULL != buffer) && (len > 0)) {
        bytes_written = nopoll_conn_send_binary (conn, buffer, len);
        if (bytes_written == len) {
            //log_debug ("send all. ");
            return bytes_written;
        }

        while (tries < 5 && errno == NOPOLL_EWOULDBLOCK 
          && nopoll_conn_pending_write_bytes (conn) > 0) {
            nopoll_sleep (10000); 
            if (0 == nopoll_conn_complete_pending_write (conn)) {
                log_debug ("send complete. ");
                return len;
            }
            tries++;
        }
        log_debug ("send len: %d, try: %d. ", bytes_written, tries);
        return  bytes_written;
    } else {
        log_debug ("ERROR: content is NULL or len <= 0 ");
    }
    return -1;
}

int ws_nopoll_destroy(void *handle)
{
    noPollConn *conn = (noPollConn *)handle;
    noPollCtx *ctx = NULL;

	/* finish connection */
	if (NULL != conn) {
        ctx = nopoll_conn_ctx(conn);
    	nopoll_conn_close(conn);
	}
	
	/* finish */
	if (NULL != ctx) {
    	nopoll_loop_stop(ctx);
        nopoll_ctx_unref(ctx);
	}

	/* call to cleanup */
	nopoll_cleanup_library ();

	return 0;
}

noPollConn *ws_nopoll_connect(const int debug_on, const char *host, const char *port, const char *get_url, const int tls_on, const char *ca_crt)
{
    noPollCtx  *ctx = NULL;
    noPollConn *conn = NULL;
	noPollConnOpts *opts = NULL;
    nopoll_bool debug = (nopoll_bool)debug_on;
    int i = 0;

    if (NULL == host || NULL == port) {
        log_debug("host or port is NULL ");
        return NULL;
    }

    /* create context */
    ctx = nopoll_ctx_new ();
    nopoll_log_enable (ctx, debug);
    nopoll_log_color_enable (ctx, debug);

    /* check connections registered */
    if (nopoll_ctx_conns(ctx) != 0) {
        log_debug("ERROR: expected to find 0 registered connections but found ");
        return NULL;
    }

    /* call to create a connection */
    if (1 == tls_on) {
    	opts = nopoll_conn_opts_new ();
    	if (NULL == ca_crt) {
            /* disable verification */
            log_debug("set peer verify false ");
            nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);
    	} else {
            nopoll_conn_opts_set_ssl_certs (opts, NULL, NULL, NULL, ca_crt);
    	}

        conn = nopoll_conn_tls_new (ctx, opts, host, port, NULL, get_url, NULL, NULL);
        if (!nopoll_conn_is_ok (conn)) {
            log_debug("ERROR: Expected to find proper client connection status, but found error.. ");
            nopoll_ctx_unref(ctx);
            return NULL;
        }

        if (!nopoll_conn_is_tls_on (conn)) {
            log_debug("ERROR: expected to find TLS enabled on the connection but found it isn't.. ");
            nopoll_ctx_unref(ctx);
            return NULL;
        }    
    } else {
        conn = nopoll_conn_new (ctx, host, port, NULL, get_url, NULL, NULL);
        if (!nopoll_conn_is_ok (conn)) {
            log_debug("ERROR: Expected to find proper client connection status, but found error.. ");
            nopoll_ctx_unref(ctx);
            return NULL;
        }
    }

    /* wait for ready */
    while (nopoll_true) {
        if (nopoll_conn_is_ready(conn)) {
            break;
        }
        i++;
        if (i > 100) {
            log_debug("ERROR: connection not ready ");
            nopoll_conn_close(conn);
            nopoll_ctx_unref(ctx);
            return NULL;
        }
        nopoll_sleep (100000);
    }

    return conn;
}

noPollConn *ws_nopoll_listener(const int debug_on, const char *host, const char *port, const int tls_on, const char *server_crt, const char *private_key)
{
    noPollCtx  *ctx = NULL;
	noPollConn *listener = NULL;
    nopoll_bool debug = (nopoll_bool)debug_on;

    if (NULL == host || NULL == port) {
        log_debug("host or port is NULL ");
        return NULL;
    }

    /* create context */
    ctx = nopoll_ctx_new ();
    nopoll_log_enable (ctx, debug);
    nopoll_log_color_enable (ctx, debug);

    if (1 == tls_on) {
    	/* now start a TLS version */
    	listener = nopoll_listener_tls_new (ctx, host, port);
    	if (!nopoll_conn_is_ok (listener)) {
    		log_debug("ERROR: Expected to find proper listener TLS connection status, but found.. ");
    		nopoll_ctx_unref(ctx);
    		return NULL;
    	}

    	/* configure certificates to be used by this listener */
    	if (!nopoll_listener_set_certificate (listener, server_crt, private_key, NULL)) {
    		log_debug("ERROR: unable to configure certificates for TLS websocket.. ");
            nopoll_conn_close (listener);
            nopoll_ctx_unref(ctx);
    		return NULL;
    	}

    	/* register certificates at context level */
    	if (! nopoll_ctx_set_certificate (ctx, NULL, server_crt, private_key, NULL)) {
    		log_debug("ERROR: unable to setup certificates at context level.. ");
            nopoll_conn_close (listener);
            nopoll_ctx_unref(ctx);
    		return NULL;
    	}
	} else {
        listener = nopoll_listener_new (ctx, host, port);
        if (!nopoll_conn_is_ok (listener)) {
            log_debug ("ERROR: Expected to find proper listener connection status, but found.. ");
    		nopoll_ctx_unref(ctx);
            return NULL;
        }
	}

    return listener;
}

int ws_nopoll_loop_wait(void *handle)
{
    noPollConn *conn = (noPollConn *)handle;
    noPollCtx *ctx = NULL;

    if (NULL == conn) {
        log_debug("ERROR: conn is NULL ");
        return -1;
    }

    ctx = nopoll_conn_ctx(conn);
    if (NULL == ctx) {
        log_debug("ERROR: ctx is NULL ");
        return -1;
    }

    nopoll_loop_wait(ctx, 0);
	return 0;
}

int ws_nopoll_set_on_ready(void *handle, noPollActionHandler on_ready)
{
    noPollConn *conn = (noPollConn *)handle;
    noPollCtx *ctx = NULL;

    if (NULL == conn) {
        log_debug("ERROR: conn is NULL ");
        return -1;
    }

    ctx = nopoll_conn_ctx(conn);
    if (NULL == ctx) {
        log_debug("ERROR: ctx is NULL ");
        return -1;
    }

    /* set on connect ready */
	if (NULL != on_ready) {
    	nopoll_ctx_set_on_ready (ctx, on_ready, NULL);
	}

	return 0;
}

int ws_nopoll_set_on_msg(void *handle, noPollOnMessageHandler on_message)
{
    noPollConn *conn = (noPollConn *)handle;
    noPollCtx *ctx = NULL;

    if (NULL == conn) {
        log_debug("ERROR: conn is NULL ");
        return -1;
    }

    ctx = nopoll_conn_ctx(conn);
    if (NULL == ctx) {
        log_debug("ERROR: ctx is NULL ");
        return -1;
    }

	/* set on message received */
	if (NULL != on_message) {
    	nopoll_ctx_set_on_msg (ctx, on_message, NULL);
	}

	return 0;
}

noPollConn *ws_nopoll_accept(void *handle)
{
    noPollConn *master = (noPollConn *)handle;
    noPollConn *conn = NULL;
    noPollCtx *ctx = NULL;

    if (NULL == master) {
        log_debug("ERROR: master conn is NULL ");
        return NULL;
    }

    ctx = nopoll_conn_ctx(master);
    if (NULL == ctx) {
        log_debug("ERROR: ctx is NULL ");
        return NULL;
    }

    conn = nopoll_conn_accept(ctx, master);
	if (!nopoll_conn_is_ok (conn)) {
		log_debug("ERROR: expected to find proper listener status (connection accepted), but found failure.. ");
		return NULL;
	}

	return conn;
}

int ws_nopoll_get_conn_info(void *handle, ConnectInfo_t *connInfo)
{
    if (NULL == connInfo) {
        log_debug("ERROR: connInfo is NULL ");
        return -1;
    }

    noPollConn *conn = (noPollConn *)handle;
    if (NULL != conn) {
        connInfo->sockfd = (int) nopoll_conn_socket(conn);
        connInfo->host = nopoll_conn_host(conn);
        connInfo->port = nopoll_conn_port(conn);
        connInfo->origin = nopoll_conn_get_origin(conn);
        //printf("----> origin:  %s\n", connInfo->origin);
        connInfo->get_url = nopoll_conn_get_requested_url(conn);
    }

    return 0;
}

