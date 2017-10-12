/*
 * memp2.h
 *
 *  Created on: 2015��7��31��
 *      Author: liwei
 */

#ifndef LIB_UTIL_MEMLIB_MEMP2_H_
#define LIB_UTIL_MEMLIB_MEMP2_H_
#include <stdint.h>

#include "../db_chain.h"
#ifndef ALIGN
#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))
#endif
#ifndef likely
# define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)    __builtin_expect(!!(x), 0)
#endif
#define BLOCK_SIZE (64*1024)
#define MAX_BLOCK_SIZE (64*64*1024)
#define cache_size  64
#define max_free_block 16;
//#define MEMDEBUG 1
#ifdef MEMDEBUG
#include <stdio.h>
#include<time.h>
#endif
struct lo_mempool_giant;
typedef struct lo_mempool_block_giant
{
	struct lo_mempool_giant *m;
	chain_head using_block_list;
	chain_node cn;
	char  buf[1];
}mempool_block_giant;
struct _mp_buf_giant;
typedef struct lo_mempool_giant
{
	size_t dt_size; //��ݳ���
	size_t block_size;
	unsigned int flags;
	unsigned long align_len;//���볤��
	unsigned int block_dt_count;//ÿ��block�����ɵ��������
	chain_head using_block_list;
	chain_head used_block_list;
	struct _mp_buf_giant *cache[cache_size];//����
	int cache_count;
#ifdef MEMDEBUG
	FILE *fp;
	int allocCount;
#endif
}mempool_giant;
typedef struct _mp_buf_giant
{
	mempool_block_giant * mb;
	union
	{
		chain_node cn;
		char buf[1];
	}mem;
}mp_buf_giant;
#define BLOCK_BASE_SIZE 16
int init_mempool_giant(mempool_giant *m,size_t size,int align)
{
	m->align_len=align;
	if(align>0)
		m->dt_size=ALIGN(size+sizeof(void*),align);
	else
		m->dt_size=size+sizeof(void*);
	if(m->dt_size*BLOCK_BASE_SIZE+offsetof(mempool_block_giant,buf)<BLOCK_SIZE)
		m->block_size=BLOCK_SIZE;
	else if(m->dt_size*BLOCK_BASE_SIZE+offsetof(mempool_block_giant,buf)<MAX_BLOCK_SIZE)
		m->block_size=m->dt_size*BLOCK_BASE_SIZE+offsetof(mempool_block_giant,buf);
	else
		m->block_size=MAX_BLOCK_SIZE;
	m->block_dt_count=(m->block_size-offsetof(mempool_block_giant,buf))/m->dt_size;
	c_init_chain(&m->using_block_list);
	c_init_chain(&m->used_block_list);
	memset(m->cache,0,sizeof(m->cache));
	m->cache_count=-1;
#ifdef MEMDEBUG
	char buf[64]={0};
	sprintf(buf,"0X%lX_mem.log",m);
	m->fp=fopen(buf,"a+");
	m->allocCount=0;
#endif
	return 0;
}
//����һ���µ�4kģʽ��block��block��ÿ��buf�Ĵ�С��4k��buf�Ŀ�ͷ��mp_4k_buf�ṹ��
static inline mempool_block_giant * get_new_giant_block(mempool_giant *pool)
{
	mp_buf_giant *buf;
	mempool_block_giant * mb=(mempool_block_giant *)malloc(pool->block_size);
	if(mb==NULL)
		return NULL;
	unsigned long end=((unsigned long)mb)+pool->block_size-pool->dt_size;
	mb->m=pool;
	c_init_chain(&mb->using_block_list);
	buf=(mp_buf_giant*)(void*)ALIGN((unsigned long)&mb->buf,pool->align_len);
	do
	{
		c_insert_in_head(&mb->using_block_list,&buf->mem.cn);
		buf->mb=mb;
		buf=(mp_buf_giant *)(((long)buf)+pool->dt_size);
	}while(((unsigned long)buf)<=end);
	c_insert_in_head(&pool->using_block_list,&mb->cn);
#ifdef MEMDEBUG
	fprintf(pool->fp,"%d,alloc.dt_size:%d,using_block:%d,used_block:%d,all mem:%d,mem used:%d\n",time(NULL),pool->dt_size,pool->using_block_list.count,pool->used_block_list.count,BLOCK_SIZE*(pool->using_block_list.count+pool->used_block_list.count),pool->allocCount*pool->dt_size);
#endif
	return mb;
}

