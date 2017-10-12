/*
 * c_block.c
 *
 *  Created on: 2016年12月8日
 *      Author: liwei
 */
#include "c_block.h"
#include "file_opt.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <errno.h>
#define __USE_LARGEFILE64
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int (*decompress_func)(char *decompress_buf, size_t *decompress_buf_size, const char * data, size_t size);
uint32_t (*crc32_func)(const char * data, size_t size);

#define GET_PAGE_DATA_SIZE(block , page_id)   ( (block)->page_offset_list[(page_id) + 1].offset - (block)->page_offset_list[(page_id)].offset )

static inline int check_is_num(const char * key, uint64_t key_size,
        int64_t *key_int_value)
{
    uint8_t off = 0;
    if (key[0] == '-')
        off = 1;
    *key_int_value = 0;
    while (off < key_size)
    {
        if (key[off] < '0' || key[off] > '9')
            return -1;
        *key_int_value = (*key_int_value * 10) + key[off] - '0';
        off++;
    }
    if (key[0] == '-')
        *key_int_value = -(*key_int_value);
    return 0;
}

static inline int updata_itor(c_block_iterator *iter)
{
    if (iter->index->flag & C_INDEX_INT)
    {
        GET_INT_KEY(iter->index, iter->index_offset,iter->key_int);
        iter->key_size=sizeof(int64_t);
    }
    else
    {
        GET_STR_KEY(iter->index, iter->index_offset, iter->key_str, iter->key_size);
    }


    if(iter->block->page[iter->index->index_head[iter->index_offset].page_value.value_page].data==NULL)
    {
        if(0!=load_page(iter->block,iter->index->index_head[iter->index_offset].page_value.value_page))
            return -1;
    }
    iter->value=iter->block->page[iter->index->index_head[iter->index_offset].page_value.value_page].data+iter->index->index_head[iter->index_offset].page_value.value_offset_in_page;
    iter->value_size = *(uint64_t*)iter->value;
    iter->value+=sizeof(uint64_t);
    return 0;
}
int seek_to_first(c_table_base_block *block, c_block_iterator *iter,int index_id)
{
    if(block->index[index_id]->count<=0)
        return -1;
    iter->index=block->index[index_id];
    iter->index_offset=0;
    iter->block=block;
    if(0!=updata_itor(iter))
        return -1;
    iter->flag = C_TABLE_SEARCH_EQUAL;
    return 0;
}
int seek_to_end(c_table_base_block *block, c_block_iterator *iter,int index_id)
{
    if(block->index.count<=0)
        return -1;
    iter->index=block->index[index_id];
    iter->index_offset=block->index.count-1;
    iter->block=block;
    if(0!=updata_itor(iter))
        return -1;
    iter->flag = C_TABLE_SEARCH_EQUAL;
    return 0;
}
int find_in_c_block(c_table_base_block *block, c_block_iterator *iter, const char * key, uint64_t key_size,int index_id, uint32_t flag)
{
    uint64_t value_off;
    if (block->index[index_id]->flag & C_INDEX_INT)
    {
        int64_t key_int_value;
        if (check_is_num(key, key_size, &key_int_value) != 0)
        {
            return -1;
        }
        if (0 >(iter->index_offset= search_int_key(block->index[index_id], key_int_value, &value_off,flag&C_TABLE_SEARCH_MASK)))
            return -1;
    }
    else
    {
        if (0  >(iter->index_offset=search_str_key(block->index[index_id], key, key_size, &value_off,flag&C_TABLE_SEARCH_MASK)))
            return -1;
    }
    iter->index=block->index[index_id];
    iter->flag = flag;
    iter->block = block;
    updata_itor(iter);
    return 0;
}
int find_intkey_in_c_block(c_table_base_block *block, c_block_iterator *iter, int64_t key, int index_id,uint32_t flag)
{
    uint64_t value_off;
    if (block->index[index_id]->flag & C_INDEX_INT)
    {
        if (0 >(iter->index_offset= search_int_key(block->index[index_id], key, &value_off,flag&C_TABLE_SEARCH_MASK)))
            return -1;
    }
    else
        return -1;
    iter->index=block->index[index_id];
    iter->flag = flag;
    iter->block = block;
    updata_itor(iter);
    return 0;
}
int c_table_block_iter_next(c_block_iterator *iter)
{
    if(++iter->index_offset>=iter->block->index.count)
    {
        iter->value=NULL;
        iter->key_str=NULL;
        return -1;
    }
    else
    {
        updata_itor(iter);
        return 0;
    }
}
int c_table_block_iter_prev(c_block_iterator *iter)
{
    if(--iter->index_offset<0)
    {
        iter->value=NULL;
        iter->key_str=NULL;
        return -1;
    }
    else
    {
        updata_itor(iter);
        return 0;
    }
}
int load_all_page(c_table_base_block * block)
{
    char * decompress_buf=NULL;
    uint64_t  decompress_buf_size=0;
    if(block->page_offset_list[0].offset!=lseek64(block->fd, block->page_offset_list[0].offset, SEEK_SET))
        return -1;
    if(block->flag&FLAG_BLOCK_COMPRESS)
        decompress_buf=(char*)malloc(decompress_buf_size=2*(block->page_offset_list[1].offset
                - block->page_offset_list[0].offset));

    for(int page_id=0;page_id<block->page_size;page_id++)
    {
        c_table_page * page = block->page[page_id];
        uint64_t read_size = GET_PAGE_DATA_SIZE(block,page_id);
        page->data = (char*) malloc(block->page_offset_list[page_id].size);
        /*
         * 压缩模式下需要读取并解压缩
         */
        if (block->flag & FLAG_BLOCK_COMPRESS)
        {
            if(decompress_buf_size<read_size)
            {
                free(decompress_buf);
                decompress_buf=(char*)malloc(decompress_buf_size=read_size);
            }
            if (file_read(block->fd, (unsigned char*) decompress_buf, read_size) != read_size)
                goto ERR_RETURN;

            size_t decompress_size = 0;
            if (0 != decompress_func(page->data, &decompress_size, decompress_buf, read_size)
                    || decompress_size != block->page_offset_list[page_id].size)
                goto ERR_RETURN;
        }
        /*
         * 非压缩模式需要校验crc
         */
        else
        {
            if (file_read(block->fd, (unsigned char*) page->data, read_size) != read_size)
                goto ERR_RETURN;
            if (crc32_func(page->data, read_size)
                    != block->page_offset_list[page_id].crc)
                goto ERR_RETURN;
        }
        continue;
ERR_RETURN:
        if(decompress_buf!=NULL)
            free(decompress_buf);
        for(int i=0;i<=page_id;i++)
        {
            if(block->page[page_id]->data!=NULL)
            {
                free(block->page[page_id].data);
                block->page[page_id].data=NULL;
            }
        }
        return -1;
    }
    if(decompress_buf!=NULL)
        free(decompress_buf);
    return 0;
}
int load_page(c_table_base_block * block, int page_id)
{
    if (block->page_size <= page_id)
        return -1;
    if (block->page[page_id].data != NULL)
        return 0;

    c_table_page * page = block->page[page_id];
    char *data = NULL;
    uint64_t read_size = GET_PAGE_DATA_SIZE(block,page_id);
    char *tmp = (char*) malloc(read_size);

    /*
     * 保证文件操作串行
     */
    pthread_mutex_lock(&block_file_locks[block & BLOCK_MAX_FILE_LOCK_COUNT]);
    lseek64(block->fd, block->page_offset_list[page_id].offset, SEEK_SET);
    if (file_read(block->fd, (unsigned char*) tmp, read_size) != read_size)
    {
        free(tmp);
        return -1;
    }
    pthread_mutex_unlock(&block_file_locks[block & BLOCK_MAX_FILE_LOCK_COUNT]);

    /*
     * 压缩模式下需要读取并解压缩
     */
    if (block->flag & FLAG_BLOCK_COMPRESS)
    {
        data = (char*) malloc(block->page_offset_list[page_id].size);
        size_t decompress_size = 0;
        if (0 != decompress_func(data, &decompress_size, tmp, read_size)
                || decompress_size != block->page_offset_list[page_id].size)
        {
            free(tmp);
            return -1;
        }
        free(tmp);
    }
    /*
     * 非压缩模式需要校验crc
     */
    else
    {
        if (crc32_func(tmp, read_size)
                != block->page_offset_list[page_id].crc)
        {
            free(tmp);
            return -1;
        }
        data = tmp;
    }
    /*
     * 如果此时page已经被设置，放弃操作
     */
    if (page->data != NULL)
    {
        free(data);
        return 0;
    }
    /*
     * 原子的设置page，只有在page为NULL时才能设置，否则放弃
     */
    if (NULL != atomic64_cmpxchg((atomic64_t*) &page->data, NULL, (long) data))
        free(data);
    return 0;
}
int unload_page(c_table_base_block * block, int page_id)
{
    if (block->page_size <= page_id)
        return -1;
    char * data=block->page[page_id].data;
    if (NULL != atomic64_cmpxchg((atomic64_t*) &block->page[page_id].data, (long) data, NULL))
        free(data);
    return 0;
}
c_table_base_block * load_block_from_fd(int fd,int flag)
{
    size_t off = lseek64(fd, 0, SEEK_CUR);
    int index_count=0;
    c_table_base_block head;
    c_table_base_block *block = NULL;
    if (file_read(fd, (unsigned char*)&head, offsetof(c_table_base_block,page_offset_list)) != offsetof(c_table_base_block,page_offset_list))
        goto ERROR_RETURN;

    block=(c_table_base_block*)malloc(sizeof(c_table_base_block)+(head.page_size+1)*(sizeof(c_table_page)+sizeof(c_table_page_info))+BLOCK_MAX_INDEX_COUNT*sizeof(c_table_index *));
    memcpy(block,&head,offsetof(c_table_base_block,page_offset_list));
    block->page_offset_list=(c_table_page_info*)(((char*)block)+sizeof(c_table_base_block));
    block->page=(c_table_page*)(((char*)block->page_offset_list)+sizeof(c_table_page_info)*(head.page_size+1));
    block->index=(c_table_index**)(((char*)block->page)+sizeof(c_table_page)*(head.page_size+1));
    block->fd=fd;

    if (file_read(fd, (unsigned char*)&block->page_offset_list, sizeof(c_table_page_info)*(block->page_size+1)) != sizeof(c_table_page_info)*(block->page_size+1))
        goto ERROR_RETURN;
    memset(block->page,0,sizeof(c_table_page)*(head.page_size+1)+BLOCK_MAX_INDEX_COUNT*sizeof(c_table_index *));

    /*
     * FLAG_BLOCK_LAZY_LOAD模式下，初始化时不会加载page与索引，反之，加载全部page与索引
     */
    if(!(flag&FLAG_BLOCK_LAZY_LOAD))
    {
        /*
         * 加载page
         */
        if(load_all_page(block)!=0)
            goto ERROR_RETURN;
        /*
         * 加载索引,在加载page成功后，无需再lseek，直接继续读取即可
         */
        for(int i=0;i<head.index_size;i++)
        {
            if((block->index[i]=load_c_index(fd))==NULL)
                goto ERROR_RETURN;
        }
    }
    block->fd=fd;
    block->users.counter=0;
    block->flag|=flag;
    block->flag&=(~BLOCK_MEM_FLAG_MASK);
    return block;

ERROR_RETURN:
    lseek64(fd, off, SEEK_SET);
    for(int i=0;i<block->index_size;i++)
    {
        if(block->index[i]!=NULL)
            free(block->index[i]);
        if(block->page[i].data!=NULL)
            free(block->page[i].data);
    }
    free(block);
    return NULL;
}
void destroy_block(c_table_base_block *block)
{
    if(block->index!=NULL)
    {
        for(int i=0;i<block->index_size;i++)
        {
            if(block->index[i]!=NULL)
                free(block->index[i]);
        }
        free(block->index);
    }
    if(block->page!=NULL)
    {
        for(int i=0;i<block->page_size;i++)
        {
            if(block->page[i].data!=NULL)
                free(block->page[i].data);
        }
        free(block->index);
    }
    if(block->fd>0)
        close(block->fd);
    free(block);
}

