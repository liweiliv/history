/*
 * block_file_manager_test.cpp
 *
 *  Created on: 2017年3月10日
 *      Author: liwei
 */
#include <stdio.h>
#include "../block_file_manager.h"
#include <pthread.h>
block_file_manager *m;
void * erase(void *argv)
{
    while(1)
    {
        uint64_t max=m->get_max();
        uint64_t min=m->get_min();
        if(min==10000-2)
            return NULL;
        if(max>min)
            m->clear_to(min+1);
       // usleep(100);
    }
}
void *insert(void *argv)
{
    char buf[128];
    for(int i=1;i<=10000;i++)
    {
        sprintf(buf, "idx.%06d", i);
        m->insert(buf);
     //   usleep(100);
    }
}
int main()
{
    m=new block_file_manager("test.index");
    char buf[32]="idx.000000";
    m->insert(buf);
    block_file_manager::iterator *iter =
                        new block_file_manager::iterator(m, m->get_min(), block_file_manager::BM_SEARCH_EQUAL);
    pthread_t i,e;
    pthread_create(&i,NULL,insert,NULL);
//    pthread_create(&e,NULL,erase,NULL);
    long a=0,b=0;
    struct timespec s_tm,e_tm;
    clock_gettime(CLOCK_REALTIME,&s_tm);
    while(1)
    {
        do
        {
            printf("%s,%d\n", iter->get_filename(), iter->get_id());
            a++;

        }while(iter->next());

        do
        {
            printf("%s,%d\n", iter->get_filename(), iter->get_id());
            b++;

        }while(iter->prev());
        if(iter->get_id()>=10000-2)
            break;
        if(m->get_min()>=10000-2)
            break;
        if(a>100000000||b>100000000)
            break;

    }
    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);
    pthread_join(i,NULL);
    pthread_join(e,NULL);
    printf("%ld,%ld\n",a,b);
    delete iter;
    delete m;
}
