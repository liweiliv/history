/*
 * page_pool.c
 *
 *  Created on: 2017年5月4日
 *      Author: liwei
 */
#include "page_pool.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#ifndef ALIGN
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif
#ifndef likely
# define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#define MAX_PAGE_NODE 128
#define MAX_PAGE_IN_BLOCK 128
#define MIN_PAGE_IN_BLOCK 16

#define MAX_BLOCK_SIZE (1024*1024*16)

#define MAX_FREE_BLOCK 16
#define DEFAULT_MAX_MEM (256*1204*1024)


static inline page_cache_node * create_page_cache_node()
{
	page_cache_node *pcn = (page_cache_node*) malloc(sizeof(page_cache_node));
	c_init_chain(&pcn->cache);
	pcn->cn.next = pcn->cn.prev = NULL;
	return pcn;
}
/*
 *对链表快速排序
 */
static inline void sort_page_chain(page_cache_node * pcn, pp_page * l, pp_page * r)
{
	if (l != r)
	{
		chain_node *lp = l->cn.prev, *rn = r->cn.next;
		/*
		 *  取链表的第二个做中间数字x，一轮排序完成后，x之前的都小于等于x，x之后的都大于x
		 */
		pp_page * _l = l, *_r = r, *x = (pp_page *) ((char *) ((_l)->cn.next)
				- ((size_t) &((pp_page *) 0)->cn));
		/*将x从链表中摘除*/
		x->cn.prev->next = x->cn.next;
		x->cn.next->prev = x->cn.prev;

		/*
		 * 如果链表只有两个节点，则不用处理，直接根据x与_l的大小来排序
		 */
		if (_r != x)
		{
			/*
			 * 从后往前，找到第一个小于等于x的节点，放到_l的前面
			 */
			while (_l != _r)
			{
				while (_l != _r && _r->page_id > x->page_id)
				{
					_r = get_prev_dt(_r, pp_page, cn);
				}
				if (_l != _r)
				{
					chain_node * _tmp = _r->cn.prev;

					_r->cn.prev->next = _r->cn.next;
					_r->cn.next->prev = _r->cn.prev;

					struct chaint_node *__tmp = (_l->cn.prev)->next;
					(&_r->cn)->prev = (_l->cn.prev);
					(&_r->cn)->next = __tmp;
					(_l->cn.prev)->next = (&_r->cn);
					__tmp->prev = (&_r->cn);

					_r = get_dt_from_chain(_tmp, pp_page, cn);
				}
				/*
				 * 从前往后，找到第一个大于x的节点，放到_r的后面
				 */
				while (_l != _r && _l->page_id <= x->page_id)
				{
					_l = get_next_dt(_l, pp_page, cn);
				}

				if (_l != _r)
				{
					chain_node * _tmp = _l->cn.next;

					_l->cn.prev->next = _tmp;
					_tmp->prev = _l->cn.prev;

					_l->cn.prev = &_r->cn;
					_l->cn.next = _r->cn.next;
					_r->cn.next->prev = &_l->cn;
					_r->cn.next = &_l->cn;

					_l = get_dt_from_chain(_tmp, pp_page, cn);
				}
			}
		}
		/*
		 * 根据x与l的大小决定x与l的顺序
		 */
		if (x->page_id >= _l->page_id)
		{
			struct chaint_node *__tmp = (&_l->cn)->next;
			(&x->cn)->prev = (&_l->cn);
			(&x->cn)->next = __tmp;
			(&_l->cn)->next = (&x->cn);
			__tmp->prev = (&x->cn);
		}
		else
		{
			struct chaint_node *__tmp = (_l->cn.prev)->next;
			(&x->cn)->prev = (_l->cn.prev);
			(&x->cn)->next = __tmp;
			(_l->cn.prev)->next = (&x->cn);
			__tmp->prev = (&x->cn);
		}

		if (x->cn.prev != lp && lp->next != x->cn.prev)
			sort_page_chain(pcn, get_dt_from_chain(lp->next, pp_page, cn), get_prev_dt(x, pp_page, cn));
		if (x->cn.next != rn && x->cn.next != rn->prev)
			sort_page_chain(pcn, get_next_dt(x, pp_page, cn), get_dt_from_chain(rn->prev, pp_page, cn));
	}
}
static inline void sort_page_chain_list(page_cache_node * start,
		page_cache_node *end)
{
	if (start != end)
	{
		page_cache_node *pcn = start;
		while (1)
		{
			c_merge_list(&start->cache, &pcn->cache);
			if (pcn == end)
				break;
			pcn = get_next_dt(pcn, page_cache_node, cn);
		}
	}
	sort_page_chain(start, get_first_dt(&start->cache, pp_page, cn),
			get_last_dt(&start->cache, pp_page, cn));
}

