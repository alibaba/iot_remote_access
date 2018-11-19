#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

typedef struct{
    char *cloud_ip;
    char *cloud_port;  
    char *cert_path;
    char *pk;
    char *dn;
    char *ds;
    char *listen_ip;
    unsigned int listen_port;
    unsigned char is_tls_on;
    unsigned char is_debug_on;
}CFG_STARTUP;

#define CFG_FILE_NAME   "remote_terminal.json"

CFG_STARTUP *init_cfg(char *pk, char *dn, char *ds);

void deinit_cfg();

#endif

