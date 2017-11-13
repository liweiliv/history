/*
 * MempRing.h
 *
 *  Created on: 2016年1月21日
 *      Author: liwei
 */
#ifndef LIB_UTIL_MEMLIB_MEMPRING_H_
#define LIB_UTIL_MEMLIB_MEMPRING_H_
#include "db_chain.h"
#include <stdlib.h>
#include <stdint.h>
typedef struct _memp_ring_node
{
    chain_node cn;
    size_t buf_size;
    char* hole;
    char buf[1];
}memp_ring_node;
typedef struct _memp_ring_ptr
{
    char *pos;
    size_t size;
}memp_ring_ptr;
typedef struct _memp_ring
{
    chain_cst head;
    memp_ring_node *head_node;
    memp_ring_node *end_node;
    uint64_t default_node_size;
    char * head_pos;
    char * end_pos;
    memp_ring_ptr q[1024];
    unsigned short qh;
    unsigned short qe;
    void * lock;
}memp_ring;
int init_memp_ring(memp_ring* p,uint64_t default_node_size = 1024*1024*16);
void destroy_memp_ring(memp_ring* p);
void * ring_alloc(memp_ring *p ,size_t size);
void * ring_realloc(memp_ring *p ,void *addr,size_t size);
void ring_free(memp_ring *p,void * v);
void ring_clear(memp_ring *p,int min_nodes);
#endif /* LIB_UTIL_MEMLIB_MEMPRING_H_ */
