/*
 * page_pool.h
 *
 *  Created on: 2017年6月17日
 *      Author: liwei
 */

#ifndef PAGE_POOL_H_
#define PAGE_POOL_H_
#include <stdint.h>
#include "util/atomic.h"
#include "util/db_chain.h"

#ifdef MEMDEBUG
#include <stdio.h>
#include <time.h>
#include "batch_log.h"
#endif
#include <pthread.h>

#define MAX_LOCK_SIZE 0X1F

typedef struct _page_cache_node
{
	chain_node cn;
	chain_head cache;
} page_cache_node;
typedef struct page_pool_thread_handle
{
	pthread_t tid; //线程id
	chain_node cn; //注册在pool中的节点

	chain_head cache; //缓存，可以直接分配出去
	chain_head alloced; //已分配出去，正在使用中的
	chain_head gc_list; //被gc的部分，如果gc_list不为空，thd需要将gc list
	uint32_t max_thd_cache;
	struct _page_pool * pool;
} pp_thd;
struct _page_pool_block;

typedef struct _pp_page
{
	chain_node cn;
	pp_thd * thd;
	struct _page_pool_block * block;
	uint64_t page_id;
	char data[1];
} pp_page;

struct _page_pool;
typedef struct _page_pool_block
{
	struct _page_pool *p;
	chain_head using_block_list;
	chain_node cn;
	uint32_t id;
	char buf[1];
} page_pool_block;

typedef struct _page_pool
{
	size_t dt_size;
	size_t block_size;
	unsigned int flags;
	unsigned long align_len;
	unsigned int block_dt_count;

	atomic_t global_block_id;
	chain_head using_block_list;
	chain_head used_block_list;
	pthread_mutex_t global_block_lock;

	uint64_t max_mem;

	chain_head global_page_cache;
	pthread_mutex_t global_page_cache_lock;
	uint32_t global_page_cache_limit;

	pthread_t gc_thread_id;
	pthread_cond_t gc_cond;
	pthread_mutex_t gc_cond_mutex;
	volatile char exit;
	atomic_t high_load;

	chain_head thd_head;
	pthread_mutex_t thd_lock;
	pthread_cond_t lack_cond;
	pthread_mutex_t lack_cond_mutex;

	uint32_t max_thd_cache;
	pp_thd * default_thd;
	pthread_mutex_t default_thd_lock;
	pthread_mutex_t block_lock_list[MAX_LOCK_SIZE+1];

#ifdef MEMDEBUG
	FILE *fp;
	int allocCount;
	batch_log pool_log;
#endif
} page_pool;

pp_thd * create_thd(page_pool *pool, int max_thd_cache);
void destroy_thd(pp_thd * thd);
void invalidate_thd_alloced_list(pp_thd *thd);

int init_page_pool(page_pool *p, size_t page_size, int align, uint64_t max_mem, int max_thd_cache, int global_page_cache_limit);
void destory_page_pool(page_pool *pool);

void * alloc_page(pp_thd *thd);
void free_page(void * data);
#endif /* PAGE_POOL_H_ */
