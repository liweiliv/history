#include "radixTree.h"
#include <stdio.h>
#include <pthread.h>
#include <set>
using namespace std;
radixTree<uint64_t> tree(NULL);
bool start = false;
void * t(void*argv)
{
	initThreadID();
	while(!start);
	for(uint64_t i =2;i<10000000;i++)
	{
		tree.insert(i,(void*)i);
	}
	pthread_exit(NULL);
}
void * t1(void*argv)
{
	initThreadID();
	while(!start);
	bool exit = false;
	uint64_t min = 0;
	while(true)
	{
		min = 0;
		radixTree<uint64_t>::iterator iter = tree.begin();
		while(iter.valid())
		{
			if(iter.key()!=min+1&&iter.key()!=0)
				abort();
			min = iter.key();
			assert(iter.key()==(uint64_t)iter.value());
			iter.next();
		}
		if(exit)
			break;
		if(!start)
			exit = true;
	}
	if(min!=10000000-1)
		abort();
	pthread_exit(NULL);
}
void test()
{
	initThreadID();
	uint64_t i =1;
	tree.insert(i,(void*)i);
	pthread_t pid[2]={0};
	pthread_create(&pid[0],NULL,t,NULL);
	pthread_create(&pid[1],NULL,t1,NULL);
	start = true;
	pthread_join(pid[0],NULL);
	start = false;
	pthread_join(pid[1],NULL);
	return ;
}
void test1()
{
	initThreadID();
	tree.clear();
	set<uint64_t> s;
	for (int i=0;i<10000000;i++)
	{
		srandom(i);
		uint64_t v = random();
		if(0==tree.insert(v,(void*)v))
			s.insert(v);
	}
	printf("finished insert\n");
	radixTree<uint64_t>::iterator iter = tree.begin();
	set<uint64_t>::iterator siter = s.begin();
	for(;siter!=s.end();siter++,iter.next())
	{
		if(!iter.valid()||*siter!=iter.key())
		{
			abort();
		}
		if(tree.findValue(*siter)!=(void*)(uint64_t)(*siter))
			abort();
	}
	if(iter.valid())
		abort();
	for(siter = s.begin();siter!=s.end();siter++)
	{
		tree.erase(*siter);
		if(tree.findValue(*siter)!=nullptr)
			abort();
	}
	iter = tree.begin();
	if(!iter.valid())
		abort();
}
int main()
{
	test();
	printf("test1 finished\n");
	test1();
}
