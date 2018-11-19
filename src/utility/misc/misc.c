#include <time.h>
#include "misc.h"

#define SHELL_GET_IP "ifconfig eth0 | grep 'inet ' | awk '{ print $2}'"

void dump_hex(char *p , int len)
{
    int i = 0;

    for(i = 0; i < len; i++){
        printf("0x%x ",(unsigned char)p[i]) ; 
        if(i != 0 && i % 10 == 0)
            printf("\n");
    }
    printf("\n");
}

//get local ip of eth0
int get_dev_ip(char *buf, int size)
{
    FILE *fp = NULL;
    char bufTmp[256];
    int len = 0, j = 0, i = 0;

    if (NULL == buf || size < 256) {
        return 0;
    }
    
    fp = popen(SHELL_GET_IP, "r");
    if(fp == NULL)
        return 0;

    memset(buf, 0x0, size);
    memset(bufTmp, 0x0, sizeof(bufTmp));
    fgets(bufTmp, sizeof(bufTmp), fp);

    for(i = 0; i < strlen(bufTmp); i++) {
        if (bufTmp[i] != '[' && bufTmp[i] != ']') {
            buf[j] = bufTmp[i];
            j++;
        }
    }
    len = strlen(buf);
    if(len > 0) {
        if (buf[len - 1] == '\n' || buf[len - 1] == '\r') {
            buf[len - 1] = '\0';
        }
    }

    pclose(fp);

    return len;
}

static char *_rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (size) {
        --size;
        size_t n = 0;
        for (n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

char* rand_string_alloc(size_t size)
{
    char *s = malloc(size + 1);
    if (s) {
        _rand_string(s, size);
    }
    return s;
}

char* rand_string_static()
{
    static char str[32];

    memset(str, 0, sizeof(str));
    
    return _rand_string(str, sizeof(str) - 1);
}