//����cacheֱ��ˮλ�ﵽnew_top
static inline void clear_mp_cache_giant(mempool_giant *pool,int new_top)
{
	mp_buf_giant *buf;
	mempool_block_giant *block;
	//ѭ������cache�Ķ���
	while(pool->cache_count>=new_top)
	{
		buf=pool->cache[pool->cache_count--];
		block=buf->mb;
		c_insert_in_head(&block->using_block_list,&buf->mem.cn);
		if(block->using_block_list.count==1)
		{
			c_delete_node(&pool->used_block_list,&block->cn);
			c_insert_in_head(&pool->using_block_list,&block->cn);
		}
		if(block->using_block_list.count==pool->block_dt_count&&pool->using_block_list.count>2)
		{
			c_delete_node(&pool->using_block_list,&block->cn);
			free(block);
		}
	}
#ifdef MEMDEBUG
    fprintf(pool->fp,"%d.free dt_size:%d,using_block:%d,used_block:%d,all mem:%d,mem used:%d\n",time(NULL),pool->dt_size,pool->using_block_list.count,pool->used_block_list.count,BLOCK_SIZE*(pool->using_block_list.count+pool->used_block_list.count),pool->allocCount*pool->dt_size);
#endif
}
#ifdef __cplusplus
static inline int destory_mempool_giant(mempool_giant *pool,int force=1)
#else
static inline int destory_mempool_giant(mempool_giant *pool,int force)
#endif
{
	realase_chain(&pool->using_block_list,mempool_block_giant,cn,free);
	realase_chain(&pool->used_block_list,mempool_block_giant,cn,free);
	#ifdef MEMDEBUG
	fclose(pool->fp);
	#endif
	return 0;
}
static inline void *get_mem_giant(mempool_giant *pool)
{
	if(pool->cache_count>=0)
	{
		#ifdef MEMDEBUG
		pool->allocCount++;
		fprintf(pool->fp,"alloc:0x%lx\n",pool->cache[pool->cache_count]->mem.buf);
		#endif
		return pool->cache[pool->cache_count--]->mem.buf;
	}
	mp_buf_giant *buf;
	mempool_block_giant *block;
	while(pool->cache_count<cache_size/2)
	{
		if(pool->using_block_list.count>0)
		{
			block=get_first_dt(&pool->using_block_list,mempool_block_giant,cn);
			while(pool->cache_count<cache_size/2)
			{
				buf=get_first_dt(&block->using_block_list,mp_buf_giant,mem.cn);
				c_delete_node(&block->using_block_list,&buf->mem.cn);
				pool->cache[++pool->cache_count]=buf;
				if(block->using_block_list.count<=0)
				{
					c_delete_node(&pool->using_block_list,&block->cn);
					c_insert_in_head(&pool->used_block_list,&block->cn);
					break;
				}
			}
		}
		else
		{
			if(NULL==(block=get_new_giant_block(pool)))
			{
				if(pool->cache_count>=0)
					break;
				return NULL;
			}
		}
	}
	if(pool->cache_count>=0)
	{
		#ifdef MEMDEBUG
        pool->allocCount++;
	fprintf(pool->fp,"alloc:0x%lx\n",pool->cache[pool->cache_count]->mem.buf);
        #endif
		return pool->cache[pool->cache_count--]->mem.buf;
	}
	else
		return NULL;
}
static inline void free_mem_giant(void *mem)
{
	if(mem==NULL)
		return;
	mp_buf_giant *buf=(mp_buf_giant*)((long)mem-offsetof(mp_buf_giant,mem.buf));
	mempool_giant *pool=buf->mb->m;
    #ifdef MEMDEBUG	
    fprintf(pool->fp,"free:0x%lx\n",mem);
    pool->allocCount--;
    #endif
put:
	if(pool->cache_count<cache_size-1)
	{
		pool->cache[++pool->cache_count]=buf;
		return;
	}
	clear_mp_cache_giant(pool,cache_size*4/5);
	goto put;
}
#endif /* LIB_UTIL_MEMLIB_MEMP2_H_ */
