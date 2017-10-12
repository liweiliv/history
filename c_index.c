/*
 * c_index.c
 *
 *  Created on: 2016年12月8日
 *      Author: liwei
 */
#include "c_index.h"
#include "file_opt.h"
#include <stdio.h>
#include <fcntl.h>
#define __USE_LARGEFILE64
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
c_table_index * create_c_index(const char * buf, size_t size)
{
    c_table_index * idx = (c_table_index*) buf;
    if (idx->flag & C_INDEX_INT)
    {
        if (size
                < offsetof(c_table_index, index_head)
                        + (idx->count + 1) * sizeof(key_and_offset))
            return NULL;
        idx->keys = NULL;
    }
    else
    {
        if (size
                < offsetof(c_table_index, index_head)
                        + (idx->count + 1) * sizeof(key_and_offset))
            return NULL;
        if (size
                < offsetof(c_table_index, index_head)
                        + (idx->count + 1) * sizeof(key_and_offset)
                        + idx->index_head[idx->count].key_offset)
            return NULL;
        idx->keys = (char*) &idx->index_head[idx->count + 1];
    }
    return idx;
}
c_table_index * load_c_index(int fd)
{
    c_table_index * idx;
    size_t off = lseek64(fd, 0, SEEK_CUR);
    size_t size = lseek64(fd, 0, SEEK_END) - off;
    lseek64(fd, off, SEEK_SET);
    c_table_index head;
    if (size < offsetof(c_table_index,fd))
        return NULL;
    if (file_read(fd, (unsigned char *)&head, offsetof(c_table_index,fd)) != offsetof(c_table_index,fd)) //flag，size，keys;在文件中keys存放的是keys的长度
    {
        lseek64(fd, off, SEEK_SET);
        return NULL;
    }
    size_t index_size = head.keys_size+(head.count + 1) * sizeof(key_and_offset); //剩余未读取的长度包含index_head数组和key
    if (size < offsetof(c_table_index,index_head) + index_size) //文件剩余长度不能小于未读取长度
    {
        lseek64(fd, off, SEEK_SET);
        return NULL;
    }
    idx = (c_table_index*) malloc(offsetof(c_table_index, index_head)
            + index_size);
    memcpy(idx,&head,offsetof(c_table_index, fd));
    if (file_read(fd, (unsigned char *)&idx->index_head[0], index_size) != index_size)
    {
        free(idx);
        lseek64(fd, off, SEEK_SET);
        return NULL;
    }
    if(idx->keys_size!=0)
        idx->keys = (char*) &idx->index_head[idx->count + 1];
    else
        idx->keys = NULL;
    return idx;
}

//查找数字类型的key
int search_int_key(c_table_index * index, int64_t key, uint64_t *value_offset,
        int type)
{
    int start = 0, end = index->count - 1, idx;
    while (start <= end)
    {
        idx = (start + end) >> 1;
        if ((index->index_head[idx].key) > key)
            end = idx - 1;
        else if ((index->index_head[idx].key) < key)
            start = idx + 1;
        else
        {
            *value_offset = index->index_head[idx].value_offset;
            return idx;
        }
    }
    if (type == C_INDEX_SEARCH_BEFORE)
    {
        if (start > end)
            start = end;
        if (start < 0)
            return -1;
        *value_offset = index->index_head[start].value_offset;
        return start;
    }
    else if (type == C_INDEX_SEARCH_AFTER)
    {
        //todo
        if (start > index->count - 1)
            return -1;
        *value_offset = index->index_head[start].value_offset;
        return start;
    }
    else if (type == C_INDEX_SEARCH_EQUAL)
        return -1;
    else
        return -1;
}

static inline int compare_str(const char* src_key, size_t src_key_size,
        const char* dest_key, size_t dest_key_size)
{
    if (src_key_size <= dest_key_size)
        return memcmp(src_key, dest_key, src_key_size);
    else
        return memcmp(src_key, dest_key, dest_key_size);
}

