/*
 * skiplist2block.h
 *
 *  Created on: 2017年2月8日
 *      Author: liwei
 */

#ifndef LIB_CALCULATE_ENGINE_SKIPLIST2INDEX_H_
#define LIB_CALCULATE_ENGINE_SKIPLIST2INDEX_H_
#include "skip_list.h"
#include "c_block.h"
#include <string.h>

#include "../util/atomic.h"
typedef struct _int_key
{
    int64_t k;
    void* v;
    _int_key(const _int_key &k)
    {
        this->k=k.k;
        this->v=k.v;
    }
    _int_key(int64_t K,void* v)
    {
        this->k=K;
        this->v=v;
    }
    _int_key(int64_t k)
    {
        this->k=k;
        this->v=NULL;
    }
}int_key;
struct int_key_comparator
{
  int operator()(const _int_key& a, const _int_key& b) const
  {
      if (a.k < b.k) {
        return -1;
      } else if (a.k> b.k) {
        return +1;
      } else {
        return 0;
      }
  }
};
typedef struct _str_key
{
    char* k;
    size_t k_size;
    void* v;
    size_t v_size;
    _str_key(const _str_key &k)
    {
        this->k=k.k;
        this->k_size=k.k_size;
        this->v=k.v;
        this->v_size=k.v_size;
    }
    _str_key(char* k,size_t k_size,void* v,size_t v_size)
    {
        this->k=k;
        this->k_size=k_size;
        this->v=v;
        this->v_size=v_size;
    }
    _str_key(char* k)
    {
        this->k=k;
        this->k_size=0;
        this->v=NULL;
        this->v_size=0;
    }
}str_key;

typedef struct _uinon_key
{
    char* k;
    size_t k_size;
    void* v;
    size_t v_size;
    _uinon_key(const _str_key &k)
    {
        this->k=k.k;
        this->k_size=k.k_size;
        this->v=k.v;
        this->v_size=k.v_size;
    }
    _uinon_key(char* k,size_t k_size,void* v,size_t v_size)
    {
        this->k=k;
        this->k_size=k_size;
        this->v=v;
        this->v_size=v_size;
    }
    _uinon_key(char* k)
    {
        this->k=k;
        this->k_size=0;
        this->v=NULL;
        this->v_size=0;
    }
}uinon_key;


struct str_key_comparator
{
  int operator()(const _str_key& a, const _str_key& b) const
  {
      if(a.k_size==b.k_size)
          return memcmp(a.k,b.k,a.k_size);
      else if(a.k_size>b.k_size)
      {
          int c=memcmp(a.k,b.k,b.k_size);
          if(c==0)
              return 1;
          else
              return c;
      }
      else
      {
          int c=memcmp(a.k,b.k,a.k_size);
          if(c==0)
              return -1;
          else
              return c;
      }
  }
};


struct _memp_ring;
class recent_block_cache
{
private:
    chain_cst mem_cache;
    chain_cst disk_cache;
    _memp_ring *mem_pool_child;
    _memp_ring *mem_pool_head;
    _memp_ring *mem_pool_head_idx_buf;
    _memp_ring *mem_pool_index_keys;
    atomic_t id_start;
    atomic_t id_end;
public:
    recent_block_cache();
    ~recent_block_cache();
    int str_skiplist_to_cache(leveldb::SkipList<str_key, str_key_comparator> *skip_list, size_t data_size, uint32_t flag);
    int int_skiplist_to_index(leveldb::SkipList<int_key, int_key_comparator> *skip_list, int count);

    c_table_block * get_first_block();
private:
    int flush_block_to_disk()
    {
        return 0;
    }
};



#endif /* LIB_CALCULATE_ENGINE_SKIPLIST2INDEX_H_ */
