/*
 * skip_list.h
 *
 *  Created on: 2017年11月2日
 *      Author: liwei
 */

#ifndef G_SKIP_LIST_H_
#define G_SKIP_LIST_H_
#include "atomic_pointer.h"
#include "random.h"
#include "MempRing.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#define kMaxHeight  12
typedef struct _skip_list_node
{
    int64_t key;
    void * value;
    atomic_pointer next[1];
}skip_list_node;
static inline skip_list_node * next(skip_list_node * node,int n)
{
    atomic_pointer _tmp;
    Acquire_Load(node->next[n],_tmp);
    return (skip_list_node*)_tmp;
}
static inline void set_next(skip_list_node * node,int n, skip_list_node * x)
{
    Release_Store(node->next[n],x);
}
static inline skip_list_node * nobarrier_next(skip_list_node * node,int n)
{
  return (skip_list_node*)NoBarrier_Load(node->next[n]);
}
static inline  void nobarrier_set_next(skip_list_node * node,int n, skip_list_node * x)
{
    NoBarrier_Store(node->next[n],x);
}
typedef struct _skip_list
{
    int flag;
    skip_list_node  * head;
    atomic_pointer max_height;   // Height of the entire list
    memp_ring * mp;
    t_random rnd;
}skip_list;
typedef struct _skip_list_iterator
{
    skip_list * skip;
    skip_list_node * node;
}skip_list_iterator;
#define NewSkipListNode(skip_list,height) ((skip_list_node*)ring_alloc(((skip_list)->mp),sizeof(skip_list_node)+sizeof(atomic_pointer)*((height)-1)))
#define KeyIsAfterNode(k,node) ((node)!=NULL&&(node)->key<(k))
#define SKIP_ITER_VALID(i) ((i)->node!=NULL)
void init_skip_list(skip_list * skip_list,memp_ring * mp)
{
    NoBarrier_Store(skip_list->max_height,(void*)1);
    skip_list->flag = 0;
    if(mp == NULL)
    {
        skip_list->mp = (memp_ring*)malloc(sizeof(memp_ring));
        init_memp_ring(skip_list->mp,1024*512);
        skip_list->flag |= 0x01;
    }
    else
    {
        skip_list->mp = mp;
        skip_list->flag &= (~(int)0x01);
    }
    skip_list->head = NewSkipListNode(skip_list,kMaxHeight);
    skip_list->head->key = 0;
    skip_list->head->value = NULL;
    init_random(skip_list->rnd,0xdeadbeef);
}
void clear_skip_list(skip_list * skip_list)
{
    ring_clear(skip_list->mp,2);
    NoBarrier_Store(skip_list->max_height,(void*)1);
    skip_list->head = NewSkipListNode(skip_list,kMaxHeight);
    skip_list->head->key = 0;
    skip_list->head->value = NULL;
    init_random(skip_list->rnd,0xdeadbeef);
}
void destroy_skip_list(skip_list * skip_list)
{
    if(skip_list->flag & 0x01)
        destroy_memp_ring(skip_list->mp);
    else
        ring_clear(skip_list->mp,1);
}
static inline int random_height(skip_list * skip_list)
{
    // Increase height with probability 1 in kBranching
#define kBranching  4
    int height = 1;
    while (height < kMaxHeight)
    {
        next_random(skip_list->rnd);
        if(skip_list->rnd%kBranching==0)
            height++;
        else
            break;
    }
    return height;
}
static inline skip_list_node * find_greater_or_equal(skip_list * skip_list,int64_t key, skip_list_node** prev)
{
    skip_list_node* x = skip_list->head;
    int level = (uint64_t) NoBarrier_Load(skip_list->max_height) -1;
    while (1)
    {
        skip_list_node* n = next(x, level);
        if (KeyIsAfterNode(key, n))
        {
            // Keep searching in this list
            x = n;
        }
        else
        {
            if (prev != NULL)
                prev[level] = x;
            if (level == 0)
            {
                return n;
            }
            else
            {
                // Switch to next list
                level--;
            }
        }
    }
}
skip_list_node * find_less_than(skip_list * skip_list, int64_t key)
{
    skip_list_node* x = skip_list->head;
    int level = (uint64_t) NoBarrier_Load(skip_list->max_height) - 1;
    while (1)
    {
        skip_list_node* n = next(x,level);
        if (n == NULL || n->key>=key)
        {
            if (level == 0)
            {
                return x;
            }
            else
            {
                // Switch to next list
                level--;
            }
        }
        else
        {
            x = n;
        }
    }
}
skip_list_node * find_last(skip_list * skip_list)
{
    skip_list_node* x = skip_list->head;
    int level = (uint64_t) NoBarrier_Load(skip_list->max_height) - 1;
    while (true)
    {
        skip_list_node* n = next(x,level);
        if (n == NULL)
        {
            if (level == 0)
            {
                return x;
            }
            else
            {
                // Switch to next list
                level--;
            }
        }
        else
        {
            x = n;
        }
    }
}
static inline skip_list_node * find_first(skip_list * skip_list)
{
    return next(skip_list->head,0);
}
static void * insert(skip_list * skip_list, int64_t key, void * value)
{
    skip_list_node * prev[kMaxHeight];
    skip_list_node * x = find_greater_or_equal(skip_list, key, prev);
    if (x != NULL && x->key == key)
        return x->value;
    int height = random_height(skip_list);
    if (height > (int64_t) NoBarrier_Load(skip_list->max_height))
    {
        for (int i = (uint64_t) NoBarrier_Load(skip_list->max_height);
                i < height; i++)
        {
            prev[i] = skip_list->head;
        }
        //fprintf(stderr, "Change height from %d to %d\n", max_height_, height);

        // It is ok to mutate max_height_ without any synchronization
        // with concurrent readers.  A concurrent reader that observes
        // the new value of max_height_ will see either the old value of
        // new level pointers from head_ (NULL), or a new value set in
        // the loop below.  In the former case the reader will
        // immediately drop to the next level since NULL sorts after all
        // keys.  In the latter case the reader will use the new node.
        NoBarrier_Store(skip_list->max_height, (void*)(uint64_t)(height));
    }
    x = NewSkipListNode(skip_list, height);
    x->key = key;
    x->value = value;
    for (int i = 0; i < height; i++)
    {
        // NoBarrier_SetNext() suffices since we will add a barrier when
        // we publish a pointer to "x" in prev[i].
        nobarrier_set_next(x,i, nobarrier_next(prev[i],i));
        set_next(prev[i],i, x);
    }
    return NULL;
}
static inline void * find(skip_list * skip_list, int64_t key)
{
    skip_list_node * node = find_greater_or_equal(skip_list,key,NULL);
    if(node == NULL || node->key!=key)
        return NULL;
    else
        return node->value;
}
#define skip_list_iterator_valid(iter) ((iter)->node!=NULL)
static int seek(skip_list * skip_list, int64_t key,skip_list_iterator * iterator)
{
    skip_list_node * node =  find_greater_or_equal(skip_list,key,NULL);
    iterator->skip = skip_list;
    iterator->node = node;
    return 0;
}
static void* next(skip_list_iterator * iterator)
{
    assert(iterator->node);
    iterator->node = next(iterator->node,0);
    if(iterator->node)
        return iterator->node->value;
    else
        return NULL;
}
static void* prev(skip_list_iterator * iterator)
{
    assert(iterator->node);
    iterator->node = next(iterator->node,0);
    if(iterator->node)
        return iterator->node->value;
    else
        return NULL;
}
static int begin(skip_list * skip_list,skip_list_iterator * iterator)
{
    skip_list_node * node = find_first(skip_list);
    if(node==NULL)
    {
        iterator->node = NULL;
        iterator->skip = NULL;
        return -1;
    }
    else
    {
        iterator->node = node;
        iterator->skip = skip_list;
        return 0;
    }
}
static int rbegin(skip_list * skip_list,skip_list_iterator * iterator)
{
    skip_list_node * node = find_last(skip_list);
    if(node==NULL)
    {
        iterator->node = NULL;
        iterator->skip = NULL;
        return -1;
    }
    else
    {
        iterator->node = node;
        iterator->skip = skip_list;
        return 0;
    }
}
#endif /* G_SKIP_LIST_H_ */
