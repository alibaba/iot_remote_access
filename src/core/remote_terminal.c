#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "config_manager.h"
#include "rd_net.h"
#include "json_parser.h"
#include "misc.h"
#include "sha256.h"
#include "hmac-sha256.h"
#include "simple_log.h"
#include "net_protocol.h"
#include "session_mgr.h"

static CFG_STARTUP *g_cfg = NULL;
Network_t g_network;
static int  g_is_running = 1;

static void _sig_int_handler(int sig)
{
    if (SIGINT == sig) {
        if(g_is_running == 0){
            log_info("force exit !\r\n");
            exit(0); 
        }
        g_is_running = 0; 
        log_info( "Caught signal: %s, exiting..., %d\r\n", strsignal(sig), sig);
    }
}

#define DEFAULT_MSG_ID_HDSK "msg_id_for_handshake"
static int sda_connect_to_cloud (void)
{
    int ret = 0;
    char *msg_hdsk = NULL;
    char *msg_hdr = NULL;
    char buf[DEFAULT_MSG_HDR_LEN * 2] = {0};

    rd_net_init(&g_network, RD_NET_WEBSOCKET, g_cfg->is_debug_on == 1 ? 1 : 0,
                    g_cfg->cloud_ip, g_cfg->cloud_port, NULL, 
                    g_cfg->is_tls_on , g_cfg->cert_path, NULL);

    ret = rd_net_connect(&g_network);
    if (0 != ret) {
        log_error("connect error ");
        return ret;
    }

    msg_hdsk = sda_gen_handshake_msg(g_cfg->pk, g_cfg->dn, g_cfg->ds);
    msg_hdr =  sda_gen_msg_header(MSG_SERVICE_PROVIDER_CONN_REQ, strlen(msg_hdsk), DEFAULT_MSG_ID_HDSK, NULL);

    log_info("handshake header:%s", msg_hdr);
    log_info("handshak payload:%s", msg_hdsk);

    snprintf(buf,sizeof(buf),"%s%s",msg_hdr,msg_hdsk); 
    ret = rd_net_write(&g_network, NULL, buf, strlen(buf), 0);
    if (ret <= 0) {
        log_error("write data to cloud failed ");
        return -1;
    }

    log_info("send msg handshake, len:%d ", ret);

    return 0;
}

static int sda_connect_to_local_service (void)
{
    int ret = 0;
    int socketfd = -1;

    struct sockaddr_in server_sshd;

    memset(&server_sshd, 0, sizeof(struct sockaddr_in));
    server_sshd.sin_family = PF_INET;
    server_sshd.sin_port = htons(g_cfg->listen_port);
    server_sshd.sin_addr.s_addr = inet_addr(g_cfg->listen_ip);

    socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == socketfd) {
        log_error("create socket failed, %s ", strerror(errno));
        return -1;
    }

    ret = connect(socketfd, (struct sockaddr *)&server_sshd, sizeof(server_sshd));
    if (-1 == ret) {
        log_error("connect to failed, %s ", strerror(errno));
        close(socketfd);
        return -1;
    }
    log_info("connect to local service succeed, socketfd: %d. ", socketfd);
    return socketfd;
}

