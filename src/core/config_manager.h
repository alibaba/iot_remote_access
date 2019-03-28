#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#define DEFAULT_LEN_SERVICE_NAME 32
#define DEFAULT_LEN_SERVICE_TYPE 16 
#define DEFAULT_LEN_IP           16
//max port is 65535
#define DEFAULT_LEN_PORT         5 
#define CFG_FILE_NAME   "remote_terminal.json"

struct service_info {
    char                 type[DEFAULT_LEN_SERVICE_TYPE];
    char                 name[DEFAULT_LEN_SERVICE_NAME];
    char                 ip[DEFAULT_LEN_IP];
    unsigned             port;
    struct service_info *next;
};

typedef struct{
    char                *cloud_ip;
    char                *cloud_port;  
    char                *pk;
    char                *dn;
    char                *ds;
    unsigned char       is_tls_on;
    unsigned char       is_debug_on;
    struct service_info *service;
    unsigned int        service_count;
}CFG_STARTUP;

CFG_STARTUP *init_cfg(char *pk, char *dn, char *ds);

void deinit_cfg();

#endif