void invalidate_thd_alloced_list(pp_thd *thd)
{
	c_merge_list(&thd->gc_list, &thd->alloced);
}
static inline void clear_gc_list(pp_thd *thd)
{
	if (c_is_empty(&thd->gc_list))
		return;
	chain_node *h = thd->gc_list.head.next, *e = h;
	page_cache_node * rc;
	if (c_is_empty(&thd->cache))
	{
		rc = create_page_cache_node();
		c_insert_in_head(&thd->cache, &rc->cn);
	}
	else
		rc = get_first_dt(&thd->cache, page_cache_node, cn);
	for (;;)
	{
		/*h会加入到rc，所以初始就已经要+1了*/
		rc->cache.count += 1;
		while (rc->cache.count < MAX_PAGE_NODE && e != &thd->gc_list.head)
		{
			e = e->next;
			rc->cache.count++;
		}

		h->prev->next = e->next;
		e->next->prev = h->prev;

		rc->cache.head.next->prev = e;
		e->next = rc->cache.head.next;
		rc->cache.head.next = h;
		h->prev = &rc->cache.head;
		if (!c_is_empty(&thd->cache))
		{
			h = e = thd->gc_list.head.next;
			rc = create_page_cache_node();
			c_insert_in_head(&thd->cache, &rc->cn);
		}
		else
			break;
	}
	/*gc_list已经清空，但清空的方式不标准，直接将其重置*/
	c_init_chain(&thd->gc_list);
}

static inline void _clear_thd_cache(pp_thd *thd, uint32_t max_thd_cache)
{
	while (thd->cache.count > max_thd_cache)
	{
		/* 最近释放的内存命中cpu cache的概率更高，所以优先选择清理掉最早释放的*/
		page_cache_node * rc = get_last_dt(&thd->cache, page_cache_node, cn);
		c_delete_node(&thd->cache, &rc->cn);
		pthread_mutex_lock(&thd->pool->global_page_cache_lock);
		c_insert_in_head(&thd->pool->global_page_cache, &rc->cn);
		pthread_mutex_unlock(&thd->pool->global_page_cache_lock);
	}
	/*
	 * 如果处于高负载下，很可能有线程在等待可用内存，将其唤醒
	 */
	if (thd->pool->high_load.counter > 0)
	{
		pthread_mutex_lock(&thd->pool->lack_cond_mutex);
		pthread_cond_signal(&thd->pool->lack_cond);
		pthread_mutex_unlock(&thd->pool->lack_cond_mutex);
	}
}
static inline void clear_thd_cache(pp_thd *thd)
{
	/*
	 * 如果thd的cache满了，则将最后的page_cache_node放入全局cache
	 */
	page_pool *pool = thd->pool;
	uint32_t max_thd_cache;

	/*高负载下，直接释放到只剩1个节点*/
	if (likely(!pool->high_load.counter))
		max_thd_cache = thd->max_thd_cache < pool->max_thd_cache ?
						thd->max_thd_cache : pool->max_thd_cache;/*如果thd->max_thd_cache更小，以thd->max_thd_cache为准*/
	else
		max_thd_cache = 1;
	_clear_thd_cache(thd, max_thd_cache);
	/*
	 * 如果不是default thd，在全局cache过大时，唤醒清理线程
	 * 清理线程会调用本函数来处理default thd的cache，无需也无法唤醒自己
	 */
	if (thd->tid != 0 && pool->global_page_cache.count > MAX_PAGE_NODE + 5)
	{
		pthread_mutex_lock(&pool->gc_cond_mutex);
		pthread_cond_signal(&pool->gc_cond);
		pthread_mutex_unlock(&pool->gc_cond_mutex);
	}
}

