/*
 * big_block.c
 *
 *  Created on: 2017年3月29日
 *      Author: liwei
 */
#include "file_opt.h"
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
//todo
#include <stdio.h>

#define DEFAULT_PAGE_SIZE (128*1024)
#define MAX_PAGE_IN_BLOCK (1024*4)
#define MAX_BLOCK_SIZE (MAX_PAGE_IN_BLOCK*DEFAULT_PAGE_SIZE)

#define FLAG_BLOCK_COMPRESS 0x00000001  //压缩数据
#define FLAG_BLOCK_CRC      0x00000002    //开启压缩的情况下不做crc校验
#define FLAG_BLOCK_SYNCWRITE  0x00000004 //append时需要flush

int (*decompress_func)(char *decompress_buf, size_t *decompress_buf_size, const char * data, size_t size);
int (*compress_func) (char *dest,   size_t *destLen,const char *source, size_t sourceLen);
uint32_t (*crc32_func)(const char * data, size_t size);

typedef struct _b_page
{
    uint64_t real_size;
    uint32_t crc;
    char data[1];
}b_page;
#define dbgprint(format,args...) fprintf(stderr, format, ##args)
typedef struct _block
{
    int fd;
    char *filename;
    uint32_t flag;
    uint32_t page_count;
    uint64_t page_offset_list[1];
}block;
static char* default_compress_buf=NULL;
static char *big_compress_buf=NULL;
static long big_compress_buf_size=0;

int active_block_append_page(block *b,b_page *p)
{
    if(b->page_count>=MAX_PAGE_IN_BLOCK||b->page_offset_list[b->page_count]>MAX_BLOCK_SIZE)
        return -1;
    char * real_write_data;
    size_t real_write_data_size;
    if(b->flag&FLAG_BLOCK_COMPRESS)
    {
        if(p->real_size>DEFAULT_PAGE_SIZE)
        {
            if(p->real_size>big_compress_buf_size)
            {
                if(big_compress_buf!=NULL)
                    free(big_compress_buf);
                big_compress_buf=(char*)malloc(big_compress_buf_size=p->real_size*2+sizeof(uint64_t));
            }
            real_write_data=default_compress_buf;
        }
        else
            real_write_data=default_compress_buf;
        if(0!=compress_func(real_write_data+sizeof(uint64_t),&real_write_data_size,p->data,p->real_size))
        {
            dbgprint("compress_func failed in file %s ,page id :%d,page size :%lu\n",b->filename,b->page_count+1,p->real_size);
            return -1;
        }
        *(uint64_t*)real_write_data=p->real_size;
        real_write_data_size+=sizeof(uint64_t);
    }
    else
    {
        p->crc=crc32_func(p->data,p->real_size);
        real_write_data=(char*)p;
        real_write_data_size= offsetof(b_page,data) + p->real_size;
    }
    if(real_write_data_size!=file_write(b->fd,(unsigned char*)real_write_data,real_write_data_size))
    {
        //todo log
        dbgprint("append file to block %s  failed ,id:%d,size:%lu,errno:%d\n",b->filename,b->page_count+1,real_write_data_size,errno);
        return -2;
    }
    if(b->flag&FLAG_BLOCK_SYNCWRITE)
    {
        if(fsync(b->fd)!=0)
        {
            //todo log
            dbgprint("flush append data to block file %s failed ,id:%d,size:%lu,errno:%d\n",b->filename,b->page_count+1,real_write_data_size,errno);
            return -3;
        }
    }
    b->page_offset_list[b->page_count+1]=b->page_offset_list[b->page_count]+real_write_data_size;
    b->page_count++;
    return 0;
}
int flush_block(block *b)
{
    int ret=0;
    char * filename=malloc(strlen(b->filename)+6);
    sprintf(filename,"%s.page",b->filename);
    if(0!= (ret=create_file(filename,&b->page_count,sizeof(uint32_t)+sizeof(uint64_t)*(b->page_count+1))))
    {
        //todo
        dbgprint("flush page index  to file :%s failed,create_file return value:%d\n",filename,ret);
        free(filename);
        return -1;
    }
    free(filename);
    if(fsync(b->fd)!=0)
    {
        //todo log
        dbgprint("flush append data to block file :%s failed,errno:%d\n",b->filename,b->page_count,errno);
        return -2;
    }
    return 0;
}
