/*
 * MempRing.cpp
 *
 * Created on: 2016年1月21日
 * Author: liwei
 */

#include "MempRing.h"
#include <pthread.h>
#include <string.h>
#ifndef likely
# define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)    __builtin_expect(!!(x), 0)
#endif
#define MR_DEFAULT_BUF_SIZE (1024*1024*16)
int init_memp_ring(memp_ring* p,size_t block_size)
{
    c_init_chain(&p->head);
    if(block_size ==0)
    	p->block_size = MR_DEFAULT_BUF_SIZE;
    else
    	p->block_size = block_size;
    if(NULL==(p->head_node=(memp_ring_node*)malloc(offsetof(memp_ring_node,buf)+p->block_size)))
        return -1;
    c_insert_in_head(&p->head,&p->head_node->cn);
    p->head_node->buf_size=p->block_size;
    p->head_node->hole=NULL;

    p->end_node=p->head_node;
    p->end_pos=p->head_pos=p->head_node->buf;
    p->qe=p->qh=0;
    p->lock=malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init((pthread_mutex_t *)p->lock,NULL);
    return 0;
}
void destroy_memp_ring(memp_ring* p)
{
    realase_chain(&p->head,memp_ring_node,cn,free);
    p->end_node=p->head_node=NULL;
    p->qe=p->qh=0;
    p->end_pos=p->head_pos=NULL;
    pthread_mutex_destroy((pthread_mutex_t *)p->lock);
    free(p->lock);
}
void * ring_alloc(memp_ring *p ,size_t size)
{
    /*需要分配的内存大小必须包括sizeof(size_t)的长度描述和申请的长度*/
    size_t buf_size=size+sizeof(size_t);
    /*如果头部的node的剩余空间够用，则直接从头部获取*/
    if(likely(p->head_node->buf_size-((unsigned long)p->head_pos-(unsigned long)p->head_node->buf)>buf_size))
    {
        char * b=p->head_pos+sizeof(size_t);
        *(size_t*)p->head_pos=size;
        p->head_pos=b+size;
        return b;
    }
    else /*头部空间不足，尝试从下一个node取*/
    {
        memp_ring_node * n;
        pthread_mutex_lock((pthread_mutex_t*)p->lock);
        if(c_is_end(&p->head,&p->head_node->cn))
        {
            n=get_first_dt(&p->head,memp_ring_node,cn);
        }
        else
        {
            n=get_next_dt(p->head_node,memp_ring_node,cn);
        }
        /*如果下一个node是尾部node，则创建一个新的node，不允许继续在尾部的前半截分配，避免头部与尾部在同一个node里，而且头在尾之前时，遇到剩余空间不足必须新分配node的情况*/
        if(n==p->end_node)
        {
            if(buf_size<p->block_size)
                buf_size=p->block_size;
        }
        else /*head之后的下一个node可以用*/
        {

            if(buf_size<p->block_size&&n->buf_size>p->block_size)/*如果空间太大，则收缩到默认值*/
                buf_size=p->block_size;
            else if(buf_size<=n->buf_size) /*如果空间不足，则直接使用原始的buf_size=size+sizeof(size_t)创建一个刚刚够用的node替换 ;否则直接使用next node*/
                goto nalloc;
            c_delete_node(&p->head,&n->cn);
            free(n);
        }
        n=(memp_ring_node*)malloc(offsetof(memp_ring_node,buf)+buf_size);
        if(n==NULL)
            return NULL;
        n->buf_size=buf_size;
        n->hole=NULL;
        c_insert_aft(&p->head,&p->head_node->cn,&n->cn);
nalloc:
        p->head_node->hole=p->head_pos;
        p->head_node=n;
        p->head_pos=n->buf+size+sizeof(size_t);
        *(size_t*)(void*)n->buf=size;
        pthread_mutex_unlock((pthread_mutex_t*)p->lock);
        return n->buf+sizeof(size_t);
    }
}
static inline void check_hole(memp_ring *p)
{
    while(p->end_node->hole==p->end_pos)
    {
        memp_ring_node * n;
        pthread_mutex_lock((pthread_mutex_t*)p->lock);
        if(p->end_node->hole!=p->end_pos)
        {
            pthread_mutex_unlock((pthread_mutex_t*)p->lock);
            return;
        }
        p->end_node->hole=NULL;
        /*如果end的前一个不是head，则说明包括end在内至少有两个空闲的node，释放掉，节省内存*/
        if(c_is_end(&p->head,&p->end_node->cn))
        {
            n=get_first_dt(&p->head,memp_ring_node,cn);
        }
        else
        {
            n=get_next_dt(p->end_node,memp_ring_node,cn);
        }
        if(c_is_head(&p->head,&p->end_node->cn))
        {
            if(get_last_dt(&p->head,memp_ring_node,cn)!=p->head_node)
            {
                c_delete_node(&p->head,&p->end_node->cn);
                free(p->end_node);
            }
        }
        else
        {
            if(get_prev_dt(p->end_node,memp_ring_node,cn)!=p->head_node)
            {
                c_delete_node(&p->head,&p->end_node->cn);
                free(p->end_node);
            }
        }
       /*end转移到下一个node*/
       p->end_node=n;
       p->end_pos=n->buf;
       pthread_mutex_unlock((pthread_mutex_t*)p->lock);
    }
}
void ring_free(memp_ring *p,void * v)
{
    if(v==NULL)
        return;
    v=((char*)v)-sizeof(size_t);
    size_t size=*((size_t*)v)+sizeof(size_t);
    /*如果释放的内存不是end，加入到hole队列里*/
    if(v!=p->end_pos)
    {
        check_hole(p);
        if(v!=p->end_pos)
        {
            p->q[p->qh].pos=((char*)v);
            p->q[p->qh].size=size;
            p->qh=(p->qh+1)%1024;
            return;
        }
    }
    p->end_pos+=size;
    /*判断end_pos是否达到node的尾部，每个node的尾部都可能有一截不够用的空洞*/
    check_hole(p);
    /*从前往后遍历hole queue直到无法合并*/
    while(p->qe!=p->qh)
    {
        if(p->q[p->qe].pos==p->end_pos)
        {
            p->end_pos+=p->q[p->qe].size;
            check_hole(p);
            p->qe=(p->qe+1)%1024;
        }
        else
            break;
    }
}
/*
 * 只允许对刚刚分配的，位于顶端的内存进行重分配
 * 如果是扩张，保证现有的内容不变
 * addr是待重分配的内存地址，size是新的长度
 */
void * ring_realloc(memp_ring *p ,void *addr,size_t size)
{
    if(p->head_node->buf>(char*)addr||p->head_pos<(char*)addr)
        return NULL;
    size_t *p_current_size=(size_t*)((char*)addr-sizeof(size_t));
    if(p->head_pos!=(char*)addr+*p_current_size)
        return NULL;
    /*需要扩张*/
    if(*p_current_size<=size)
    {
        size_t current_size=*p_current_size;
        p->head_pos=(char*)p_current_size;
        char * new_addr=(char*)ring_alloc(p,size);
        if(new_addr!=(char*)addr)
            memcpy(new_addr,addr,current_size);
        return new_addr;
    }
    /*需要缩小*/
    else
    {
        *p_current_size=size;
        p->head_pos=(char*)addr+size;
        return addr;
    }
}

