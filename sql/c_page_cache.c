/*
 * c_page_cache.c
 *
 *  Created on: 2017年3月27日
 *      Author: liwei
 */
#include <stdlib.h>
#include <stdint.h>
#include "../util/atomic.h"
#include "spinlock.h"
#define PAGE_LEVEL_HIGH_OFF 8
#define PAGE_LEVEL_MID_OFF  10
#define PAGE_LEVEL_LOW_OFF  14
#define PAGE_LEVEL_MID_MASK  0x00000fff
#define PAGE_LEVEL_LOW_MASK  0x00000fff
/*
 * high和mid用来放block id，low存放page id，以默认的128k page计算，最高可存放512TB的压缩前数据。每个block最高允许存放2G的的数据
 * block id会循环使用，并不保证严格递增
 */
#define GET_PAGE(p,v) (p)->child_level[(v)>>PAGE_LEVEL_LOW_OFF]!=NULL?(p)->child_level[(v)>>PAGE_LEVEL_LOW_OFF].page[(v)&PAGE_LEVEL_LOW_MASK]:
typedef struct _page_level
{
    unsigned int size;
    atomic_t user;
    arch_spinlock_t lock;
    void * level_value;
    union
    {
        struct _page_level *child_level[1];
        const char * page[1];
    };
}page_level;
extern const char* load_page(uint32_t page_id);
extern const char* release_page(uint32_t page_id,const char *page);

#define INIT_LEVEL(l,s) \
do {\
    (l)=(page_level*)malloc(sizeof(page_level)+sizeof(struct _page_level *)*(s));\
    memset((l)->child_level,0,sizeof(struct _page_level *)*(s));\
    (l)->size=1;\
    (l)->user.counter=1; \
    (l)->lock.head_tail=0;\
    }while(0);

volatile int purse_get_page=0;
const char* get_page( page_level *p, uint32_t page_id)
{
    uint32_t page_high_level=page_id>>(PAGE_LEVEL_LOW_OFF+PAGE_LEVEL_MID_OFF);
    uint32_t page_mid_level=(page_id>>PAGE_LEVEL_LOW_OFF)&PAGE_LEVEL_MID_MASK;

    uint32_t page_low_level=page_id&PAGE_LEVEL_LOW_MASK;
    page_level *high_level,*mid_level;
    const char* page;
    if((high_level=p->child_level[page_high_level])==NULL)
    {
        if((page=load_page(page_id))==NULL)
            return NULL;
        INIT_LEVEL(high_level,1<<PAGE_LEVEL_HIGH_OFF);
        INIT_LEVEL(mid_level,1<<PAGE_LEVEL_MID_OFF);
        high_level->child_level[page_mid_level]=mid_level;
        mid_level->page[page_low_level]=page;
        arch_spin_lock(&p->lock);
        if(p->child_level[page_high_level]!=NULL)
        {
            page_level *tmp=p->child_level[page_high_level];
            arch_spin_unlock(&p->lock);
            free(high_level);
            free(mid_level);
            release_page(page,page_id);
            high_level=tmp;
        }
        else
        {
            p->child_level[page_high_level]=high_level;
            arch_spin_unlock(&p->lock);
            return page;
        }
    }
    if((mid_level=high_level->child_level[page_mid_level])==NULL)
    {
        if((page=load_page(page_id))==NULL)
            return NULL;
        INIT_LEVEL(mid_level,1<<PAGE_LEVEL_MID_OFF);
        mid_level->page[page_low_level]=page;
        arch_spin_lock(&high_level->lock);
        if(high_level->child_level[page_mid_level]!=NULL)
        {
            page_level *tmp=high_level->child_level[page_mid_level];
            arch_spin_unlock(&high_level->lock);
            free(mid_level);
            release_page(page,page_id);
            mid_level=tmp;
        }
        else
        {
            high_level->child_level[page_mid_level]=mid_level;
            arch_spin_unlock(&high_level->lock);
            return page;
        }
    }
    if((page=mid_level->page[page_low_level])==NULL)
    {
        if((page=load_page(page_id))==NULL)
            return NULL;
        arch_spin_lock(&mid_level->lock);
        if(mid_level->page[page_low_level]!=page)
        {
            const char *tmp=mid_level->page[page_low_level];
            arch_spin_unlock(&mid_level->lock);
            release_page(page,page_id);
            return tmp;
        }
        else
        {
            mid_level->page[page_low_level]=page;
            arch_spin_unlock(&mid_level->lock);
            return page;
        }
    }
    return page;
}


