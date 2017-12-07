/*
 * test_page_pool.c
 *
 *  Created on: 2017年6月17日
 *      Author: liwei
 */

#include <stdio.h>
#include "../sql/page_pool.h"

int m_exit = 0;
int type = 0;
int psize = 1024;
void * t(void * argv)
{
	pp_thd * thd = create_thd((page_pool *) argv, 2);
	batch_log blog;
	char filename[64];
	sprintf(filename, "%lx.log", pthread_self());
	//   init_batch_log(&blog,filename,128*1024);
	int a = 0, f = 0;
	int bufsize = 2048;
	void ** mem = (void**) malloc(sizeof(void*) * bufsize);
	memset(mem, 0, sizeof(void*) * bufsize);
	while (!m_exit)
	{
		int i = random() % bufsize;
		if (i < 0)
			i = 0 - i;
		if (mem[i] != NULL)
		{

			for (int j = 0; j < 128 / 4; j++)
			{
				if (((int*) mem[i])[j] != i + j)
					abort();
			}
			// _log(&blog,"free_page %lx @ %d \n",mem[i],i);
			//printf("free_page %lx @ %d \n",mem[i],i);

			if (type == 0)
				free_page(mem[i]);
			else
				free(mem[i]);
			f++;
		}
		if (type == 0)
			mem[i] = alloc_page(thd);
		else
			mem[i] = malloc(psize);
		a++;
		//_log(&blog,"alloc_page %lx @ %d \n",mem[i],i);
		//printf("alloc_page %lx @ %d \n",mem[i],i);
		//mem[i] =malloc(128);

		for (int j = 0; j < 128 / 4; j++)
		{
			((int*) mem[i])[j] = i + j;
		}

		if (i > bufsize - bufsize / 10)
		{
			for (i = random() % (bufsize / 2); i < bufsize / 2; i++)
			{
				if (mem[i] != NULL)
				{

					for (int j = 0; j < 128 / 4; j++)
					{
						if (((int*) mem[i])[j] != i + j)
							abort();
					}

					// _log(&blog,"free_page %lx @ %d \n",mem[i],i);
					// printf("free_page %lx @ %d \n",mem[i],i);
					if (type == 0)
						free_page(mem[i]);
					else
						free(mem[i]);
					f++;
				}
				mem[i] = NULL;
			}
		}
	}
	if(thd->tid%2==0)
		sleep(1);
	destroy_thd(thd);
	printf("%d,%d\n", a, f);
	free(mem);
	pthread_exit(NULL);
}
int main(int argc, char * argv[])
{
	int tc = 16;
	if (argc >= 2)
		tc = atoi(argv[1]);
	if (argc >= 3)
		psize = atoi(argv[2]);
	if (tc > 32)
		tc = 32;
	if (argc >= 4)
		type = atoi(argv[3]);
	pthread_t tl[32] =
	{ 0 };
	page_pool p;
	init_page_pool(&p, psize, 8, 1024 * psize * 8 * tc, 4, 4);
	for (int i = 0; i < tc; i++)
		pthread_create(&tl[i], NULL, t, (void*) &p);
	sleep(10);
	m_exit = 1;
	for (int i = 0; i < tc; i++)
		pthread_join(tl[i], NULL);
	sleep(2);
	destory_page_pool(&p, 1);
	return 0;
}
