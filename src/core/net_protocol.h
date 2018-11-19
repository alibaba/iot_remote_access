#ifndef _NET_PROTOCOL_H__
#define _NET_PROTOCOL_H__

#define MSG_HEAD_FMT "{\"msg_type\": %d,\"payload_len\": %d,\"msg_id\": \"%s\",\"timestamp\":%ld,\"token\":\"%s\"}\r\n\r\n"
#define MSG_HDSK_FMT "{\"uuid\": \"%s\",\"product_key\": \"%s\",\"device_name\": \"%s\",\"version\":\"%s\",\"IP\":\"%s\",\"MAC\":\"%s\",\"token\":\"%s\", \"service_supported\": \"ssh\",\"signmethod\": \"hmacsha256\", \"sign\": \"%s\"}"
#define MSG_RESPONSE_FMT "{\"code\": %d, \"message\": \"%s\"}\r\n\r\n%s"

typedef enum{
    MSG_DBG_SWITCH                      = 1,            //tunnel功能开关.
    MSG_SERVICE_PROVIDER_CONN_REQ       = 2,            //服务提供者发起（握手）连接请求.
    MSG_SERVICE_CONSUMER_CONN_REQ       = 3,            //服务消费者发起（握手）连接请求.
    MSG_SERVICE_PROVIDER_RAW_PROTOCOL   = 4,            //服务提供者发送的原始服务协议.
    MSG_SERVICE_CONSUMER_RAW_PROTOCOL   = 5,            //服务消费者发送的原始服务协议. 
    MSG_WEB_APPLY_DEV_LIST              = 6,            //前端向云端请求设备列表.
    MSG_CLOUD_DEV_LIST_RESP             = 7,            //云端向前端返回设备列表信息.
    MSG_SERVICE_CONSUMER_NEW_SESSION    = 8,            //新增session
    MSG_SERVICE_CONSUMER_RELEASE_SESSION = 9,           //释放session
    MSG_SESSION_HEART_BEAT              = 10,           // session 保活心跳
    MSG_RESP_OK                         = 255           //消息的response

}RemoteTerminalMsgType;

typedef enum {
    SERVICE_TYPE_SSH                = 1,                //SSH 服务,端口默认 22
    SERVICE_TYPE_HTTP               = 2,                //HTTP 服务，端口默认 80
    SERVICE_TYPE_REMOTE_DESKTOP     = 3,                //windows远程桌面服务，端口默认 3389
    SERVICE_TYPE_FTP                = 4,                //FTP服务， 默认端口为21
    SERVICE_TYPE_OPENAPI            = 5,                //eweb和openApi 服务， 默认端口 9999
    SERVICE_TYPE_TELNET             = 6,                //telnet 服务， 默认端口 23
    SERVICE_TYPE_NONE               = 255               //无服务
}RemoteTerminalServiceType;

typedef enum{
    ERR_SUCCESS                     = 0,                //成功 
    ERR_SIGNATURE_INVALID           = 101600,           //签名验证失败 
    ERR_PARAM_INVALID               = 101601,           //入参不合法 
    ERR_SESSION_LIMIT               = 101602,           //Session已达最大值
    ERR_SESSION_NONEXISTENT         = 101603,           //Session不存在
    ERR_SESSION_CREATE_FAILED       = 101604,           //Session创建失败
    ERR_SERVICE_UNAVALIBE           = 101604,           //服务不可达
    ERR_SERVICE_EXIT                = 101605,           //服务异常退出
    ERR_CONNECTION_CLOSE            = 101606            //连接异常退出
}RemoteTerminalErrorType;

typedef struct{
   	RemoteTerminalMsgType 		msg_type;
	RemoteTerminalServiceType 	srv_type;
	unsigned int 			  	payload_len; 
	char						msg_id[64];
	unsigned long long			timestamp;
	char						token[64];
    unsigned int                hdr_len;
}RemoteTerminalMsgHeader;

typedef struct{
    char                        uuid[32];
    char                        pk[16];
    char                        dn[32];
    char                        version[16];
    char                        ip[16];         //FIXME
    char                        mac[32];        //FIXME
    char                        token[32];      //FIXME
    char                        service_supported[32];
    char                        sign_method[32];
    char                        sign[256];
}HandshakeMsg;

#define DEFAULT_MSG_HDR_LEN 1024

char *sda_gen_msg_header(int msg_type, int payload_len, char *msg_id, char *token);

char *sda_gen_payload_msg(int code, char *data, char *msg);

char *sda_gen_handshake_msg(char *pk, char *dn, char *ds);

#endif

