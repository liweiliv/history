/*
 * c_block.h
 *
 *  Created on: 2016年12月8日
 *      Author: liwei
 */

#ifndef LIB_CALCULATE_ENGINE_C_BLOCK_H_
#define LIB_CALCULATE_ENGINE_C_BLOCK_H_
#include <stdint.h>
#include "util/db_chain.h"
#include "util/atomic.h"
#include <pthread.h>
#include "c_index.h"

/*
 * block在磁盘的格式
 * [head]+[page_index]+[page_1]【page_2】...【page_n】+[index_list]+[index_1][index_2]...[index_n]
 * 每个page单独压缩，避免在随机查询少量记录时需要加载全部数据，牺牲了一定的压缩率
 * 索引在为文件末尾，即使在文件生成后也可以在增加新的索引
 * 第一个索引（如果有索引）为主索引，数据将按第一个索引排序
 * FLAG_BLOCK_LAZY_LOAD模式加载时，只读取[head]+[page_index]+指定的索引
 */
#define FLAG_BLOCK_COMPRESS 0x00000001  //压缩数据
#define FLAG_BLOCK_CRC      0x00000002    //开启压缩的情况下不做crc校验
#define FLAG_BLOCK_LAZY_LOAD  0x00000004 //创建block时只读取到index,data不读取

#define BLOCK_MEM_FLAG_MASK 0xF0000000

#define BLOCK_MAX_INDEX_COUNT 8
#define BLOCK_MAX_FILE_LOCK_COUNT 32

pthread_mutex_t block_file_locks[BLOCK_MAX_FILE_LOCK_COUNT];

typedef struct _c_table_page_info
{
    uint64_t offset;
    uint64_t size;
    uint32_t crc;
} c_table_page_info;
typedef struct _c_table_page
{
    const char *data;
    chain_node cn;
} c_table_page;
typedef struct _c_table_base_block
{
    uint32_t flag;
    uint32_t page_size;
    uint32_t index_size;
    uint32_t data_count;
    uint64_t indexs_offset[BLOCK_MAX_INDEX_COUNT];

    c_table_page_info *page_offset_list;
    c_table_page *page;
    int fd;
    atomic_t users;
    c_table_index **index[BLOCK_MAX_INDEX_COUNT];
} c_table_base_block;

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */
c_table_base_block * load_block_from_fd(int fd,int flag);
void destroy_block(c_table_base_block *block);
int load_all_page(c_table_base_block * block);
int load_page(c_table_base_block * block, int page_id);
int unload_page(c_table_base_block * block, int page_id);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#define C_TABLE_SEARCH_BEFORE 0x00000001 //查找第一个小于等于指定值的key
#define C_TABLE_SEARCH_EQUAL 0x00000002  //查找等于指定值的key
#define C_TABLE_SEARCH_AFTER 0x00000003  //查找第一个大于等于指定值的key

#define C_TABLE_SEARCH_MASK 0x00000004

typedef struct _c_block_iterator
{
    c_table_base_block * block;
    c_table_index * index;

    uint32_t flag;
    int32_t index_offset;
    union
    {
        const char * key_str;
        int64_t key_int;
    };
    size_t key_size;
    const char *value;
    size_t value_size;
} c_block_iterator;



#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */
int seek_to_first(c_table_base_block *block, c_block_iterator *iter,int index_id);
int seek_to_end(c_table_base_block *block, c_block_iterator *iter,int index_id);
int find_in_c_block(c_table_base_block *block, c_block_iterator *iter,
        const char * key, uint64_t key_size,int index_id, uint32_t flag);
int find_intkey_in_c_block(c_table_base_block *block, c_block_iterator *iter,
        int64_t key, int index_id,uint32_t flag);
int c_table_block_iter_prev(c_block_iterator *iter);
int c_table_block_iter_next(c_block_iterator *iter);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* LIB_CALCULATE_ENGINE_C_BLOCK_H_ */
