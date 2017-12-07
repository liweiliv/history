/*
 * test_skiplist2block.cpp
 *
 *  Created on: 2017年2月8日
 *      Author: liwei
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../sql/skiplist2index.h"
int test_int()
{
    leveldb::Arena arena;
    int_key_comparator cmp;
    leveldb::SkipList<int_key,int_key_comparator> skip_list(cmp,&arena);
    recent_block_cache c;
    char * data =(char*)malloc(1000000*64);
    int off=0;
    struct timespec s_tm,e_tm;
    char cmpbuf[65]={0};
    snprintf(cmpbuf,24,"%d__%lu_AAA_BBB_CCC_DDD_EEE_FFF_GGG_HHH_III_JJJ_KKK_LLL_MMM_NNN_OOO_PPP",0,24);
    clock_gettime(CLOCK_REALTIME,&s_tm);
    for(int i=0;i<1000000;i++)
    {
        int_key k(i,data+off,24);
        off+=k.v_size;
     //   snprintf((char*)k.v,k.v_size,"%d__%lu_AAA_BBB_CCC_DDD_EEE_FFF_GGG_HHH_III_JJJ_KKK_LLL_MMM_NNN_OOO_PPP",i,k.v_size);
        skip_list.Insert(k);
    }
    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);

    clock_gettime(CLOCK_REALTIME,&s_tm);
    c.int_skiplist_to_cache(&skip_list,off,0);

    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);
    c_table_block *block=c.get_first_block();
    c_block_iterator iter;
    find_intkey_in_c_block(&block->base_block, &iter, 0, C_INDEX_SEARCH_AFTER);
    int64_t cnt=0;

    do
    {
        c_table_base_block * cb=(c_table_base_block*)iter.value;
        c_block_iterator it;
        if(0>find_intkey_in_c_block(cb, &it, cnt, C_INDEX_SEARCH_AFTER))
        {
            break;
        }
        do
        {
            if(it.key_int!=cnt)
            {
                abort();
            }
#if 0
            snprintf(cmpbuf,it.value_size,"%ld__%ld_AAA_BBB_CCC_DDD_EEE_FFF_GGG_HHH_III_JJJ_KKK_LLL_MMM_NNN_OOO_PPP",cnt,it.value_size);
            if(0!=memcmp(cmpbuf,it.value,it.value_size))
            {
                abort();
            }
#endif
            cnt++;
            if(c_table_block_iter_next(&it)<0)
            {
                break;
            }

        }while(1);
        if(c_table_block_iter_next(&iter)<0)
        {
            break;
        }
    }while(1);
    if(cnt!=1000000)
        abort();
    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);
    printf("test OK\n");
    free(data);
}
int test_str()
{
    leveldb::Arena arena;
    str_key_comparator cmp;
    leveldb::SkipList<str_key,str_key_comparator> skip_list(cmp,&arena);
    char * data =(char*)malloc(1000000*64);
    char * key =(char*)malloc(1000000*64);
    int off=0,key_off=0;
    struct timespec s_tm,e_tm;
    char cmpbuf[65]={0};
    recent_block_cache c;
    snprintf(cmpbuf,24,"%d__%lu_AAA_BBB_CCC_DDD_EEE_FFF_GGG_HHH_III_JJJ_KKK_LLL_MMM_NNN_OOO_PPP",0,24);
    clock_gettime(CLOCK_REALTIME,&s_tm);
    for(int i=0;i<1000000;i++)
    {
        int len=sprintf(key+key_off,"%d",i)+1;
        str_key k(key+key_off,len,data+off,24);
        off+=k.v_size;
        key_off+=len;
//        snprintf((char*)k.v,k.v_size,"%d__%lu_AAA_BBB_CCC_DDD_EEE_FFF_GGG_HHH_III_JJJ_KKK_LLL_MMM_NNN_OOO_PPP",i,k.v_size);
        skip_list.Insert(k);
    }
    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);

    clock_gettime(CLOCK_REALTIME,&s_tm);
    c.str_skiplist_to_cache(&skip_list,off,0);
    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);
    c_table_block *block=c.get_first_block();
    c_block_iterator iter;
    seek_to_first(&block->base_block, &iter);
    int64_t cnt=0;
    key_off=0;
    do
    {
        c_table_base_block * cb=(c_table_base_block*)iter.value;
        c_block_iterator it;
        if(0>seek_to_first(cb, &it))
        {
            break;
        }

        do
        {
  //          printf("%s,%d,%d,%s,%d,%d\n",it.key_str,it.key_size,strlen(it.key_str)+1,it.value,it.value_size,strlen((char*)it.value)+1);
#if 0
            if(strcmp(it.key_str,key+key_off)!=0)
            {
                abort();
            }
#endif
#if 0
            snprintf(cmpbuf,it.value_size,"%ld__%ld_AAA_BBB_CCC_DDD_EEE_FFF_GGG_HHH_III_JJJ_KKK_LLL_MMM_NNN_OOO_PPP",cnt,it.value_size);
            if(0!=memcmp(cmpbuf,it.value,it.value_size))
            {
                abort();
            }
#endif
            cnt++;
            key_off+=strlen(key+key_off)+1;
            if(c_table_block_iter_next(&it)<0)
            {
                break;
            }

        }while(1);
        if(c_table_block_iter_next(&iter)<0)
        {
            break;
        }
    }while(1);
    if(cnt!=1000000)
        abort();
    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);
    printf("test OK\n");
    free(data);
    free(key);
}
int main()
{
    test_int();
    test_str();
}

