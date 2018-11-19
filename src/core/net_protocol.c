#include <time.h>
#include "net_protocol.h"
#include "simple_log.h"
#include "hmac-sha256.h"
#include "misc.h"

static char *g_uuid = "alibaba_iot";
char *sda_gen_msg_header(int msg_type, int payload_len, char *msg_id, char *token)
{
    static char msg[DEFAULT_MSG_HDR_LEN] = {0};

    memset(msg, 0, sizeof(msg));
    snprintf(msg, sizeof(msg) - 1, MSG_HEAD_FMT,
                    msg_type, payload_len, msg_id == NULL ? rand_string_static() : msg_id,
                    time(NULL), token == NULL ? "" : token);

    log_debug("send msg : %s\n", msg);
    return msg;
}

char *sda_gen_payload_msg(int code, char *data, char *msg)
{
    static char _msg[DEFAULT_MSG_HDR_LEN] = {0};

    memset(_msg, 0, sizeof(_msg));
    snprintf(_msg, sizeof(_msg) - 1, MSG_RESPONSE_FMT, code, !msg ? "null" : msg, !data ? "null" : data);
    
    return _msg;
}

#define HMAC_SHA256_BYTES 32
static char *calc_sign(char *uuid, char *dn, char *pk, char *ds)
{
    static char *format = "clientId%sdeviceName%sproductKey%stimestamp%lu";
    static unsigned char msg[DEFAULT_MSG_HDR_LEN] = {0};
    uint8_t mac[HMAC_SHA256_BYTES];
    static char ret[DEFAULT_MSG_HDR_LEN] = {0};

    memset(msg, 0, sizeof(msg));
    memset(mac, 0, sizeof(mac));
    memset(ret, 0, sizeof(ret));

    snprintf((char *)msg, sizeof(msg), format, uuid, dn, pk, time(NULL));
    log_debug("sign string is :  %s", msg);
    hmac_sha256(mac, msg, strlen((char *)msg), (unsigned char *)ds, strlen((char *)ds) + 1);
    int j = 0; 
    for (j = 0; j < HMAC_SHA256_BYTES; j++)
        sprintf(ret + strlen(ret), "%02x", mac[j]);
    
    return ret;
}

char *sda_gen_handshake_msg(char *pk, char *dn, char *ds)
{
    static char msg[DEFAULT_MSG_HDR_LEN] = {0};
    char ipaddr[256] = {0};

    //FIXME
    //get_dev_ip(ipaddr, sizeof(ipaddr));

    memset(msg,0,sizeof(msg));
    snprintf(msg, sizeof(msg), MSG_HDSK_FMT, 
                    g_uuid, pk, dn, "version_", 
                    ipaddr, "MAC_", "", calc_sign(g_uuid, dn, pk, ds));

    return msg; 
}


