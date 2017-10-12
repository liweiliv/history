/*
 * dynamicSizeBuf.h
 *
 *  Created on: 2015��7��23��
 *      Author: liwei
 */

#ifndef SRC_CONGO_DRC_LIB_MEMLIB_COLUMNTREE_H_
#define SRC_CONGO_DRC_LIB_MEMLIB_COLUMNTREE_H_
#include <stdlib.h>
#include <string.h>

#include "../db_chain.h"
#include "memp2.h"

template <typename T>
class dynamicSizeBuf
{
private :
	mempool_giant *mpb;
	int *sizelist;
	int count;
public:
	dynamicSizeBuf(int *sizelist,int count)
	{
		this->count=count;
		this->sizelist=new int[count];
		memcpy(this->sizelist,sizelist,sizeof(int)*count);
		mpb= (mempool_giant*)malloc(sizeof(mempool_giant)*count);
		for(int i=0;i<count;i++)
			init_mempool_giant(&mpb[i],sizeof(T)*sizelist[i],4);
	}
	~dynamicSizeBuf()
	{
		for(int i=0;i<count;i++)
			destory_mempool_giant(&mpb[i],1);
		delete []sizelist;
		free(mpb);
	}
	T* getMem(unsigned int &size)
	{
		long idx=0;
		size_t dtsize=size*sizeof(T);
		while(mpb[idx].dt_size<dtsize)
			idx++;
		size=sizelist[idx];
		return (T*)get_mem_giant(&mpb[idx]);
	}
	void putMem(T* v)
	{
		free_mem_giant(v);
	}
};
#if 0
struct   columnBufBlock
{
	chain_node cn;
	void **buf;
};
typedef void ** COLBUF;
#define _COLMASK 0xf000000000000000
#define _COL_LEN_OFF 60
#define getcolumn(b,index,type) (((type**)(void*)(((uint64_t)(void*)(b))&(~_COLMASK)))[(index)>>3][(index)&0x07])

//static int sizelist[]={32,48,64,128,256,1024};
//#define cbufSize (sizeof(sizelist)/sizeof(int))
struct columnbuf
{
	mempool *mp;
	mempool **mpb;
	int *bufVolmn;
	int *sizelist;
	int count;
	int botsize;
	chain_head *list;
};
#include <stdio.h>
void create_columnbuf(struct columnbuf *c,int align,int *sizelist,int count,int botsize)
{
	c->mp=(mempool*)malloc(sizeof(mempool));
	init_mempool(c->mp,0,align,4);
	c->botsize=botsize;
	c->mpb=(mempool **)malloc(sizeof(mempool*)*count);
	c->count=count;
	c->bufVolmn=(int*)malloc(sizeof(int)*count);
	memset(c->bufVolmn,0,sizeof(int)*count);
	c->sizelist=(int*)malloc(sizeof(int)*count);
	memcpy(c->sizelist,sizelist,sizeof(int)*count);

	c->list=(chain_head *)malloc(sizeof(chain_head)*count);
	for(int i=0;i<count;i++)
	{
		c->mpb[i]=(mempool*)malloc(sizeof(mempool));
		init_mempool(c->mpb[i],0,sizeof(void**)*sizelist[i]/botsize,4);
		c->bufVolmn[i]=1;
		c_init_chain(&c->list[i]);
		for(int j=0;j<10;j++)
		{
			COLBUF nc=(COLBUF)get_mem(c->mpb[i]);
			if(nc==NULL)
				printf("error1");
			//printf("%d\n",sizelist[i]/c->botsize-1);
			for(int k=sizelist[i]/c->botsize-1;k>=0;k--)
			{
				if(NULL==(nc[k]=get_mem(c->mp)))
					printf("error");

			}
			((struct columnBufBlock*)nc[0])->buf=nc;
			c_insert_in_end(&c->list[i],&((struct columnBufBlock*)nc[0])->cn);
		}
	}
}
void destory_columnbuf(struct columnbuf *c)
{
	free(c->list);
	free(c->sizelist);
	free(c->bufVolmn);
	for(int i=0;i<c->count;i++)
	{
		destory_mempool(c->mpb[i],1);
		free(c->mpb[i]);
	}
	free(c->mpb);
	destory_mempool(c->mp,1);
	free(c->mp);
}
int get_columnbuf(struct columnbuf *c,int size,COLBUF *b)
{
	if(*b!=NULL)
	{
		int idx=(((unsigned long)(void*)(*b))>>_COL_LEN_OFF);
		if(c->sizelist[idx]>=size&&c->sizelist[idx-1]<size)
			return 0;
		c_insert_in_end(&c->list[idx],&((struct columnBufBlock *)(((void**)(((uint64_t)*b)&(~_COLMASK)))[0]))->cn);
		if(c->list[idx].count>c->bufVolmn[idx]/5&&c->list[idx].count>10)//����̫�ߣ��ͷ�
		{
			int v=c->list[idx].count-c->bufVolmn[idx]/10;
			if(c->list[idx].count-v<10)
				v+=c->list[idx].count-10;
			do
			{
				struct columnBufBlock *cb=get_last_dt(&c->list[idx],struct columnBufBlock,cn);
				c_delete_node(&c->list[idx],&cb->cn);
				COLBUF b=cb->buf;
				for(int k=c->sizelist[idx]/c->botsize-1;k>=0;k--)
					free_mem(b[k]);
				free_mem(cb);
			}while(v>0);
		}
	}
	//�ҵ����ʵ�buf
	if(size>c->sizelist[c->count-1])
		return -1;
	long s=0;
	while(c->sizelist[s]<size)
		s++;
	/*
	long s=0,e=c->count-1,m;
	do
	{
		m=(s+e)>>1;
		if(c->sizelist[m]>size)
			e=((s+e)>>1)-1;
		else if(c->sizelist[m]<size)
			s=((s+e)>>1)+1;
		else
		{
			s=m;
			goto find;
		}
	}while(s>=e);
	while(c->sizelist[s]<size)
		s++;
		*/
find:
	if(c->list[s].count<=0)//�������㣬����
	{
		while(c->list[s].count<10)
		{
			COLBUF cb=(COLBUF)get_mem(c->mpb[s]);

			if(cb==NULL)
				printf("error5\n");
			for(int k=c->sizelist[s]/c->botsize-1;k>=0;k--)
			{
				if(NULL==(cb[k]=get_mem(c->mp)))
					printf("error4");
			}
			((struct columnBufBlock*)cb[0])->buf=cb;
		//	printf("%lx,%lx\n",cb,cb[0]);
			c_insert_in_end(&c->list[s],&((struct columnBufBlock*)cb[0])->cn);
		}
	}
	struct columnBufBlock *cb=get_last_dt(&c->list[s],struct columnBufBlock,cn);
	c_delete_node(&c->list[s],&cb->cn);
	*b=(COLBUF)((((unsigned long)(void*)(cb->buf))&(~_COLMASK))|(s<<_COL_LEN_OFF));
	//printf("get :%lx,%lx\n",cb->buf,cb);
	return 0;
}
#endif

#endif /* SRC_CONGO_DRC_LIB_MEMLIB_COLUMNTREE_H_ */
