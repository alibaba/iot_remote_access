#include <time.h>
#include <errno.h>
#include "net_protocol.h"
#include "session_mgr.h"
#include "hash_table.h"
#include "misc.h"
#include "simple_log.h"

#define DEFAULT_SESSION_LEN 64
#define DEFAULT_SESSION_TIMEOUT 30*60

static void *g_ht = NULL;
static int  g_session_count = 0;

typedef struct{
    int *sockfd;
    time_t last_modify_time;
}SESSION_ITEM;

int init_session_system()
{
    if(g_ht != NULL)
        return 0;

    g_ht = ht_init(DEFAULT_SESSION_COUNT * 3);
    if(g_ht == NULL){
        log_error("failed to init session system: %s", strerror(errno)); 
        return -1;
    }
    g_session_count = 0;
    return 0;
}

void deinit_session_system()
{
    if(g_ht == NULL)
        return;
    ht_destroy(g_ht);
    g_session_count = 0;
    g_ht = NULL;
}

static int _delete_item(void *key, void *val, void *usr)
{
    SESSION_ITEM  *item = val; 

    if(item->last_modify_time + DEFAULT_SESSION_TIMEOUT < time(NULL)){
        log_info("delete timeout session:  %s ", (char *)key);
        *(int *)usr += 1; 
        *item->sockfd = -1;
        free_session(key);
    }

    return 1;
}

static int clear_timeout_session()
{
    int count = 0;

    if(g_ht == NULL){
        log_error("session system is not ready, please retry ");
    }

    ht_iterator_lockless(g_ht, _delete_item, &count);

    return count;
}

int is_session_avalibe()
{
    if(g_session_count >=  DEFAULT_SESSION_COUNT){
        //clear timeout session 
        return clear_timeout_session();
    }    

    return 1;
}

char *alloc_new_session(int *socketfd)
{
    char *session_id = NULL; 
    int ret = 0;
    SESSION_ITEM  *item = NULL; 

    if(g_session_count >= DEFAULT_SESSION_COUNT){
        log_error("session is full ");
        return NULL;    
    }

    if(g_ht == NULL){
        log_error("session system is not ready, please retry ");
        return NULL;
    }

    item = malloc(sizeof(SESSION_ITEM));
    if(item == NULL){
        log_error("no memory to alloc space to store session item "); 
        return NULL;
    }
    item->sockfd = socketfd; 
    item->last_modify_time = 0;

    session_id = rand_string_alloc(DEFAULT_SESSION_LEN);
    if(session_id == NULL){
        log_error("failed to alloc new session for socketfd: %d ", *socketfd);
        free(item);
        return NULL;
    }

    ret = ht_add_lockless(g_ht, session_id, DEFAULT_SESSION_LEN, item, sizeof(SESSION_ITEM));
    if(ret != 0){
        log_error("failed to add mapping for socketfd: %d ", *socketfd);
        free(session_id);
        free(item);
        return NULL; 
    }
    log_debug("session id: %s, socketfd: %d\n", session_id, *socketfd);
    ++g_session_count;
    free(item);
    return session_id;
}

int _get_timestamp_by_session_id(char *session_id)
{
    SESSION_ITEM  *item = NULL; 
    
    if(g_ht == NULL || session_id == NULL){
        log_error("session system is not ready, please retry ");
        return -1;
    }

    item = ht_find(g_ht, session_id, DEFAULT_SESSION_LEN, NULL, NULL);
    if(item == NULL){
        log_error("could not find socketfd for session: %s ", session_id);
        return -1;
    }
    
    return item->last_modify_time;
}

void free_session(char *session_id)
{
    if(g_ht == NULL){
        log_error("session system is not ready, please retry ");
        return ;
    }
    if(_get_timestamp_by_session_id(session_id) >= 0){
        ht_del_lockless(g_ht, session_id, DEFAULT_SESSION_LEN);
    
        --g_session_count;
        log_debug("current session count: %d\n", g_session_count);
    }
}

static int _get_socketfd(void *key, void *val, void *usr)
{
    SESSION_ITEM  *item = val; 
    int *tmp = usr;
   
    //log_debug("%p, fd: %d\n", key, item->sockfd);
    if(*item->sockfd == *tmp){
   
        item->last_modify_time = time(NULL);
        return 0;
    }

    return 1;
}

char *get_session_id_by_socketfd(int *socketfd)
{
    char *ret = NULL;

    if(g_ht == NULL){
        log_error("session system is not ready, please retry ");
        return 0;
    }

    ret = ht_iterator_lockless(g_ht, _get_socketfd, socketfd);

    log_debug("get session id: %s by socketfd: %d\n", ret, *socketfd);
    return ret;
}

int get_socketfd_by_session_id(char *session_id)
{
    SESSION_ITEM  *item = NULL; 
    
    if(g_ht == NULL || session_id == NULL){
        log_error("session system is not ready, please retry ");
        return -1;
    }

    item = ht_find(g_ht, session_id, DEFAULT_SESSION_LEN, NULL, NULL);
    if(item == NULL){
        log_error("could not find socketfd for session: %s ", session_id);
        return -1;
    }
    
    log_debug("get socketfd: %d by session id: %s \n", *item->sockfd, session_id);
    item->last_modify_time = time(NULL);
    return *item->sockfd;
}