static inline int get_msg_header(char *buf, RemoteTerminalMsgHeader *hdr)
{
    int first = 0, second = 0, len = 0, hdr_len = 0;
    char *tmp = NULL, *ret = NULL, *hdr_start = NULL;

    if(!buf || !hdr)
        return 0;

    memset(hdr, 0, sizeof(RemoteTerminalMsgHeader));
    tmp = strchr(buf,'{');
    if(!tmp)
        goto _exit;
    first = tmp - buf; 

    tmp = strchr(buf, '}');
    if(!tmp)
        goto _exit;
    second = tmp - buf;

    hdr_start = buf + first;
    hdr_len = second - first + 1;

    ret = json_get_value_by_name(hdr_start, hdr_len, "msg_type", NULL, NULL);
    if(!ret)
        goto _exit;
    hdr->msg_type = atoi(ret);
    
    ret = json_get_value_by_name(hdr_start, hdr_len, "service_type", NULL, NULL);
    //service type is not a must
    if(ret){
        hdr->srv_type = atoi(ret);
    }
    
    ret = json_get_value_by_name(hdr_start, hdr_len, "payload_len", NULL, NULL);
    if(!ret)
        goto _exit;
    hdr->payload_len = atoi(ret);
   
    len = 0;
    ret = json_get_value_by_name(hdr_start, hdr_len, "msg_id", &len, NULL);
    //msg_id is not a must
    if(ret){
        strncpy(hdr->msg_id, ret, len < 63 ? len : 63);
    }
 
    ret = json_get_value_by_name(hdr_start, hdr_len, "timestamp", NULL, NULL);
    if(ret){
        hdr->timestamp = atol(ret);
    }
   
    len = 0;
    ret = json_get_value_by_name(hdr_start, hdr_len, "token", &len, NULL);
    if(ret){
        strncpy(hdr->token, ret, len < 63 ? len : 63);
    }
    
    hdr->hdr_len = second - first + sizeof("\r \r ");

    log_info("msg hander:  %s", buf); 
    return 1;
_exit:
    log_error("header json formate error:  %s ", buf);
    return 0;
}

static int is_login_ok(char *buf, int buf_len)
{
    char *ret = NULL;
    int len = 0, type = 0;
    
    ret = json_get_value_by_name(buf, buf_len, "code", &len, &type);
    if(ret == NULL || len <= 0 || type != JNUMBER || atoi(ret) != 0){
   		log_error("cloud response error: %s ", buf);
		return 0; 
	}

	return 1;
}

static inline int _get_max_socketfd(int cloudfd, int *localfd_array, int len)
{
    int i = 0;
    int max = 0;

    for(i = 0; i < len; i++){
        if(localfd_array[i] > max){
            max = localfd_array[i];
        }
    }

    return 1 + MAX_VAL(cloudfd, max); 
}

static inline int _add_localfd(int localfd, int *localfd_array, int len)
{
    int i = 0;

    for(i = 0; i < len; i++){
        if(localfd_array[i] == -1){
            localfd_array[i] = localfd;
            log_info("index: %d, localfd: %d\n", i, localfd);
            break; 
        }
    }

    return i;
}

static inline void _add_select_array(int cloudfd, int *localfd_array, int len, fd_set *rfds)
{
    int i = 0;

    FD_ZERO(rfds);
    if (cloudfd >= 0) {
        FD_SET(cloudfd, rfds);
    }
    for(i = 0; i < len; i++){
        if(localfd_array[i] >= 0)
            FD_SET(localfd_array[i], rfds);
    }
}

static inline void _del_socketfd_by_index(int index, int *localfd_array, int len)
{
    if (index < len) {
        localfd_array[index] = -1;
    }
}

static inline void _del_socketfd_by_id(int fd, int *localfd_array, int len)
{
    int i = 0;
    
    for(i = 0; i < len; i++){
        if (fd == localfd_array[i]) {
            localfd_array[i] = -1;
        }
    }
}

static inline void _close_all_conn(int *localfd_array, int len)
{
    int i = 0;

    for(i = 0; i < len; i++){
        if(localfd_array[i] > 0){
            close(localfd_array[i]);
            localfd_array[i] = -1;
        }
    }
}

static inline int _get_active_socketfd_index(int *localfd_array, int len, fd_set *rfds)
{
    int i = 0;

    for(i = 0; i < len; i++){
        if(localfd_array[i] == -1)
            continue;
        if(FD_ISSET(localfd_array[i], rfds) > 0)
            return i;
    }

    return -1;
}

static inline void _init_local_fd_array(int *localfd_array, int len)
{
    int i = 0;

    for(i = 0; i < len; i++){
        localfd_array[i] = -1;
    }
}

