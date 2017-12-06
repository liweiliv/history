/*
 * quadtree.h
 *
 *  Created on: 2017年12月4日
 *      Author: liwei
 */

#ifndef G_QUADTREE_H_
#define G_QUADTREE_H_
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "mempool.h"
#define QT_MASK_OFF 60
#define QT_MASK (15UL<<QT_MASK_OFF)
#define QT_STEP (1UL<<QT_MASK_OFF)

#define QT_LEVEL(id) (((id)&QT_MASK)>>QT_MASK_OFF)
#define QT_GET_IDX(id) ((id)>>((0x0f-QT_LEVEL(id))*4)&0x0f)

typedef struct _quadtree_node
{
    void * v;
    uint64_t id;
    struct _quadtree_node* child[4];
    struct _quadtree_node * parent;
}quadtree_node;

typedef struct _quadtree
{
    quadtree_node root;
    mempool mp;
    void (*destroy_value) (void*);
}quadtree;

void init_quadtree(quadtree * tree,void (*destroy_value) (void*))
{
    init_mempool(&tree->mp,mem_flag_lock,sizeof(quadtree_node),8);
    memset(&tree->root,0,sizeof(tree->root));
    tree->destroy_value = destroy_value;
}

static inline uint64_t gen_Id(uint64_t pid,uint8_t idx)
{
    assert(idx<4);
    uint64_t id = idx;
    int off = QT_LEVEL(pid);
    assert(off<=0x0f);
    off = (0x0f-off)<<2;
    assert(((pid>>off)<<off)==pid);
    id<<=(off-4);
    id|=pid;
    id+=QT_STEP;
    return id;
}
static inline uint64_t genId_without_check(uint64_t pid,uint8_t idx)
{
    uint64_t id = idx;
    id<<=((0x0f-QT_LEVEL(pid)+1)<<2);
    id|=pid;
    id+=QT_STEP;
    return id;
}
quadtree_node * find_node(quadtree * tree,uint64_t id)
{
    quadtree_node * n = &tree->root;
    uint8_t level = QT_LEVEL(id);
    id&=~QT_MASK;
    for(uint8_t off = level;off>0;off--)
    {
        if(NULL == (n = n->child[(id>>((0x0f-off)<<2))&0x0f]))
            return NULL;
    }
    return n;
}
void * find(quadtree * tree,uint64_t id)
{
    quadtree_node * n = find_node(tree,id);
    if(n==NULL)
        return NULL;
    return n->v ;
}
uint64_t insert(quadtree * tree,quadtree_node * n,uint8_t idx,void *value,void ** exist_value)
{
    assert(idx<4);
    uint64_t id = gen_Id(n->id,idx);
    if(n->child[idx]!=NULL)
    {
        exist_value!=NULL?*exist_value = n->child[idx]:0;
        return 0;
    }
    n->child[idx] = (quadtree_node*)get_mem(&tree->mp);
    quadtree_node * new_node = n->child[idx];
    memset(new_node->child,0,sizeof(new_node->child));
    new_node->id = id;
    new_node->parent = n;
    new_node->v = value;
    return id;
}
uint64_t insert(quadtree * tree,uint64_t pid,uint8_t idx,void *value,void ** exist_value)
{
    quadtree_node * n = find_node(tree,pid);
    if(n==NULL)
    {
        exist_value!=NULL?*exist_value = NULL:0;
        return 0;
    }
   return insert(tree,n,idx,value,exist_value);
}
int erase(quadtree * tree,uint64_t id,int force)
{
    quadtree_node * n = find_node(tree,id);
    if(n==NULL)
        return 0;
    if(n->child[0]!=NULL||n->child[1]!=NULL||n->child[2]!=NULL||n->child[3]!=NULL)
    {
        if(!force)
            return -1;
        uint8_t idx = 0;
        quadtree_node * d = n;
        /*深度遍历，删除id的所有子节点*/
        for (;;)
        {
            if (d->child[idx] != NULL)
            {
                d = d->child[idx];
                idx = 0;
                continue;
            }
            else
            {
                if (idx == 3)
                {
                    idx = QT_GET_IDX(d->id);
                    d = d->parent;
                    if (d == n->parent)
                        break;
                    else
                    {
                        tree->destroy_value(d->child[idx]->v);
                        free_mem_small(d->child[idx]);
                        d->child[idx] = NULL;
                    }
                }
                idx++;
            }
        }
    }
    if(n->parent!=NULL)
        n->parent->child[QT_GET_IDX(id)]=NULL;
    tree->destroy_value(n->v);
    free_mem_small(n);
    return 0;
}
void destroy_quadtree(quadtree * tree)
{
    erase(tree,gen_Id(0,0),1);
    erase(tree,gen_Id(0,1),1);
    erase(tree,gen_Id(0,2),1);
    erase(tree,gen_Id(0,3),1);
    destory_mempool(&tree->mp,1);
}
#ifdef __TEST
int test_quadtree_1()
{
    quadtree tree;
    init_quadtree(&tree);
    uint64_t pid=0;
    for(int i=0;i<1000;i++)
    {
        insert(&tree,pid,i%4,i,NULL);
        if(i%4==3)
            pid = gen_Id(pid,i%20%4);
    }
    pid = 0;
    for(int i=0;i<1000;i++)
    {
        uint64_t id = gen_Id(pid,i%4);
        if(i!=(uint64_t)find(&tree,id))
        {
            printf("find id %lx ,expect value is %lx but actully is %lx\n",id,i,(uint64_t)find(&tree,id));
            printf("%s test failed\n",__func__);
            destroy_quadtree(&tree);
            return -1;
        }
        if(i%4==3)
            pid = gen_Id(pid,i%20%4);
    }
    destroy_quadtree(&tree);
    printf("%s test ok\n",__func__);
    return 0;
}
int main()
{
    test_quadtree_1();
}
#endif



#endif /* G_QUADTREE_H_ */