int search_str_key(c_table_index * index, const char* key, size_t key_size, uint64_t *value_offset, int type)
{
    int start = 0, end = index->count - 1, idx;
    while (start <= end)
    {
        idx = (start + end) >> 1;
        int c = compare_str(key, key_size, index->keys + index->index_head[idx].key_offset, index->index_head[idx + 1].key_offset - index->index_head[idx].key_offset);
        if (c < 0)
            end = idx - 1;
        else if (c > 0)
            start = idx + 1;
        else
        {
            *value_offset = index->index_head[idx].value_offset;
            return idx;
        }
    }
    if (type == C_INDEX_SEARCH_BEFORE)
    {
        if (start > end)
            start = end;
        if (start < 0)
            return -1;
        *value_offset = index->index_head[start].value_offset;
        return start;
    }
    else if (type == C_INDEX_SEARCH_AFTER)
    {
        //todo
        if (start > index->count - 1)
            return -1;
        *value_offset = index->index_head[start].value_offset;
        return start;
    }
    else if (type == C_INDEX_SEARCH_EQUAL)
        return -1;
    else
        return -1;
}
int c_table_index_to_file(c_table_index *idx,int fd)
{
    uint32_t idx_head_size=offsetof(c_table_index,index_head)+sizeof(key_and_offset)*(idx->count+1);
    char *keys=NULL;
    if((!idx->flag&C_INDEX_INT)||(idx->flag&C_UNION_KEY))
    {
        keys=idx->keys;
        idx->keys_size=idx->index_head[idx->count].key_offset;
    }

    if(file_write(fd,(unsigned char *)idx,idx_head_size)!=idx_head_size)
    {
        close(fd);
        return -1;
    }
    if((!idx->flag&C_INDEX_INT)||(idx->flag&C_UNION_KEY))
    {
        if(file_write(fd,(unsigned char *)keys,(size_t)idx->keys)!=(size_t)idx->keys)
        {
            close(fd);
            return -2;
        }
    }
    idx->keys=keys;
    return 0;
}
void init_c_table_index(c_table_index * idx, uint32_t flag)
{

}
/*
 * count系列函数
 */
/*
 * get_str_count_by_range ，返回大于等于min、小于等于max的所有的string类型的key的数量
 * 需要找所有大于等于min的，需要将max设置为NULL
 * 需要找所有小于等于max的，需要将min设置为NULL
 */
uint64_t get_str_count_by_range(c_table_index * idx,const char * min,size_t min_size,const char *max,size_t max_size)
{
    int start,end;
    uint64_t value_offset;
    /*
     * 如果找不到大于等于min的，返回0
     */
    if(min==NULL)
        start=0;
    else if(0>(start=search_str_key(idx,min,min_size,&value_offset,C_INDEX_SEARCH_AFTER)))
        return 0;
    /*
     * 在找到了大于等于min的前提下，找不到小于等于max的，要么max比min小，要么第一个大于min的比max也大
     */
    if(max==NULL)
        end=idx->count - 1;
    if(0>(end=search_str_key(idx,max,max_size,&value_offset,C_INDEX_SEARCH_BEFORE)))
        return 0;
    /*
     * end小于start，参数非法
     */
    if(end<start)
        return 0;
    return end-start+1;
}
/*
 * get_num_count_by_range ，返回大于等于min、小于等于max的所有的number类型的key的数量
 * 需要找所有大于等于min的，需要将max设置为0xfffffffffffffff
 * 需要找所有小于等于max的，需要将min设置为0x7ffffffffffffff
 */
uint64_t get_num_count_by_range(c_table_index * idx,int64_t min,int64_t max)
{
    int start,end;
    uint64_t value_offset;
    /*
     * 如果找不到大于等于min的，返回0
     */
    if(min==(int64_t)0x7ffffffffffffff)
        start=0;
    else if(0>(start=search_int_key(idx,min,&value_offset,C_INDEX_SEARCH_AFTER)))
        return 0;
    /*
     * 在找到了大于等于min的前提下，找不到小于等于max的，要么max比min小，要么第一个大于min的比max也大
     */
    if(max==(int64_t)0xfffffffffffffff)
        end=idx->count - 1;
    else if(0>(end=search_int_key(idx,max,&value_offset,C_INDEX_SEARCH_BEFORE)))
        return 0;
    /*
     * end小于start，参数非法
     */
    if(end<start)
        return 0;
    return end-start+1;
}