static void * clear_global_cache(void * argv)
{
	page_pool * pool = (page_pool*) argv;
	struct timespec abstime;
	while (!pool->exit)
	{
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec += 1;
		pthread_mutex_lock(&pool->gc_cond_mutex);
		pthread_cond_timedwait(&pool->gc_cond, &pool->gc_cond_mutex, &abstime);
		pthread_mutex_unlock(&pool->gc_cond_mutex);
		if (pool->exit)
			pthread_exit(NULL);
		/*
		 * 将default_thd的缓存全部刷到全局cache
		 */
	//	_clear_thd_cache(pool->default_thd, 0);
		/*
		 * 清理掉过剩的全局cache到using list
		 */
		while (pool->global_page_cache.count > pool->global_page_cache_limit)
		{
			page_cache_node * rc;
			pthread_mutex_lock(&pool->global_page_cache_lock);
			rc = get_first_dt(&pool->global_page_cache, page_cache_node, cn);
			c_delete_node(&pool->global_page_cache, &rc->cn);
			pthread_mutex_unlock(&pool->global_page_cache_lock);
			sort_page_chain_list(rc, rc);
			c_get_list_of_chain(pg,&rc->cache,pp_page,cn)
			{
				printf("%lx %d %d ,",pg->block,pg->block->id,pg->page_id);
			}
			printf("\n");
			page_pool_block * block = NULL;
			pp_page * h = NULL, *page;
			int cnt = 0;
			char end = 0;
			for (chain_node * cn = rc->cache.head.next;; cn = cn->next)
			{
				page = container_of_chain_node(cn, pp_page, cn);
				if (page->block != block)
				{
					if (block != NULL)
					{
DEAL:
						rc->cache.head.next = page->cn.next;
						pthread_mutex_lock(&pool->block_lock_list[block->id & MAX_LOCK_SIZE]);
						block->using_block_list.head.next->prev = page->cn.prev;
						cn->prev->next = block->using_block_list.head.next;
						block->using_block_list.head.next = &h->cn;
						h->cn.prev = &block->using_block_list.head;
						/*
						 *对于目前为空的block，需要从used_block_list移到using_block_list
						 */
						if (block->using_block_list.count == 0)
						{
							block->using_block_list.count += cnt;
							pthread_mutex_lock(&pool->global_block_lock);
							c_delete_node(&pool->used_block_list, &block->cn);
							if (cnt != pool->block_dt_count)
							{
								c_insert_in_head(&pool->using_block_list, &block->cn);
							}
							else /*已满的block放入using_block_list的末尾，方便在内存占用过高时清理*/
							{
								c_insert_in_end(&pool->using_block_list, &block->cn);
							}
							pthread_mutex_unlock(&pool->global_block_lock);
						}
						else
						{
							block->using_block_list.count += cnt;
							if (block->using_block_list.count == pool->block_dt_count)
							{
								pthread_mutex_lock(&pool->global_block_lock);
								c_delete_node(&pool->using_block_list, &block->cn);
								c_insert_in_end(&pool->using_block_list, &block->cn);
								pthread_mutex_unlock(&pool->global_block_lock);
							}
						}
						pthread_mutex_unlock( &pool->block_lock_list[block->id & MAX_LOCK_SIZE]);
					}
					cnt = 0;
					h = page;
					printf("update block form %lx to %lx\n",block,page->block);
					if(page->block<0xffffff)
						abort();
					block = page->block;
				}
				cnt++;
				if (cn->next == &rc->cache.head)
				{
					if (end > 0)
					{
						free(rc);
						break;
					}
					else
					{
						end = 1;
						goto DEAL;
					}
				}
			}
		}

		/*
		 * 从using list末尾开始free，只有满的block可以释放
		 * 在using block数量超过MAX_FREE_BLOCK的情况下，尽力free到MAX_FREE_BLOCK以下，否则一次只free掉一个
		 */
		int free_cnt = 0;
		while (1)
		{
			page_pool_block * block = get_last_dt(&pool->using_block_list,
					page_pool_block, cn);
			if (block == NULL
					|| block->using_block_list.count != pool->block_dt_count)
				break;
			if (pool->using_block_list.count >= MAX_FREE_BLOCK
					|| free_cnt++ == 0)
			{
				pthread_mutex_lock(&pool->global_block_lock);
				if (c_is_end(&pool->using_block_list, &block->cn)
						&& block->using_block_list.count
								== pool->block_dt_count)
				{
					c_delete_node(&pool->using_block_list, &block->cn);
					pthread_mutex_unlock(&pool->global_block_lock);
					free(block);
				}
				else
					pthread_mutex_unlock(&pool->global_block_lock);
			}
			else
				break;

		}
	}
	pthread_exit(NULL);
}
pp_thd * create_thd(page_pool *pool, int max_thd_cache)
{
	pthread_t tid = pthread_self();
	pthread_mutex_lock(&pool->thd_lock);
	c_get_list_of_chain(thd,&pool->thd_head,pp_thd,cn)
	{
		if (thd->tid == tid)
		{
			pthread_mutex_unlock(&pool->thd_lock);
			return thd;
		}
	}
	pthread_mutex_unlock(&pool->thd_lock);
	pp_thd *thd = (pp_thd*) malloc(sizeof(pp_thd));
	c_init_chain(&thd->alloced);
	c_init_chain(&thd->cache);
	c_init_chain(&thd->gc_list);
	thd->pool = pool;
	thd->tid = tid;
	thd->max_thd_cache = max_thd_cache;
	pthread_mutex_lock(&pool->thd_lock);
	c_insert_in_head(&pool->thd_head, &thd->cn);
	pthread_mutex_unlock(&pool->thd_lock);
	return thd;
}
void destroy_thd(pp_thd * thd)
{
	pthread_t tid = pthread_self();
	if (tid != thd->tid)
		return;
	clear_gc_list(thd);
	_clear_thd_cache(thd, 0);
	page_pool *pool = thd->pool;

	pthread_mutex_lock(&pool->default_thd_lock);
	c_merge_list(&pool->default_thd->alloced, &thd->alloced);
	pthread_mutex_unlock(&pool->default_thd_lock);

	c_get_list_of_chain(pg,&thd->alloced,pp_page,cn)
	{
		pg->thd = pool->default_thd;
	}
	pthread_mutex_lock(&pool->thd_lock);
	c_delete_node(&pool->thd_head, &thd->cn);
	pthread_mutex_unlock(&pool->thd_lock);
	free(thd);

}
int init_page_pool(page_pool *p, size_t page_size, int align, uint64_t max_mem,
		int max_thd_cache, int global_page_cache_limit)
{
	p->align_len = align;
	if (align > 0)
		p->dt_size = ALIGN(page_size + offsetof(pp_page,data), align);
	else
		p->dt_size = page_size + offsetof(pp_page, data);
	if (p->dt_size * MAX_PAGE_IN_BLOCK + offsetof(page_pool_block, buf)
			< MAX_BLOCK_SIZE)
		p->block_size = p->dt_size * MAX_PAGE_IN_BLOCK
				+ offsetof(page_pool_block, buf);
	else
		p->block_size = ((MAX_BLOCK_SIZE - offsetof(page_pool_block, buf)) / p->dt_size) * p->dt_size + offsetof(page_pool_block, buf);
	p->block_dt_count = (p->block_size - offsetof(page_pool_block, buf)) / p->dt_size;
	if (max_mem != 0)
		p->max_mem = max_mem;
	else
		p->max_mem = DEFAULT_MAX_MEM;
	atomic_set(&p->global_block_id, 0);
	pthread_mutex_init(&p->global_block_lock, NULL);
	pthread_mutex_init(&p->global_page_cache_lock, NULL);
	c_init_chain(&p->using_block_list);
	c_init_chain(&p->used_block_list);
	c_init_chain(&p->thd_head);
	c_init_chain(&p->global_page_cache);
	p->global_page_cache_limit = global_page_cache_limit;
	p->exit = 0;
	atomic_set(&p->high_load, 0);
	pthread_cond_init(&p->gc_cond, NULL);
	pthread_mutex_init(&p->gc_cond_mutex, NULL);

	pthread_cond_init(&p->lack_cond, NULL);
	pthread_mutex_init(&p->lack_cond_mutex, NULL);
	pthread_create(&p->gc_thread_id, NULL, clear_global_cache, (void*) p);
	pthread_mutex_init(&p->thd_lock, NULL);
	p->max_thd_cache = max_thd_cache;

	p->default_thd = create_thd(p, 1);
	p->default_thd->tid = 0;
	pthread_mutex_init(&p->default_thd_lock, NULL);

	for (int i = 0; i < MAX_LOCK_SIZE+1; i++)
		pthread_mutex_init(&p->block_lock_list[i], NULL);
#ifdef MEMDEBUG
	char buf[64] =
	{ 0 };
	sprintf(buf, "0X%lX_mem.log", p);
	p->fp = fopen(buf, "a+");
	p->allocCount = 0;
	remove("m.log");
	init_batch_log(&p->pool_log, "m.log", 128 * 1024);
#endif
	return 0;
}

