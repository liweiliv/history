/*
 * test_skip_list.cpp
 *
 *  Created on: 2016年12月30日
 *      Author: liwei
 */
#include <stdio.h>
#include "../skip_list.h"
typedef uint64_t Key;

struct Comparator {
  int operator()(const Key& a, const Key& b) const {
    if (a < b) {
      return -1;
    } else if (a > b) {
      return +1;
    } else {
      return 0;
    }
  }
};
int main()
{
    leveldb::Arena arena;
    Comparator cmp;
    leveldb::SkipList<Key,Comparator> s(arena,cmp);
    for(int i=0;i<10000;i++)
    {
        s.Insert(i);
    }
    for(int i=0;i<10000;i++)
    {
        if(!s.Contains(i))
        {
            printf("can not find %d\n",i);
            abort();
        }
    }
}