int _timeval_substract (struct timeval * a,
                           struct timeval * b,
                           struct timeval * result)
{
    /* Perform the carry for the later subtraction by updating
     * y. */
    if (a->tv_usec < b->tv_usec) {
        int nsec = (b->tv_usec - a->tv_usec) / 1000000 + 1;
        b->tv_usec -= 1000000 * nsec;
        b->tv_sec += nsec;
    }

    if (a->tv_usec - b->tv_usec > 1000000) {
        int nsec = (a->tv_usec - b->tv_usec) / 1000000;
        b->tv_usec += 1000000 * nsec;
        b->tv_sec -= nsec;
    }

    /* get the result */
    result->tv_sec = a->tv_sec - b->tv_sec;
    result->tv_usec = a->tv_usec - b->tv_usec;

       /* return 1 if result is negative. */
       return a->tv_sec < b->tv_sec;
}

static int _read_msg_hdr(char *buf, int len)
{
    int i = 0;
    int n = 0;
	int timeout = 1000;
	long ellapsed   = 0;
 
    if(len < DEFAULT_MSG_HDR_LEN)
        return -1;

    memset(buf, 0, len);
    struct  timeval    start;
    struct  timeval    stop;
    struct  timeval    diff; 

#if defined(NOPOLL_OS_WIN32)
	nopoll_win32_gettimeofday (&start, NULL);
#else
	gettimeofday (&start, NULL);
#endif

    while(1){
        n = nopoll_conn_read(g_network.handle, buf+i, 1, nopoll_false, 500);
        if(buf[0] != '{'){
            return -1;
        } 

		if(n == -1){
#if defined(NOPOLL_OS_WIN32)
			nopoll_win32_gettimeofday (&stop, NULL);
#else
			gettimeofday (&stop, NULL);
#endif
			_timeval_substract (&stop, &start, &diff);
			ellapsed = (diff.tv_sec * 1000) + (diff.tv_usec / 1000);
			if (ellapsed > (timeout)){
				log_error("timeout %d, we only read:  %d\n", ellapsed, i);
				break;
			}
			else
				continue;
		}

        if(buf[i] == '}'){
            nopoll_conn_read(g_network.handle, buf+i+1, 4, nopoll_true, 1000);

            return i;
        }
        ++i;
    }
    
    return i;
}

#define DEFAULT_MSG_BUFFER_LEN 8*DEFAULT_MSG_HDR_LEN
static void _send_error_resp(int code, char *msg, char *msg_id, char *session_id, char *send_buf)
{
    char *tmp_buf = NULL;
    char *buf_hdr = NULL;

    tmp_buf  = sda_gen_payload_msg(code, NULL, msg);
    buf_hdr = sda_gen_msg_header(MSG_RESP_OK , strlen(tmp_buf), msg_id, session_id);
    
    memset(send_buf, 0, DEFAULT_MSG_BUFFER_LEN);
    snprintf(send_buf, DEFAULT_MSG_BUFFER_LEN, "%s%s", buf_hdr, tmp_buf); 
    rd_net_write(&g_network, NULL, send_buf, strlen(send_buf), 0);
}

