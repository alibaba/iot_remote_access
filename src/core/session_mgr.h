#ifndef _SESSION_MGR_H__
#define _SESSION_MGR_H__

#define DEFAULT_SESSION_COUNT 32

int init_session_system();

void deinit_session_system();

char *alloc_new_session(int *socketfd);

void free_session(char *session_id);

char *get_session_id_by_socketfd(int *socketfd);

int get_socketfd_by_session_id(char *session_id);

int is_session_avalibe();

#endif

