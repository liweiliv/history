/*
 * c_index.h
 *
 *  Created on: 2016年12月8日
 *      Author: liwei
 */
#include <stdint.h>
#include <stddef.h>
#include "bloom_filter.h"
#ifndef LIB_CALCULATE_ENGINE_C_INDEX_H_
#define LIB_CALCULATE_ENGINE_C_INDEX_H_

#define C_INDEX_INT 0x00000001          //数字类型的key，不设置此位，则为字符串类型的key
#define C_UNION_KEY 0x00000002          //联合索引,key_and_offset中value_offset存放的是联合索引的下一个key的c_table_index在key_and_offset中的偏移，

#define C_INDEX_SEARCH_BEFORE 0x00000001 //查找第一个小于等于指定值的key
#define C_INDEX_SEARCH_EQUAL 0x00000002  //查找等于指定值的key
#define C_INDEX_SEARCH_AFTER 0x00000003  //查找第一个大于等于指定值的key


#define GET_STR_KEY(index,offset,key,key_size) (key)=(index)->keys+(index)->index_head[offset].key_offset;(key_size)=(index)->index_head[offset+1].key_offset-(index)->index_head[offset].key_offset;
#define GET_INT_KEY(index,offset,int_key) (int_key)=(index)->index_head[offset].key;
#pragma pack(push,1)
typedef struct _idx_page_value                 //存储于磁盘时，block分成多个page压缩
{
   uint32_t value_page;  //value_page表示，value所属的page号
   uint32_t value_offset_in_page; //value_offset_in_page，value在page内的位置
 } idx_page_value;
typedef struct _key_and_offset
{
    union
    {
        uint64_t key_offset; //key为字符串时，key_offset描述key在c_table_index::keys中的偏移，key的长度是index->index_head[i+1].key_offset-index->index_head[i].key_offset
        int64_t key; //key为数字时，key就是实际的key
    };
    union
    {
        uint64_t value_offset; //value的偏移地址
        idx_page_value page_value;
    };

} key_and_offset;
/*在文件中的格式
 *  ------------------------------------------------------------
 * |-32bit-|-32bit-|-64bit--|----------------|---------|
 * |flag   |count  |keysize |      index     |   keys  |
 *----------------------------------------------------------------
 */
typedef struct _c_table_index
{
    uint32_t flag;
    uint32_t index_id;
    uint32_t count; //key的数量
    union
    {
        char * keys; //key为字符串时,用于存放key，key为数字时为NULL
        size_t keys_size;
    };
    int fd;
    uint32_t start;
    uint32_t end;
    key_and_offset index_head[1]; //索引
} c_table_index;
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

c_table_index * create_c_index(const char * buf, size_t size);
c_table_index * load_c_index(int fd);
int search_int_key(c_table_index * index, int64_t key, uint64_t *value_offset,
        int type);
int search_str_key(c_table_index * index, const char* key, size_t key_size,
        uint64_t *value_offset, int type);
int c_table_index_to_file(c_table_index *idx,int fd);
#define  get_index_mem_size(idx) offsetof(c_table_index,index_head)+sizeof(key_and_offset)*((idx)->count+1)+(idx)->keys==NULL?0:(idx)->index_head[(idx)->count].key_offset

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* LIB_CALCULATE_ENGINE_C_INDEX_H_ */