int sda_run_loop (void)
{
    char *buf = NULL, *buf_hdr = NULL, *session_id = NULL, *tmp_buf = NULL;
    int ret = 0, tmp = 0, tmp_fd = 0, payload = 0, tmp_index = 0, timeout = 0;
    struct timeval tv;
    ConnectInfo_t connInfo;
    fd_set rfds;
    int socketfd_local[DEFAULT_SESSION_COUNT] = {0};
    RemoteTerminalMsgHeader hdr;
    
    buf = malloc(DEFAULT_MSG_BUFFER_LEN);
    if(!buf){
        log_error("failed to alloc memory  "); 
        goto _exit;
    }

    ret = sda_connect_to_cloud();
    if (0 != ret) {
        goto _exit;
    }

    memset(&connInfo, 0x0, sizeof(ConnectInfo_t));
    ret = rd_net_get_conn_info(&g_network, NULL, &connInfo);
    if (0 != ret) {
        log_error("get cloud connect info failed ");
        goto _exit;
    }
    _init_local_fd_array(socketfd_local, DEFAULT_SESSION_COUNT);
    log_info("connect to cloud success, socketfd: %d ", connInfo.sockfd);
    while (g_is_running) {
        tv.tv_sec = 0;
        tv.tv_usec = 500*1000L;
        _add_select_array(connInfo.sockfd, socketfd_local, DEFAULT_SESSION_COUNT, &rfds);

        ret = select(_get_max_socketfd(connInfo.sockfd, socketfd_local, DEFAULT_SESSION_COUNT), &rfds, NULL, NULL, &tv);

        memset(buf, 0, DEFAULT_MSG_BUFFER_LEN);
        if (ret == 0) {
            nopoll_conn_send_ping (g_network.handle);
            if(++timeout >= 30*60){
                log_info("there is no package received in 30 min, close current connection...");
                break;
            } 
        }else if(ret < 0){
            log_error("failed to select: %s , close current connection", strerror(errno));
            _send_error_resp(ERR_CONNECTION_CLOSE, "device network error.", NULL, NULL, buf);
            break;
        }

		//local service
		if(ret > 0)
		{
			int index = _get_active_socketfd_index(socketfd_local, DEFAULT_SESSION_COUNT, &rfds);
			if(index != -1){
				char *token = get_session_id_by_socketfd(socketfd_local + index);
				if(token == NULL){
					log_error("can not find session id for socketfd: %d, close it.", socketfd_local[index]);
					_del_socketfd_by_index(index, socketfd_local, DEFAULT_SESSION_COUNT);
					close(socketfd_local[index]);
					continue;
				}

				ret = recv(socketfd_local[index], buf, DEFAULT_MSG_BUFFER_LEN - DEFAULT_MSG_HDR_LEN, 0);
				timeout = 0; 
				log_debug("recv msg comes from sshd, socketfd: %d, index: %d, len:%d ", socketfd_local[index], index, ret);
				//dump_hex(buf, ret);
				if (ret > 0) {
					buf_hdr = sda_gen_msg_header(MSG_SERVICE_PROVIDER_RAW_PROTOCOL, ret, NULL, token);
					memmove(buf + strlen(buf_hdr), buf, ret);
					memmove(buf, buf_hdr, strlen(buf_hdr));
					tmp = rd_net_write(&g_network, NULL, buf, ret + strlen(buf_hdr), 0);
					log_debug("resend to cloud, len:%d", tmp);
				} else {
					log_error("local service exit:  %s ", strerror(errno));
					free_session(get_session_id_by_socketfd(socketfd_local + index));
					_del_socketfd_by_index(index, socketfd_local, DEFAULT_SESSION_COUNT);
					close(socketfd_local[index]);
					_send_error_resp(ERR_SERVICE_EXIT, "local service exit", NULL, token, buf);
				}
			}
		}
		
		//cloud service
        {
            ret = _read_msg_hdr(buf, DEFAULT_MSG_BUFFER_LEN);
			if(ret < 0)
				continue;
            
            timeout = 0;
            if(0 == get_msg_header(buf, &hdr)){
                log_error("read wrong format data, pending data len: %d", rd_net_read(&g_network, NULL, buf, DEFAULT_MSG_BUFFER_LEN, 0));
                _send_error_resp(ERR_PARAM_INVALID, "cloud msg format is invalid", NULL, NULL, buf);
                continue;
            }
            if(hdr.msg_type == MSG_SERVICE_CONSUMER_NEW_SESSION){
                rd_net_read(&g_network, NULL, buf + strlen(buf), hdr.payload_len + 4, 0);//FIXME, deal with service port
                if(0 == is_session_avalibe()){
                    log_error("session is too much...  ");
                    _send_error_resp(ERR_SESSION_LIMIT, "no available session to be allocated", hdr.msg_id, NULL, buf);
                    continue;
                }

                tmp_fd = sda_connect_to_local_service();
                if(tmp_fd == -1){
                    log_error("failed to connect to local service\n");
                    _send_error_resp(ERR_SERVICE_UNAVALIBE, "local service is not available", hdr.msg_id, NULL, buf);
                    continue;
                }
                tmp_index = _add_localfd(tmp_fd, socketfd_local, DEFAULT_SESSION_COUNT);
                if(tmp_index == DEFAULT_SESSION_COUNT){
                    _send_error_resp(ERR_SESSION_CREATE_FAILED, "socketfd insert error", hdr.msg_id, NULL, buf);
                    close(tmp_fd);
                    log_error("failed to add socketfd");
                    continue;
                }

                session_id = alloc_new_session(socketfd_local + tmp_index);
                if(session_id == NULL){
                    close(tmp_fd);
                    log_error("failed to alloc new session");
                    _send_error_resp(ERR_SESSION_CREATE_FAILED, "memory error", hdr.msg_id, NULL, buf);
                    continue;
                }
                char data[128] = {0};
                snprintf(data, sizeof(data) - 1, "{\"session_id\": \"%s\"}", session_id);
                tmp_buf  = sda_gen_payload_msg(0, data, "new session response");

                buf_hdr = sda_gen_msg_header(MSG_RESP_OK , strlen(tmp_buf), hdr.msg_id, session_id);
                memset(buf, 0, DEFAULT_MSG_BUFFER_LEN);
                snprintf(buf, DEFAULT_MSG_BUFFER_LEN, "%s%s", buf_hdr, tmp_buf); 
                rd_net_write(&g_network, NULL, buf, strlen(buf), 0);
                log_debug("send new session respose to cloud, socketfd: %d, index: %d, %s ", socketfd_local[tmp_index], tmp_index, buf);
                free(session_id);
            } else if(hdr.msg_type == MSG_SERVICE_CONSUMER_RELEASE_SESSION){//TODO
                rd_net_read(&g_network, NULL, buf + strlen(buf), hdr.payload_len, 0);//FIXME, deal with service port
                int _socketfd = get_socketfd_by_session_id(hdr.token);
                if(_socketfd < 0){
                    _send_error_resp(ERR_SESSION_NONEXISTENT, "cannot find local connection by session id", hdr.msg_id, hdr.token, buf);
                    log_error("session is not exist");
                    continue; 
                }
                _del_socketfd_by_id(_socketfd, socketfd_local, DEFAULT_SESSION_COUNT);
                close(_socketfd);

                free_session(hdr.token);

                char data[128] = {0};
                snprintf(data, sizeof(data) - 1, "{\"session_id\": \"%s\"}", hdr.token);
                tmp_buf  = sda_gen_payload_msg(0, data, "release session response");

                buf_hdr = sda_gen_msg_header(MSG_RESP_OK , strlen(tmp_buf), hdr.msg_id, hdr.token);
                memset(buf, 0, DEFAULT_MSG_BUFFER_LEN);
                snprintf(buf, DEFAULT_MSG_BUFFER_LEN, "%s%s", buf_hdr, tmp_buf); 
                rd_net_write(&g_network, NULL, buf, strlen(buf), 0);
                log_debug("send release session respose to cloud, socketfd: %d,  %s ", _socketfd, buf); 
            } else if(hdr.msg_type == MSG_SERVICE_CONSUMER_RAW_PROTOCOL){
                int _socketfd = get_socketfd_by_session_id(hdr.token);
                if(_socketfd < 0){
                    _send_error_resp(ERR_SESSION_NONEXISTENT, "cannot find local connection by session id", hdr.msg_id, hdr.token, buf);
                    log_error("session is not exist"); 
                    continue; 
                }

                int n = 0, nwritten = 0, nread = 0, m = 0;
                log_debug("send payload %d start, session id: %s\n", hdr.payload_len, hdr.token);
                memset(buf, 0, DEFAULT_MSG_BUFFER_LEN);
                while(nread < hdr.payload_len){
                    memset(buf, 0, DEFAULT_MSG_BUFFER_LEN);
                    n = nopoll_conn_read(g_network.handle, buf, hdr.payload_len - nread > DEFAULT_MSG_BUFFER_LEN ? DEFAULT_MSG_BUFFER_LEN : hdr.payload_len - nread, nopoll_true, 1000);
                    if(n == -1)
                        continue;
                    nread += n;
                    //dump_hex(buf, n);
                    nwritten = 0;
                    while(nwritten < n){
                        m = send(_socketfd, buf + nwritten, n - nwritten, 0);
                        nwritten += m;
                        if(m == -1){
                            log_info("resend to local service FAILED, len: %d, payload: %d ", n, payload);
                            free_session(hdr.token);
                            _del_socketfd_by_id(_socketfd, socketfd_local, DEFAULT_SESSION_COUNT);
                            close(_socketfd);
                            _send_error_resp(ERR_SERVICE_EXIT, "local service exit", hdr.msg_id, hdr.token, buf);
                            break;
                        }
                    }
                }
                log_debug("send payload %d done\n", hdr.payload_len);
            } else if(hdr.msg_type == MSG_RESP_OK){
                ret = rd_net_read(&g_network, NULL, buf + strlen(buf), hdr.payload_len, 0);//FIXME, deal with service port
                log_debug("login response: %d %s\n", ret, buf);
                if((0 == strcmp(hdr.msg_id, DEFAULT_MSG_ID_HDSK)) && 
                        (0 == is_login_ok(buf + hdr.hdr_len, hdr.payload_len))){
                    log_error("login to cloud failed ");
                    goto _exit;
                }
            }
        } 
    }

_exit:
    deinit_session_system();
    rd_net_destroy(&g_network);
    _close_all_conn(socketfd_local, DEFAULT_SESSION_COUNT);

    if(buf != NULL)
        free(buf);

    return ret;
}

