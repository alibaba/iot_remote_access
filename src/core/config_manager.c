#include <errno.h>
#include <unistd.h>

#include "json_parser.h"
#include "simple_log.h"
#include "config_manager.h"

static CFG_STARTUP *g_cfg = NULL;

static void _dump_cfg()
{
	if(g_cfg == NULL)
		return;

	log_info("cloud ip: %s", g_cfg->cloud_ip);
	log_info("cloud port: %s", g_cfg->cloud_port);
	log_info("cert path: %s", g_cfg->cert_path);
	log_info("tls switch: %d", g_cfg->is_tls_on);
	log_info("listen ip: %s", g_cfg->listen_ip);
	log_info("listen port: %d", g_cfg->listen_port);
	if(g_cfg->pk)
		log_info("product key: %s", g_cfg->pk);
	if(g_cfg->dn)
		log_info("device name: %s", g_cfg->dn);
	if(g_cfg->ds)
		log_info("device secret: **********%s", g_cfg->ds + 10);//just print the last 22 character
}

static char *_load_cfg_from_json(char *path, char *name)
{
	char *content = NULL;
	char *file_name = NULL;
	FILE *fp = NULL;
	long file_len = 0;
	int tmp = 0;

	if(!name || !path)
		return NULL;

	tmp = strlen(path) + strlen(name) + 1 + 1;//add \0 and /
	file_name = malloc(tmp);
	if(file_name == NULL){
		log_error("failed to alloc memory to save abs path");	
		exit(1);
	}
	memset(file_name, 0, tmp);
	snprintf(file_name, tmp, "%s/%s", path, name);
	fp = fopen(file_name, "r");
	if(!fp){
		log_error("failed to open file: %s: %s", file_name, strerror(errno));
		free(file_name);
		exit(1);
	}

	fseek(fp, 0L, SEEK_SET);
	fseek(fp, 0L, SEEK_END);
	file_len = ftell(fp);
	if(file_len <= 0){
		log_error("file: %s is empty", file_name);
		fclose(fp);
		free(file_name);
		exit(1);
	}

    file_len += 1;
	content = malloc(file_len);
	if(content == NULL){
		log_error("failed to alloc memory to save config file content ");
		fclose(fp);
		free(file_name);
		exit(1);
	}
	memset(content, 0, file_len); 
	fseek(fp, 0L, SEEK_SET);
	if(fread(content, 1, file_len, fp) < file_len - 1){
        log_error("failed to read file content: %s\n", strerror(errno)); 
    }
	fclose(fp);
	free(file_name);
	return content;
}

static char *_get_current_work_dir()
{
	char *buff = NULL;
    int read_len = 0;
    char *tmp = NULL;
	
	buff = malloc(FILENAME_MAX);
	if(buff == NULL){
        log_error("faild to alloc memory to save abs path ");
		exit(1);
	}
	memset(buff, 0, FILENAME_MAX);
    read_len = readlink("/proc/self/exe", buff, FILENAME_MAX - 1);
    if(read_len <= 0){
        log_error("faild to read /proc ");
        exit(1);
    }
    buff[read_len] = '\0';
    tmp = strrchr(buff, '/');
    if(tmp){
        buff[tmp - buff] = '\0';
    }
	return buff;
}

#define GET_INT_VAL(name, p) do{ \
        tmp = json_get_value_by_name(cfg_json, strlen(cfg_json), name, &len_val, &type_val); \
    	if(tmp && type_val == JNUMBER && len_val > 0) { \
			*p = atoi(tmp); \
			tmp = NULL; \
        } else { \
			log_error("failed to read %s from config file", name); \
			exit(1); \
		} \
}while(0)

#define GET_STRING_VAL(name, p) do{ \
        tmp = json_get_value_by_name(cfg_json, strlen(cfg_json), name, &len_val, &type_val); \
    	if(tmp && type_val == JSTRING && len_val > 0) {\
			*p = strndup(tmp, len_val); \
			tmp = NULL; \
        } else { \
			if(! (strcmp(name, "product_key") == 0 || strcmp(name, "device_name") == 0 || strcmp(name, "device_secret") == 0)) {\
				log_error("failed to read %s from config file", name); \
				exit(1); \
			} \
		} \
}while(0)

CFG_STARTUP *init_cfg(char *pk, char *dn, char *ds)
{
	char *working_dir = NULL, *tmp = NULL;
	char *cfg_json = NULL;
	int len_val = 0;
	int type_val = 0;
    if(g_cfg != NULL){
        log_warning("configure is already exist."); 
        return g_cfg;
    }

    g_cfg = malloc(sizeof(CFG_STARTUP));
    if(g_cfg == NULL){
        log_error("failed to alloc memory for store configure info."); 
        return NULL;
    }

	working_dir = _get_current_work_dir();

	cfg_json = _load_cfg_from_json(working_dir, CFG_FILE_NAME);
	
	GET_STRING_VAL("cloud_ip", &g_cfg->cloud_ip);
	GET_STRING_VAL("cloud_port", &g_cfg->cloud_port);
	GET_STRING_VAL("cert_path", &g_cfg->cert_path);
	GET_STRING_VAL("listen_ip", &g_cfg->listen_ip);
	GET_INT_VAL("is_tls_on", &g_cfg->is_tls_on);
	GET_INT_VAL("listen_port", &g_cfg->listen_port);
	GET_INT_VAL("is_debug_on", &g_cfg->is_debug_on);

    if(pk == NULL)
        GET_STRING_VAL("product_key", &g_cfg->pk);
    else
        g_cfg->pk = strdup(pk);

    if(dn == NULL)
        GET_STRING_VAL("device_name", &g_cfg->dn);
    else
        g_cfg->dn = strdup(dn);

    if(ds == NULL)
        GET_STRING_VAL("device_secret", &g_cfg->ds);
    else
        g_cfg->ds = strdup(ds);
    free(working_dir);
	free(cfg_json);

	_dump_cfg();
	return g_cfg;
}

void deinit_cfg()
{
	if(g_cfg == NULL)
		return;

	free(g_cfg->cloud_ip);
	free(g_cfg->cloud_port);
	free(g_cfg->cert_path);
	if(g_cfg->pk)
		free(g_cfg->pk);
	if(g_cfg->dn)
		free(g_cfg->dn);
	if(g_cfg->ds)
		free(g_cfg->ds);

    free(g_cfg);
    g_cfg = NULL;
	log_info("deinit cfg ok.");
}
