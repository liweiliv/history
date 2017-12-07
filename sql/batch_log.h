/*
 * batch_log.h
 *
 *  Created on: 2017年6月5日
 *      Author: liwei
 */

#ifndef LIB_CALCULATE_ENGINE_BATCH_LOG_H_
#define LIB_CALCULATE_ENGINE_BATCH_LOG_H_

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
typedef struct _batch_log
{
    char filename [256];
    int fd;
    char * buf;
    int buf_size;
    int buf_used_size;
}batch_log;

int init_batch_log(batch_log * log,const char * filename,int batch)
{
    if(strlen(filename)>=255)
    {
        printf("log file :%s is too long\n",filename);
        return -1;
    }
    strcpy(log->filename,filename);
    log->fd = open(log->filename,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
    if(log->fd <0)
    {
        printf("can not open log file :%s ,errno :%d \n",filename ,errno);
        return -1;
    }
    log->buf = (char*)malloc(log->buf_size=batch);
    log->buf_used_size = 0;
    return 0;
}
void close_batch_log(batch_log * log)
{
    if(log->buf_used_size != write(log->fd,log->buf,log->buf_used_size))
    {
        printf("write log file %s failed ,errno %d\n",log->filename,errno) ;
    }
    free(log->buf);
    close(log->fd);
}
int _log(batch_log * log,const char *fmt,...)
{
    va_list ap;
    va_start(ap,fmt);
    int log_len = 0;
WRITE_LOG:
    log_len = snprintf(log->buf+log->buf_used_size,log->buf_size-log->buf_used_size-1, fmt, ap);
    if(log_len>=log->buf_size-log->buf_used_size-2)
    {
        if(log->buf_used_size==0)
        {
            printf("single log must less than batch ,log :%s \n",log->filename) ;
            printf(fmt,ap);
            return -1;
        }
        if(log->buf_used_size != write(log->fd,log->buf,log->buf_used_size))
        {
            printf("write log file %s failed ,errno %d\n",log->filename,errno) ;
            return -2;
        }
        log->buf_used_size = 0 ;
        goto WRITE_LOG;
    }
    else
        log->buf_used_size += log_len;
    return 0;
}




#endif /* LIB_CALCULATE_ENGINE_BATCH_LOG_H_ */