void destory_page_pool(page_pool *pool)
{
	pthread_mutex_lock(&pool->gc_cond_mutex);
	pool->exit = 1;
	pthread_cond_signal(&pool->gc_cond);
	pthread_mutex_unlock(&pool->gc_cond_mutex);
	pthread_join(pool->gc_thread_id, NULL);

	/*
	 * 防止有线程在等待lack_cond
	 */
	while (atomic_read(&pool->high_load) > 0)
	{
		pthread_mutex_lock(&pool->lack_cond_mutex);
		pthread_cond_signal(&pool->lack_cond);
		pthread_mutex_unlock(&pool->lack_cond_mutex);
		usleep(10);
	}

	realase_chain(&pool->using_block_list, page_pool_block, cn, free);
	realase_chain(&pool->used_block_list, page_pool_block, cn, free);
	realase_chain(&pool->global_page_cache, page_cache_node, cn, free);
	c_get_list_of_chain(thd,&pool->thd_head,pp_thd,cn)
	{
		realase_chain(&thd->cache, page_cache_node, cn, free);
	}
	realase_chain(&pool->thd_head, pp_thd, cn, free);

	pthread_mutex_destroy(&pool->gc_cond_mutex);
	pthread_cond_destroy(&pool->gc_cond);

	pthread_mutex_destroy(&pool->lack_cond_mutex);
	pthread_cond_destroy(&pool->lack_cond);

	pthread_mutex_destroy(&pool->global_block_lock);
	pthread_mutex_destroy(&pool->global_page_cache_lock);
	pthread_mutex_destroy(&pool->default_thd_lock);
	pthread_mutex_destroy(&pool->thd_lock);
	for (int i = 0; i < MAX_LOCK_SIZE+1; i++)
		pthread_mutex_destroy(&pool->block_lock_list[i]);
#ifdef MEMDEBUG
	fclose(pool->fp);
	close_batch_log(&pool->pool_log);
#endif
}