static void _singleton()
{
    char cmd[512] = {0};
    log_debug( "current pid:  %d ",getpid());
    
    snprintf(cmd, sizeof(cmd), "kill -2 `cat /tmp/remote_terminal.pid` 2> /dev/null");  
    system(cmd); 

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "echo \"%d\" > /tmp/remote_terminal.pid", getpid());
    system(cmd);
}

static void _clear_arguments(int argc, char **argv)
{
    int i = 1;
    for (i = 1; i < argc; i++){
        int j = strlen(argv[i]);  
        for (j = j - 1; j >= 0; j--){
            argv[i][j] = ' ';
        }
    }
}

int main (int argc, char **argv)
{
    int times = 0;
 
    struct sigaction sig_int;
    memset(&sig_int, 0, sizeof(struct sigaction));
    sigemptyset(&sig_int.sa_mask);
    sig_int.sa_handler = _sig_int_handler;
    sigaction(SIGINT, &sig_int, NULL); 
    
    printf("%s compiled at %s\n", argv[0], __DATE__);
    if(argc == 4){
        g_cfg = init_cfg(argv[1], argv[2], argv[3]); 
        _clear_arguments(argc, argv); 
    } else if(argc == 1){
        g_cfg = init_cfg(NULL, NULL, NULL); 
    } else{
        log_info("./remote_terminal <product key>  <device name> <device secret> to startup ");
        return 0;
    }
  
    if(g_cfg == NULL || !g_cfg->pk || !g_cfg->dn || !g_cfg->ds){
        log_error("failed to init config\n"); 
        return 0;
    }

    _singleton();

    srand(time(NULL));
    while(g_is_running) {
        if( -1 == init_session_system()){
            sleep(1);
            continue;
        }
        sda_run_loop();
        log_info("reconnect: try %d times ", ++times);
        sleep(1);
    }

    deinit_cfg();
    deinit_session_system();
    return 0;
}

