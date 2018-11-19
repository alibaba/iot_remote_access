#include <stdarg.h>
#include <time.h>
#include "simple_log.h"

//date [module] level <tag> file-func:line content
#define LOG_FMT     "%s <%s> %s: %d "

static const char       *g_log_desc[] = {"DBG", "INF",
                                         "WRN", "ERR",
                                         "FTL" };

static int g_log_lvl = LOG_LEVEL_DEBUG;
static char *get_timestamp(char *buf, int len, time_t cur_time)
{
    struct tm tm_time;

    localtime_r(&cur_time, &tm_time);

    snprintf(buf, len, "%d-%d-%d %d:%d:%d",
             1900 + tm_time.tm_year, 1 + tm_time.tm_mon,
             tm_time.tm_mday, tm_time.tm_hour,
             tm_time.tm_min, tm_time.tm_sec);
    return buf;
}

void set_log_level(int lvl)
{
    if(lvl < LOG_LEVEL_DEBUG || lvl > LOG_LEVEL_NONE)
        g_log_lvl = LOG_LEVEL_ERR; 
    else 
        g_log_lvl = lvl;
    printf("set log level :  %d\n", lvl);
}

#define color_len_fin strlen(COL_DEF)
#define color_len_start strlen(COL_RED)
void log_print(LOG_LEVEL lvl, cchar *color, cchar *f, int l, cchar *fmt, ...)
{
    if(lvl < g_log_lvl)
        return;

    va_list ap;
    va_start(ap, fmt);

    char *tmp = NULL;
    char buf_date[20] = {0};
    time_t cur_time = time(NULL);

    f = !f ? "\b" : f;

    tmp = strrchr(f, '/');
    if(tmp)
        f = tmp + 1;

    //add color support
    if (color) 
        printf("%s", color);
    
    printf(LOG_FMT,get_timestamp(buf_date, 20, cur_time), g_log_desc[lvl], f, l);

    vprintf(fmt, ap);

    if (color) 
        printf("%s\n", COL_DEF);

    va_end(ap);
}