static inline page_pool_block * create_new_block(page_pool *pool)
{
	pp_page *page;
	/*负载高，无法创建*/
	if ((pool->using_block_list.count + pool->used_block_list.count) * pool->block_size > pool->max_mem)
		return NULL;
	page_pool_block * ppb = (page_pool_block *) malloc(pool->block_size);
	if (ppb == NULL)
		return NULL;
	unsigned long end = ((unsigned long) ppb) + pool->block_size - pool->dt_size;
	uint32_t  page_id=0;
	ppb->p = pool;
	ppb->id = atomic_read(&pool->global_block_id);
	atomic_inc(&pool->global_block_id);
	c_init_chain(&ppb->using_block_list);
	page = (pp_page*) (void*) ALIGN((unsigned long )&ppb->buf, pool->align_len);
	do
	{
		c_insert_in_head(&ppb->using_block_list, &page->cn);
		page->block = ppb;
		page->page_id = (ppb->id<<12)+page_id++;
		page = (pp_page *) (((long) page) + pool->dt_size);
	} while (((unsigned long) page) < end);

#ifdef MEMDEBUG
	fprintf(pool->fp,
			"%d,alloc.dt_size:%d,using_block:%d,used_block:%d,all mem:%d,mem used:%d\n",
			time(NULL), pool->dt_size, pool->using_block_list.count,
			pool->used_block_list.count,
			pool->block_size
					* (pool->using_block_list.count
							+ pool->used_block_list.count),
			pool->allocCount * pool->dt_size);
#endif

	return ppb;
}
void * alloc_page(pp_thd *thd)
{
	page_pool *pool = thd->pool;
	page_cache_node * rc = NULL, *tmp = NULL;
	uint8_t high_load = 0;
	GET_PAGET_CACHE_NODE: clear_gc_list(thd);
	if (c_is_empty(&thd->cache))
	{
		/*
		 * 尝试从global_page_cache获取一个node
		 */
		pthread_mutex_lock(&pool->global_page_cache_lock);
		if (c_is_empty(&pool->global_page_cache))
		{
			/*
			 * 全局global_page_cache为空，则从using_block_list中直接获取
			 */
			pthread_mutex_unlock(&pool->global_page_cache_lock);
			if (tmp == NULL)
			{
				rc = create_page_cache_node();
			}
			else
				rc = tmp;
			/*
			 * 希望每个node至少要填充MAX_PAGE_NODE个page
			 */
			while (rc->cache.count < MAX_PAGE_NODE)
			{
				page_pool_block * block;
				pthread_mutex_lock(&pool->global_block_lock);
				if (c_is_empty(&pool->using_block_list))
				{
					pthread_mutex_unlock(&pool->global_block_lock);
					/*如果using_block_list已经为空，则malloc一个*/
					if (NULL == (block = create_new_block(pool)))
					{
						/*分配失败，如果已经有可用的，则将就使用，否则开始重试，用tmp保存下指针，避免重复释放、分配*/
						if (rc->cache.count == 0)
						{
							tmp = rc;
							/*分配失败，设置高负载*/
							if (high_load == 0)
								atomic_inc(&pool->high_load);

							/*只允许重试128次*/
							if (!pool->exit && high_load++ < 128)
								goto GET_PAGET_CACHE_NODE;
							else
							{
								free(tmp);
								atomic_dec(&pool->high_load);
								return NULL;
							}

							/*等待被唤醒，或者在1ms超时后继续重试*/
							struct timespec abstime;
							clock_gettime(CLOCK_REALTIME, &abstime);
							abstime.tv_nsec += 1000000;
							if (abstime.tv_nsec >= 1000000000)
							{
								abstime.tv_sec++;
								abstime.tv_nsec -= 1000000000;
							}
							/*
							 *  其他线程在cache较高或者高负载模式下会释放cache，如果high_load大于0，就会唤醒等待资源的cache
							 */
							pthread_mutex_lock(&pool->lack_cond_mutex);
							pthread_cond_timedwait(&pool->lack_cond,
									&pool->lack_cond_mutex, &abstime);
							pthread_mutex_unlock(&pool->lack_cond_mutex);
						}
						else
							break;
					}
					c_merge_list(&rc->cache, &block->using_block_list);
				}
				else
				{
					block = get_first_dt(&pool->using_block_list,
							page_pool_block, cn);
					c_delete_node(&pool->using_block_list, &block->cn);
					pthread_mutex_unlock(&pool->global_block_lock);
					if (block->using_block_list.count == pool->block_dt_count)
						c_merge_list(&rc->cache, &block->using_block_list);
					else
					{
						/*
						 *  对于已经有部分page在使用中的block，需要加锁，以防其他线程释放page到block中
						 */
						pthread_mutex_lock( &pool->block_lock_list[block->id & MAX_LOCK_SIZE]);
						c_merge_list(&rc->cache, &block->using_block_list);
						pthread_mutex_unlock( &pool->block_lock_list[block->id & MAX_LOCK_SIZE]);
					}
				}
				/*
				 * 将block放入used_block_list中
				 */
				pthread_mutex_lock(&pool->global_block_lock);
				c_insert_in_head(&pool->used_block_list, &block->cn);
				pthread_mutex_unlock(&pool->global_block_lock);
			}
		}
		else
		{
			rc = get_first_dt(&pool->global_page_cache, page_cache_node, cn);
			c_delete_node(&pool->global_page_cache, &rc->cn);
			pthread_mutex_unlock(&pool->global_page_cache_lock);
		}
		c_insert_in_head(&thd->cache, &rc->cn);
	}
	else
	{
		/*如果在高负载下，其他thd已经无法再分配到内存，需要减低当前线程的缓存量*/
		if (thd->cache.count > 1 && unlikely(thd->pool->high_load.counter))
			_clear_thd_cache(thd, 1);
		rc = get_first_dt(&thd->cache, page_cache_node, cn);
	}

	if (c_is_empty(&rc->cache))
	{
		c_delete_node(&thd->cache, &rc->cn);
		free(rc);
		rc = NULL;
		goto GET_PAGET_CACHE_NODE;
	}

	pp_page *page = get_first_dt(&rc->cache, pp_page, cn);
	c_delete_node(&rc->cache, &page->cn);
	c_insert_in_head(&thd->alloced, &page->cn);
	page->thd = thd;

	if (tmp != NULL && tmp != rc)
		free(tmp);
	if (high_load)
		atomic_dec(&thd->pool->high_load);
	// printf("page :%lx\n",page);
	return page->data;
}
void free_page(void * data)
{
	pp_page *page = container_of_chain_node((const char (*)[1] )data, pp_page,
			data);
	pp_thd *thd = page->thd;
	page_cache_node * rc = NULL;

	clear_gc_list(thd);

	c_delete_node(&thd->alloced, &page->cn);

	if (c_is_empty(&thd->cache))
		goto PUT_IN_NEW_NODE;
	else
	{
		/*在高负载下，其他thd已经无法再分配到内存，需要减低当前线程的缓存量*/
		if (thd->cache.count > 1 && unlikely(thd->pool->high_load.counter))
		{
			_clear_thd_cache(thd, 1);
		}
		rc = get_first_dt(&thd->cache, page_cache_node, cn);
		/*
		 * 头部的page_cache_node已满，将创建新的page_cache_node来存放page
		 */
		if (rc->cache.count >= MAX_PAGE_NODE)
		{
			clear_thd_cache(thd);
			goto PUT_IN_NEW_NODE;
		}
		c_insert_in_head(&rc->cache, &page->cn);
	}

	return;
	PUT_IN_NEW_NODE: rc = create_page_cache_node();
	c_insert_in_head(&thd->cache, &rc->cn);
	c_insert_in_head(&rc->cache, &page->cn);
}

