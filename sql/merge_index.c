
/*
 * merge_index.c
 *
 *  Created on: 2017年3月21日
 *      Author: liwei
 */
#define MAX_WAYS 2048
static int ls[MAX_WAYS];            /* loser tree */
#include "sql/c_index.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
struct mi
{
    int idx;
    int count;
    key_and_offset ko[1];
};
void adjust(struct mi ** runs, int n, int s)
{
    int t, tmp;

    t = (s+n)/2;
    while (t > 0) {
        if (s == -1) {
            break;
        }
        if (ls[t] == -1 || runs[s]->ko[runs[s]->idx].key > runs[ls[t]]->ko[runs[ls[t]]->idx].key) {
            tmp = s;
            s = ls[t];
            ls[t] = tmp;
        }
        t >>= 1;
    }
    ls[0] = s;
}

void create_loser_tree(struct mi **runs, int n)
{
    int     i;

    for (i = 0; i < n; i++) {
        ls[i] = -1;
    }
    for (i = n-1; i >= 0; i--) {
        adjust(runs, n, i);
    }
}
int kway_merge(struct mi ** m,)
{

}

int main()
{
    int mc=1024;
    int msc=2560*4;
    struct mi **m=(struct mi **)malloc(sizeof(struct mi*)*mc);
    int i=0,j=0,last=0,a=mc,all=0;
    for(i=0;i<mc;i++)
    {
        m[i]=(struct mi *)malloc(sizeof(struct mi)+sizeof(key_and_offset)*msc);
        m[i]->idx=0;
        m[i]->ko[0].key=random()%mc;
        for(j=1;j<msc;j++)
        {
            m[i]->ko[j].key= m[i]->ko[j-1].key+random()%msc;
        }
        m[i]->count=j;
    }
    struct timespec s_tm,e_tm;
    clock_gettime(CLOCK_REALTIME,&s_tm);
    create_loser_tree(m,mc);
    for(;;)
    {
        if(last>m[ls[0]]->ko[m[ls[0]]->idx].key)
            abort();
        else
        {
          //  printf("%d\n",m[ls[0]]->ko[m[ls[0]]->idx].key);
            last=m[ls[0]]->ko[m[ls[0]]->idx].key;
            all++;
        }
        if(++m[ls[0]]->idx>=msc)
        {
            if(--a<=0)
                break;
            m[ls[0]]->ko[m[ls[0]]->idx].key=~(1<<31);
        }
        adjust(m, mc, ls[0]);
    }
    clock_gettime(CLOCK_REALTIME,&e_tm);
    printf("time use:%lu\n",(e_tm.tv_sec-s_tm.tv_sec)*1000000000+e_tm.tv_nsec-s_tm.tv_nsec);

    printf("%d\n",all);
    return 0;
}
