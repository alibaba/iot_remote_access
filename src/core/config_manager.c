#include <errno.h>
#include <unistd.h>

#include "json_parser.h"
#include "simple_log.h"
#include "config_manager.h"

static CFG_STARTUP *g_cfg = NULL;

static void _dump_cfg()
{
    struct service_info *p = NULL;
    int i = 1;

	if(g_cfg == NULL)
		return;

	log_info("cloud ip: %s", g_cfg->cloud_ip);
	log_info("cloud port: %s", g_cfg->cloud_port);
	log_info("cert path: %s", g_cfg->cert_path);
	log_info("tls switch: %d", g_cfg->is_tls_on);
	log_info("support service count: %d", g_cfg->service_count);
    p = g_cfg->service;
    while(p){
        log_info("[%d]: service name: %s",i, p->name);
        log_info("[%d]: service ip: %s", i, p->ip);
        log_info("[%d]: service port: %d", i, p->port);
        p = p->next;
        i += 1;
    }
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

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
static char *_get_current_work_dir()
{
	char *buff = NULL;
    char *tmp = NULL;
	
	buff = malloc(FILENAME_MAX);
	if(buff == NULL){
        log_error("faild to alloc memory to save abs path ");
		exit(1);
	}
	memset(buff, 0, FILENAME_MAX);
#ifdef __APPLE__
  uint32_t size = FILENAME_MAX;
  if (_NSGetExecutablePath(buff, &size) != 0) {
    // Buffer size is too small.
    log_error("faild to get current directory");
    exit(1);
  }
  log_info("current directory: %s\n", buff);

  tmp = strrchr(buff, '/');
  if(tmp){
      if(buff[tmp - buff - 1] == '.')
          buff[tmp - buff - 1] = '\0';
      else
          buff[tmp - buff] = '\0';
  }
#else     
    int read_len = 0;
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
#endif    
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

static void _add_new_service_info(char *name, char *type, char *ip, unsigned int port)
{
    struct service_info *p = NULL;
    struct service_info *last_item = NULL;
    struct service_info *new_item = NULL;
    
    if(!g_cfg || !name || !ip || port == 0)
        return;

    p = g_cfg->service;

    while(p){
        if(strcmp(p->name, name) == 0 && strcmp(p->type, type) == 0 
                && strcmp(p->ip, ip) == 0 && p->port == port){
            log_info("repeated items, name: %s, ip: %s, port: %d", name, ip, port); 
            return;
        }
        last_item = p;
        p = p->next;
    }

    if(last_item == g_cfg->service && strlen(last_item->name) == 0){
        last_item->port = port;
        strncpy(last_item->name, name, sizeof(last_item->name) - 1);
        strncpy(last_item->type, type, sizeof(last_item->type) - 1);
        strncpy(last_item->ip, ip, sizeof(last_item->ip) - 1);
        last_item->next = NULL;
    } else {
        new_item = malloc(sizeof(struct service_info));
        if(new_item == NULL){
            log_error("failed to alloc memory for store service info."); 
            exit(1);
        } 
        memset(new_item, 0, sizeof(struct service_info));
        new_item->next = NULL;
        new_item->port = port;
        strncpy(new_item->name, name, sizeof(new_item->name) - 1);
        strncpy(new_item->type, type, sizeof(new_item->type) - 1);
        strncpy(new_item->ip, ip, sizeof(new_item->ip) - 1);

        last_item->next = new_item;
    }
    g_cfg->service_count += 1;
}

#define GET_SERVICE_INFO(info) do { \
        char *_name = NULL; \
        char *_type = NULL; \
        char *_ip = NULL; \
        unsigned int _port = 0; \
        tmp = json_get_value_by_name(info, strlen(info), "name", &len_val, &type_val); \
    	if(tmp && type_val == JSTRING && len_val > 0) {\
			_name = strndup(tmp, len_val); \
			tmp = NULL; \
        } else { \
            log_error("failed to read service name from config file"); \
            exit(1); \
		} \
        tmp = json_get_value_by_name(info, strlen(info), "type", &len_val, &type_val); \
    	if(tmp && type_val == JSTRING && len_val > 0) {\
			_type = strndup(tmp, len_val); \
			tmp = NULL; \
        } else { \
            log_error("failed to read service type from config file"); \
            exit(1); \
		} \
        tmp = json_get_value_by_name(info, strlen(info), "ip", &len_val, &type_val); \
    	if(tmp && type_val == JSTRING && len_val > 0) {\
			_ip = strndup(tmp, len_val); \
			tmp = NULL; \
        } else { \
            log_error("failed to read service ip from config file"); \
            exit(1); \
		} \
        \
        tmp = json_get_value_by_name(info, strlen(info), "port", &len_val, &type_val); \
    	if(tmp && type_val == JNUMBER && len_val > 0) {\
			_port = atoi(tmp); \
            if(_port > 65535){ \
                log_error("service port must <= 65535"); \
                exit(1); \
            }\
			tmp = NULL; \
        } else { \
            log_error("failed to read service port from config file"); \
            exit(1); \
		} \
        _add_new_service_info(_name, _type, _ip, _port); \
        free(_name); \
        free(_type); \
        free(_ip); \
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
        exit(1);
    }

	working_dir = _get_current_work_dir();

	cfg_json = _load_cfg_from_json(working_dir, CFG_FILE_NAME);
	
	GET_STRING_VAL("cloud_ip", &g_cfg->cloud_ip);
	GET_STRING_VAL("cloud_port", &g_cfg->cloud_port);
	GET_STRING_VAL("cert_path", &g_cfg->cert_path);
	GET_INT_VAL("is_tls_on", &g_cfg->is_tls_on);
	GET_INT_VAL("is_debug_on", &g_cfg->is_debug_on);

    g_cfg->service = malloc(sizeof(struct service_info));
    if(g_cfg->service == NULL){
        log_error("failed to alloc memory for store service info."); 
        exit(1);
    } 
    memset(g_cfg->service, 0, sizeof(struct service_info));
    g_cfg->service->next = NULL;
    g_cfg->service_count = 0;
    
    len_val = 0;
    type_val = 0;
    tmp = json_get_value_by_name(cfg_json, strlen(cfg_json), "services", &len_val, &type_val);
    if(!tmp || type_val != JARRAY || len_val <= 0){
        log_error("json format error in services field, must be json array type."); 
        exit(1);
    }
    char *list = NULL;
    char *pos = NULL;
    char *entry = NULL;
    list = strndup(tmp, len_val);
    tmp = NULL;
    len_val = 0;
    type_val = 0;

    json_array_for_each_entry(list, pos, entry, len_val, type_val){
        if(type_val != JOBJECT)
            continue;
        GET_SERVICE_INFO(entry);
    }

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
  
    free(list);
    free(working_dir);
	free(cfg_json);

	_dump_cfg();
	return g_cfg;
}

void deinit_cfg()
{
    struct service_info  *p = NULL;
    struct service_info  *tmp = NULL;
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

    p = g_cfg->service;
    while(p){
        tmp = p->next;
        free(p);
        p = tmp;
    }
    free(g_cfg);
    g_cfg = NULL;
	log_info("deinit cfg ok.");
}
