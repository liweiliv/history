/*
 * mempool.h
 *
 *  Created on: 2015年4月1日
 *      Author: liwei
 */

#ifndef SRC_CONGO_DRC_LIB_MEMLIB_MEMPOOL_H_
#define SRC_CONGO_DRC_LIB_MEMLIB_MEMPOOL_H_
//#include "public.h"
#include "db_chain.h"
#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))
//#include "spinlock.h"
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//using namespace std;
#ifndef likely
# define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)    __builtin_expect(!!(x), 0)
#endif
#define block_size (64*1024)
#define cache_size  64
#define max_free_block 16;

#define mem_flag_lock 0x00000001   //加锁
#define mem_flag_small 0x00000002
#define page_mask ((1<<12)-1)
#if block_size>1024*1024
typedef uint16_t PTYPE ;
typedef uint32_t VTYPE ;
#else
typedef uint8_t PTYPE ;
typedef uint16_t VTYPE;
#endif
#define cache_thread_mod 0x3f
//#define MEMPOOL_DEBUG 1
#ifdef MEMPOOL_DEBUG
#include <stdio.h>
#endif
typedef union
{
	VTYPE v;
	struct {
		PTYPE p;
		PTYPE n;
	} node;
}block_list;
struct lo_mempool;
typedef struct lo_mempool_block
{
	void *start;        //block的有效空间的起始地址
	void *buf_start;    //block内存块的地址
	struct lo_mempool *pool;
	chain_node node;
	PTYPE all_count;   	  //block中buf的总数
	PTYPE current_free_count;   	  //block中free buf数
	PTYPE first_used_pos;  //第一个耗尽的buf的序号
	PTYPE first_using_pos; //第一个半使用的buf的序号
	block_list pos_clum[1];
}mempool_block;
typedef struct lo_mempool
{
	unsigned long dt_size; //数据长度
	unsigned int flags;
	unsigned int blocksize;
	unsigned int align_len;//对齐长度
	unsigned int buf_dt_count;//每个buf能容纳的数据数量
	unsigned int start_pos;   //每个buf中有效空间的起始偏移
	chain_head using_block_list;
	chain_head used_block_list;
#ifdef MEMPOOL_DEBUG
	FILE * fp;
#endif
	void *cache[cache_thread_mod];//缓存
	int cache_count[cache_thread_mod];
	pthread_mutex_t lock;
	pthread_mutex_t hash_lock[cache_thread_mod];
}mempool;
typedef struct _mp_4k_buf
{
	mempool_block * block;
	void * first;
	void * last;
	uint16_t c;
}mp_4k_buf;
#define free_mpnode(block) 		do{if((void*)(block)<(block)->buf_start||(char*)(block)>(char*)(block)->buf_start+(pool)->blocksize){free(block->buf_start);free(block);}else free(block->buf_start);}while(0);
static inline void check_chain(mempool_block *block)
{
}
//清理cache直至水位达到new_top
static inline void clear_mp_cache(mempool *pool,int new_top,int id)
{
	int offset=0;
	void *cache_mem;
	mp_4k_buf *buf;
	mempool_block *block;
#ifdef MEMPOOL_DEBUG
	fprintf(pool->fp,"clean pool to %d\n",new_top);
#endif
	//循环处理cache的顶部
	while(pool->cache_count[id]>new_top)
	{
		//获得cache_mem对应的buf，将cache_mem归还给buf
		buf=(mp_4k_buf*)((unsigned long)(cache_mem=pool->cache[id])&(~page_mask));
		block=buf->block;
		offset=((char*)cache_mem-((char *)buf+pool->start_pos))/pool->dt_size;
		pool->cache[id]=*(void**)pool->cache[id];
		pool->cache_count[id]--;
		if(buf->last==NULL)
			buf->last=cache_mem;
		*(void**)cache_mem=buf->first;
		buf->first=cache_mem;
		//如果buf已经空闲，将buf加入的block的空闲链表
#ifdef MEMPOOL_DEBUG
        fprintf(pool->fp,"buf:%lx get mem:%lx,count:\n",buf,cache_mem,buf->c+1);
#endif
		if(++buf->c==1)
		{
			//如果block的using为空,现在将改变状态，则将block加入到pool的using链表
			if(block->first_using_pos==block->all_count)
			{
				c_delete_node(&pool->used_block_list,&block->node);
				c_insert_in_head(&pool->using_block_list,&block->node);
			}
			offset=((char*)buf-(char*)block->start)/4096;
			//将buf从当前所处的链表中删除
			block->pos_clum[block->pos_clum[offset].node.n].node.p=block->pos_clum[offset].node.p;
			block->pos_clum[block->pos_clum[offset].node.p].node.n=block->pos_clum[offset].node.n;
			//如果buf是当前所处链表的表头，将当前链表的表头更行
			if(block->first_used_pos==offset)
				block->first_used_pos=block->pos_clum[offset].node.n;
			//将buf加入到free链表的头部
			block->pos_clum[offset].node.n=block->first_using_pos;
			block->pos_clum[block->first_using_pos].node.p=offset;
			block->first_using_pos=offset;
			block->pos_clum[block->first_using_pos].node.p=block->all_count;
#ifdef MEMPOOL_DEBUG
			fprintf(pool->fp,"buf:%lx now it is not empty\n",buf);
#endif
		}
		if(buf->c==pool->buf_dt_count)
		{
			if(++block->current_free_count==block->all_count)
			{
#ifdef MEMPOOL_DEBUG
            	fprintf(pool->fp,"buf:%lx 's block:%lx is full ,free it\n",buf,block);
#endif
				c_delete_node(&pool->using_block_list,&block->node);
				free_mpnode(block);
			}
		}
	}
}
#ifdef __cplusplus
static inline int destory_mempool(mempool *pool,int force=1)
#else
static inline int destory_mempool(mempool *pool,int force)
#endif
{

	realase_chain(&pool->using_block_list,mempool_block,node,free_mpnode);
	realase_chain(&pool->used_block_list,mempool_block,node,free_mpnode);

	memset(pool->cache,0,sizeof(pool->cache));
	memset(pool->cache_count,0,sizeof(pool->cache_count));
	if(pool->flags&mem_flag_lock)
	{
		for(int _c=0;_c<cache_thread_mod;_c++)
			pthread_mutex_destroy(&pool->hash_lock[_c]);
		pthread_mutex_destroy(&pool->lock);
	}
#ifdef MEMPOOL_DEBUG
	fclose(pool->fp);
#endif
	return 0;
}
//创建一个新的4k模式的block，block中每个buf的大小是4k，buf的开头是mp_4k_buf结构体
static inline mempool_block* get_new_block_4k(mempool *pool)
{
	void *addr,*saddr,*eaddr,*_tmp;
	mempool_block * block;
	mp_4k_buf *buf;
	//获取block结构体自身的大小
	unsigned int block_st_size=sizeof(mempool_block)+sizeof(unsigned short)*block_size/4096;
	//获得一个buf中能存放的单元的数量
	if(NULL==(addr=malloc(block_size)))//创建block
		return NULL;
	//取得addr后第一个4k地址
	saddr=(void*)(((unsigned long)addr&(~page_mask))+4096);
	if((char*)saddr-(char*)addr>=block_st_size)//如果addr与saddr之间的空间可以存放block结构体，就将block放在开头
	{
		block=(mempool_block *)addr;
		eaddr=(char*)addr+block_size;
	}
	else if((((unsigned long)((char*)addr+block_size))&(page_mask))>=block_st_size)//如果结尾到结尾前的第一个4k地址之间有足够的空间，就将block结构体放在末尾
	{
		block=(mempool_block*)(((unsigned long)((char*)addr+block_size))&(~page_mask))+1;
		eaddr=block-1;
	}
	else //头和尾都没有足够的空间，就单独创建
	{
		block=(mempool_block*)malloc(block_st_size);
		eaddr=(char*)addr+block_size;
	}
	block->start=saddr;
	block->buf_start=addr;
	block->pool=pool;
	block->all_count=((char*)eaddr-(char*)saddr)/4096;
	
#ifdef MEMPOOL_DEBUG
	fprintf(pool->fp,"create block:%lx,start:%lx,bufcount:%d\n",block,block->start,block->all_count);
#endif
	//初始化block的链表
	for(int i=0;i<block->all_count;i++)
	{
		block->pos_clum[i].node.n=i+1;
		block->pos_clum[i].node.p=i-1;
		buf=((mp_4k_buf*)((char*)block->start+4096*i));

		
		buf->block=block;
		//初始化buf的链表
		_tmp=((uint8_t*)buf)+pool->start_pos;
		buf->first=_tmp;
		for(int j=pool->buf_dt_count-1;j>0;j--)
		{
			*(void**)_tmp=(uint8_t*)_tmp+pool->dt_size;
			_tmp=*(void**)_tmp;
		}
		buf->last=_tmp;
		*(void**)_tmp=NULL;
		buf->c=pool->buf_dt_count;
#ifdef MEMPOOL_DEBUG
		fprintf(pool->fp,"create buf:%lx,first:%lx,last:%lx,dt count:%d\n",buf,buf->first,buf->last,buf->c);
#endif
	}
	block->pos_clum[0].node.p=block->all_count;
	block->pos_clum[block->all_count].node.n=block->pos_clum[block->all_count].node.p=block->all_count;
	block->first_using_pos=0;
	block->first_used_pos=block->all_count;
	block->current_free_count=block->all_count;
	c_insert_in_head(&pool->using_block_list,&block->node);
	return block;
}

