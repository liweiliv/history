/*
 * test.cpp
 *
 *  Created on: 2017年10月11日
 *      Author: liwei
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "skip_list.h"
#include <time.h>
int main()
{
    skip_list s;
    init_skip_list(&s,NULL);
    for(int i=1;i<10000000;i++)
        insert(&s,i,(void*)i);
    /*
    for(int i=1;i<10000000;i++)
    {
        if(find(&s,i)!=(void*)i)
            abort();
    }*/
    skip_list_iterator iter ;
    seek(&s,1,&iter);
    if(!skip_list_iterator_valid(&iter))
        abort();
    int check=1;
    for(seek(&s,1,&iter);skip_list_iterator_valid(&iter);next(&iter))
    {
        if(iter.node->key!=check||iter.node->value!=(void*)check)
            abort();
        check++;
    }
    if(check!=10000000)
        abort();
}
