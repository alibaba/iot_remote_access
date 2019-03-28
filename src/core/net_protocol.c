#include <time.h>
#include "net_protocol.h"
#include "simple_log.h"
#include "hmac-sha256.h"
#include "misc.h"

static char *g_uuid = "alibaba_iot";

extern CFG_STARTUP *g_cfg;

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

#define FMT_SERVICE_INFO "{\"service_type\":\"%s\",\"service_name\":\"%s\",\"service_ip\":\"%s\",\"service_port\":%d}"
static char *generate_service_supported()
{
    int buf_len = 0;
    int service_count = g_cfg->service_count > 0 ? g_cfg->service_count : 1;
    char *buf = NULL;
    struct service_info *tmp = NULL;
    int i = 0;

    //format lenght: [{},{},{}]
    buf_len = sizeof("[]") + 
                        service_count*(
                                strlen(FMT_SERVICE_INFO) + 
                                DEFAULT_LEN_SERVICE_NAME + 
                                DEFAULT_LEN_SERVICE_TYPE + 
                                DEFAULT_LEN_IP + 
                                DEFAULT_LEN_PORT + 
                                sizeof(",")) + 1;
    buf = malloc(buf_len);
    if(!buf)
        return NULL;

    memset(buf, 0, buf_len);
    
    buf[0] = '[';

    if(g_cfg->service_count == 0){
        //default service info.
        snprintf(buf+strlen(buf), buf_len - strlen(buf), FMT_SERVICE_INFO, "SSH", "ssh_localhost", "127.0.0.1", 22); 
    } else {
        tmp = g_cfg->service;
        while(tmp){
            snprintf(buf+strlen(buf), buf_len - strlen(buf), FMT_SERVICE_INFO, tmp->type, tmp->name, tmp->ip, tmp->port);
            i++;
            if(i != g_cfg->service_count)
                strcat(buf, ",");
            tmp = tmp->next;
        }
    }
    
    buf[strlen(buf)] = ']';
 
    return buf;
}

char *sda_gen_handshake_msg(char *pk, char *dn, char *ds)
{
    char *msg = NULL;
    int msg_len = 0; 
    char *service_meta = NULL;
    char *local_ip = "";
    char *local_mac = "";
    char *sign = NULL;

    service_meta = generate_service_supported();
    sign = calc_sign(g_uuid, dn, pk, ds);

    if(!service_meta || !sign){
        log_error("failed to get service meta data or sign");
        return NULL;
    }

    msg_len = strlen(MSG_HDSK_FMT) + strlen(g_uuid) + strlen(pk) + strlen(dn) + strlen(VERSION) + 
                    strlen(local_ip) + strlen(local_mac) + strlen(service_meta) + strlen(sign) + 1; 

    msg = malloc(msg_len);
    if(!msg){
        free(service_meta); 
        return NULL;
    }

    memset(msg, 0, msg_len);
    snprintf(msg, msg_len, MSG_HDSK_FMT, 
                    g_uuid, pk, dn, VERSION, 
                    local_ip, local_mac, service_meta, sign);

    log_debug("handshake payload:  %s", msg);
    free(service_meta);
    return msg; 
}
