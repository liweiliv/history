/*
 * test_c_index.c
 *
 *  Created on: 2016年12月20日
 *      Author: liwei
 */
#include <stdlib.h>
#include <stdio.h>
#include "stddef.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "../sql/c_index.h"
int check_int_data(c_table_index *idx)
{
    uint64_t value;
    for(int64_t i=8;i<51;i++)
    {
        int ret=search_int_key(idx,i,&value,C_INDEX_SEARCH_EQUAL);
        if((i>>1)*2==i&&i>=10&&i<=48)
        {
            if(ret<0||value!=i)
            {
                printf("can not find %ld in idx\n",i);
                return -1;
            }
        }
        else
        {
            if(ret>=0)
            {
                printf("find %ld in idx\n",i);
                return -1;
            }
            else
            {
                ret=search_int_key(idx,i,&value,C_INDEX_SEARCH_BEFORE);
                if(i<10)
                {
                    if(ret>=0)
                    {
                        printf("find %ld in idx use C_INDEX_SEARCH_BEFORE\n",i);
                        return -1;
                    }
                }
                else if(ret<0||(i>48?value!=48:value!=i-1))
                {
                    printf("can not find %ld in idx use C_INDEX_SEARCH_BEFORE\n",i);
                    return -1;
                }
                ret=search_int_key(idx,i,&value,C_INDEX_SEARCH_AFTER);
                if(i>48)
                {
                    if(ret>=0)
                    {
                        printf("find %ld in idx use C_INDEX_SEARCH_AFTER\n",i);
                        return -1;
                    }
                }
                else if(ret<0||(i<10?value!=10:value!=i+1))
                {
                    printf("can not find %ld in idx use C_INDEX_SEARCH_AFTER\n",i);
                    return -1;
                }
            }
        }
    }
    return 0;
}
int test_int()
{
    c_table_index *idx=(c_table_index*)malloc(offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21);
    idx->keys=NULL;
    idx->flag=C_INDEX_INT;
    idx->count=20;
    for(int64_t i=5;i<20+5;i++)
    {
        idx->index_head[i-5].key=i*2;
        idx->index_head[i-5].value_offset=i*2;
    }
    idx->index_head[20].key=0;
    idx->index_head[20].value_offset=0;
    printf("test c_table_index search int %s\n",check_int_data(idx)==0?"success":"failed");
    int fd=open("test_c_index_",O_RDONLY);
    if(fd!=0)
    {
        close(fd);
        remove("test_c_index_");
    }
    fd=open("test_c_index_",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if(fd==0)
    {
        close(fd);
        printf("create file test_c_index_ failed\n");
        return -1;
    }
    if(offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21!=write(fd,idx,offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21))
    {
        printf("write file test_c_index_ failed\n");
        close(fd);
        return -1;
    }
    free(idx);
    lseek(fd,0,SEEK_SET);
    idx=load_c_index(fd);
    if(idx==NULL)
    {
        printf("load file test_c_index_ failed\n");
        close(fd);
        return -1;
    }
    printf("test c_table_index load file and search int %s\n",check_int_data(idx)==0?"success":"failed");
    free(idx);
    lseek(fd,0,SEEK_SET);
    char *buf=(char*)malloc(offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21);
    if(read(fd,buf,offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21)!=offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21)
    {
        printf("read file test_c_index_ failed\n");
        close(fd);
        free(buf);
        return -1;
    }
    idx=create_c_index(buf,offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21);
    if(idx==NULL)
    {
        printf("create_c_index failed\n");
        close(fd);
        free(buf);
        return -1;
    }
    printf("test c_table_index create_c_index and search int%s\n",check_int_data(idx)==0?"success":"failed");
    close(fd);
    free(buf);
    return 0;
}
int check_str_data(c_table_index *idx)
{
    uint64_t value;
    char str[6]={0};
    for(int64_t i=8;i<51;i++)
    {
        sprintf(str,"%02ld%03ld",i,i);
        int ret=search_str_key(idx,str,5,&value,C_INDEX_SEARCH_EQUAL);
        if((i>>1)*2==i&&i>=10&&i<=48)
        {
            if(ret<0||value!=i)
            {
                printf("can not find %ld in idx\n",i);
                return -1;
            }
        }
        else
        {
            if(ret>=0)
            {
                printf("find %ld in idx\n",i);
                return -1;
            }
            else
            {
                ret=search_str_key(idx,str,5,&value,C_INDEX_SEARCH_BEFORE);
                if(i<10)
                {
                    if(ret>=0)
                    {
                        printf("find %ld in idx use C_INDEX_SEARCH_BEFORE\n",i);
                        return -1;
                    }
                }
                else if(ret<0||(i>48?value!=48:value!=i-1))
                {
                    printf("can not find %ld in idx use C_INDEX_SEARCH_BEFORE\n",i);
                    return -1;
                }
                ret=search_str_key(idx,str,5,&value,C_INDEX_SEARCH_AFTER);
                if(i>48)
                {
                    if(ret>=0)
                    {
                        printf("find %ld in idx use C_INDEX_SEARCH_AFTER\n",i);
                        return -1;
                    }
                }
                else if(ret<0||(i<10?value!=10:value!=i+1))
                {
                    printf("can not find %ld in idx use C_INDEX_SEARCH_AFTER\n",i);
                    return -1;
                }
            }
        }
    }
    return 0;
}
int test_str()
{
    int data_size=offsetof(c_table_index,index_head)+sizeof(key_and_offset)*21+20*5;
    c_table_index *idx=(c_table_index*)malloc(data_size);
    idx->keys=NULL;
    idx->flag=0;
    idx->count=20;
    idx->keys=(char*)&idx->index_head[21];
    for(int64_t i=5;i<20+5;i++)
    {
        idx->index_head[i-5].key_offset=(i-5)*5;
        idx->index_head[i-5].value_offset=i*2;
        sprintf(idx->keys+((i-5)*5),"%02ld%03ld",i*2,i*2);
    }
    idx->index_head[20].key_offset=20*5;
    idx->index_head[20].value_offset=0;

    printf("test c_table_index search str %s\n",check_str_data(idx)==0?"success":"failed");
    int fd=open("test_c_index_",O_RDONLY);
    if(fd!=0)
    {
        close(fd);
        remove("test_c_index_");
    }
    fd=open("test_c_index_",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if(fd==0)
    {
        close(fd);
        printf("create file test_c_index_ failed\n");
        return -1;
    }
    *(uint64_t*)&idx->keys=100;
    if(data_size!=write(fd,idx,data_size))
    {
        printf("write file test_c_index_ failed\n");
        close(fd);
        return -1;
    }
    free(idx);
    lseek(fd,0,SEEK_SET);
    idx=load_c_index(fd);
    if(idx==NULL)
    {
        printf("load file test_c_index_ failed\n");
        close(fd);
        return -1;
    }
    printf("test c_table_index load file and search str %s\n",check_str_data(idx)==0?"success":"failed");
    free(idx);
    lseek(fd,0,SEEK_SET);
    char *buf=(char*)malloc(data_size);
    if(read(fd,buf,data_size)!=data_size)
    {
        printf("read file test_c_index_ failed\n");
        close(fd);
        free(buf);
        return -1;
    }
    idx=create_c_index(buf,data_size);
    if(idx==NULL)
    {
        printf("create_c_index failed\n");
        close(fd);
        free(buf);
        return -1;
    }
    printf("test c_table_index create_c_index and search str %s\n",check_str_data(idx)==0?"success":"failed");
    close(fd);
    free(buf);
    return 0;
}
int main()
{
    test_int();
    test_str();
}