static inline void *get_mem(mempool *pool)
{
	void *mem;
	pthread_t tid=0;
	if(pool->flags&mem_flag_lock)//如果带锁，则加锁
		pthread_mutex_lock(&pool->hash_lock[(tid=(pthread_self()&cache_thread_mod))]);
	if(pool->cache[tid]!=NULL)//如果cache中有剩余的，直接从cache中取
	{
		mem=pool->cache[tid];
		pool->cache[tid]=*(void**)pool->cache[tid];
		pool->cache_count[tid]--;
		if(pool->flags&mem_flag_lock)
			pthread_mutex_unlock(&pool->hash_lock[tid]);
		return mem;
	}
	if(pool->flags&mem_flag_lock)
		pthread_mutex_lock(&pool->lock);
	//cache已经为空，填充cache
	int balance=pool->buf_dt_count;
	while(pool->cache_count[tid]<balance)
	{
		mempool_block *block;
		mp_4k_buf * buf;
		//优先从未使用完的block上取单元
		if(!c_is_empty(&pool->using_block_list))
		{
			//获得最近使用的未用完的block
			block=get_first_dt(&pool->using_block_list,mempool_block,node);
			do
			{
				//获得block的空闲链表中的第一个4k buf
				buf=(mp_4k_buf *)((char*)block->start+4096*block->first_using_pos);
#ifdef MEMPOOL_DEBUG
				fprintf(pool->fp,"set all mem from buf:%lx to cache,first:%lx,last:%lx,dt count:%d\n",buf,buf->first,buf->last,buf->c);
#endif
				*((void**)buf->last)=pool->cache[tid];
				pool->cache[tid]=buf->first;
				buf->first=buf->last=NULL;
				pool->cache_count[tid]+=buf->c;
				if(buf->c==pool->buf_dt_count)
					block->current_free_count--;
				buf->c=0;
				block->pos_clum[block->first_used_pos].node.p=block->first_using_pos;
				block->first_using_pos=block->pos_clum[block->first_using_pos].node.n;
				block->pos_clum[block->pos_clum[block->first_used_pos].node.p].node.n=block->first_used_pos;
				block->first_used_pos=block->pos_clum[block->first_used_pos].node.p;
				block->pos_clum[block->first_using_pos].node.p=block->all_count;
				//如果block实际上已经耗尽，则将其加入到已用完链表
				if(block->all_count==block->first_using_pos)
				{
					c_delete_node(&pool->using_block_list,&block->node);
					c_insert_in_head(&pool->used_block_list,&block->node);
					break;
				}
			}while(pool->cache_count[tid]<balance);
		}
		else //using链表已经耗尽，则补充两个block，允许第二个创建失败
		{
			if(NULL==get_new_block_4k(pool))
			{
				if(pool->flags&mem_flag_lock)
				{
					pthread_mutex_unlock(&pool->lock);
					pthread_mutex_unlock(&pool->hash_lock[tid]);
				}
				return NULL;
			}
			get_new_block_4k(pool);
		}
	}
	mem=pool->cache[tid];
	pool->cache[tid]=*(void**)pool->cache[tid];
	pool->cache_count[tid]--;
	if(pool->flags&mem_flag_lock)
	{
		pthread_mutex_unlock(&pool->lock);
		pthread_mutex_unlock(&pool->hash_lock[tid]);
	}
	return mem;
}
static int is_good(mempool *pool)
{
	if(pool->using_block_list.count>2)
		return 1;
	if(pool->buf_dt_count*pool->using_block_list.count>((block_size/4096)*2))
		return 1;
	return 0;
}
static inline void free_mem_small(void *mem)
{
	if(unlikely(mem==NULL))
		return;
	pthread_t tid=0;
	mp_4k_buf * buf=(mp_4k_buf*)((unsigned long)mem&(~page_mask));
	mempool_block * block=buf->block;
	mempool *pool=block->pool;
	if(pool->flags&mem_flag_lock)
	{
		tid=pthread_self()&cache_thread_mod;
		pthread_mutex_lock(&pool->hash_lock[tid]);
	}
	*(void**)mem=pool->cache[tid];
	pool->cache[tid]=mem;
	if(++pool->cache_count[tid]>(int)pool->buf_dt_count*2)
	{
		if(pool->flags&mem_flag_lock)
			pthread_mutex_lock(&pool->lock);
		clear_mp_cache(pool,pool->buf_dt_count,tid);
		if(pool->flags&mem_flag_lock)
			pthread_mutex_unlock(&pool->lock);
	}
	if(pool->flags&mem_flag_lock)
		pthread_mutex_unlock(&pool->hash_lock[tid]);
}
#define free_mem free_mem_small
static inline void init_mempool(mempool *pool,unsigned int flag,unsigned int dt_size,unsigned int align_len)
{
#ifdef MEMPOOL_DEBUG
	char filebuf[32]={0};
	sprintf(filebuf,"%lx.mem.log",pool);
	pool->fp=fopen(filebuf,"w+");
#endif
	c_init_chain(&pool->used_block_list);
	c_init_chain(&pool->using_block_list);
	pool->flags=flag;
	pool->align_len=align_len;
	pool->blocksize=block_size;
	if(align_len>0)
		pool->dt_size=ALIGN(dt_size,align_len);
	else
		pool->dt_size=dt_size;

	pool->buf_dt_count=(4096-sizeof(mp_4k_buf))/(sizeof(unsigned short)+pool->dt_size);
	if(pool->align_len>0)
	{
		if(pool->buf_dt_count*pool->dt_size+ALIGN(pool->buf_dt_count*sizeof(unsigned short)+sizeof(mp_4k_buf),pool->align_len)>4096)
			pool->buf_dt_count--;
		pool->start_pos=ALIGN(pool->buf_dt_count*sizeof(unsigned short)+sizeof(mp_4k_buf),pool->align_len);
	}
	else
		pool->start_pos=pool->buf_dt_count*sizeof(unsigned short)+sizeof(mp_4k_buf);
	memset(pool->cache,0,sizeof(pool->cache));
	memset(pool->cache_count,0,sizeof(pool->cache_count));
	if(pool->flags&mem_flag_lock)
	{
		for(int _c=0;_c<cache_thread_mod;_c++)
			pthread_mutex_init(&pool->hash_lock[_c],NULL);
		pthread_mutex_init(&pool->lock,NULL);
	}
	free_mem_small(get_mem(pool));
}
#endif
/*SRC_CONGO_DRC_LIB_MEMLIB_MEMPOOL_H__MEMPOOL_H_ */
