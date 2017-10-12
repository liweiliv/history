/*
 * cotest.c

 *
 *  Created on: 2015Äê7ÔÂ28ÈÕ
 *      Author: liwei
 */
#include <dynamicSizeBuf.h>
#include <stdio.h>
#include <string.h>
int sizelist[]={32,48,64,128,256,1024};
typedef struct _test
{
	int a;
	char b;
	char c[30];
	long d;
}test;
int main()
{

	struct timespec s,e;
	test ***t=new test**[6];
	size_t size=15;
	dynamicSizeBuf<test> d(sizelist,sizeof(sizelist)/sizeof(int));
	for(int m=0;m<sizeof(sizelist)/sizeof(int);m++)
	{
		t[m]=new test*[500];
		for(int i=0;i<500;i++)
		{
			size=sizelist[m]-1;
			t[m][i]=d.getMem(size);
			for(int j=sizelist[m]-1;j>=0;j--)
			{
				t[m][i][j].a=j;
				t[m][i][j].b='a'+m%20;
				sprintf(t[m][i][j].c,"%03d%03d%03d",m,m,m);
				t[m][i][j].d=m*m;
			}
		}
	}
	for(int m=0;m<sizeof(sizelist)/sizeof(int);m++)
	{

		for(int i=0;i<500;i++)
		{
			for(int j=sizelist[m]-1;j>=0;j--)
			{
				printf("%d,%c,%s,%ld\n",t[m][i][j].a,t[m][i][j].b,t[m][i][j].c,t[m][i][j].d);
			}
		}
		delete []t[m];
	}
	delete []t;

# if 0
	COLBUF **cbl=(COLBUF **)malloc(6*sizeof(COLBUF *));
	COLBUF *cb;
	int i=0,j=0;
	test *a,*b,*C,*d,*E,*f;
	for(;i<6;i++)
	{
		cbl[i]=(COLBUF*)malloc(1280*sizeof(COLBUF*));
		memset(cbl[i],0,1280*sizeof(COLBUF));
	}

	create_columnbuf(&c,sizeof(test)*8,sizelist,6,8);
	for(j=0;j<6;j++)
	{
		cb=cbl[j];
		for(i=0;i<1280;i++)
		{
			get_columnbuf(&c,sizelist[j]-1,&cb[i]);
			//printf("%d,%lx\n",j,cb[i]);

			for(int k=sizelist[j]-2;k>=0;k--)
			{
				test * a=&getcolumn(cb[i],k,test);
				a->a=k;
				a->b='a'+k%20;
				sprintf(a->c,"%03d%03d%03d",k,k,k);
				a->d=k*k;
			}

		}

		for(i=0;i<1280;i++)
		{
			for(int k=sizelist[j]-2;k>=0;k--)
			{
				test * a=&getcolumn(cb[i],k,test);
				//printf("%d,%c,%s,%ld\n",a->a,a->b,a->c,a->d);
			}

		}

	}

	clock_gettime(CLOCK_MONOTONIC,&s);
	for(int i=0;i<10000000;i++)
	{
		a=&getcolumn(cb[0],i,test);
		b=&getcolumn(cb[0],i,test);
		C=&getcolumn(cb[0],i,test);
		d=&getcolumn(cb[0],i,test);
		E=&getcolumn(cb[0],i,test);
	}
	clock_gettime(CLOCK_MONOTONIC,&e);
	printf("%lx,%lx,%lx,%lx,%lx\n",a,b,C,d,E);
	printf("%ld\n",(e.tv_sec-s.tv_sec)*1000000000+e.tv_nsec-s.tv_nsec);
	char **bt=(char**)malloc(sizeof(char**));
	*bt=(void*)0x4431;
	clock_gettime(CLOCK_MONOTONIC,&s);
	for(int i=0;i<10000000;i++)
	{
		a=(test*)bt[0];
		b=(test*)bt[0];
		C=(test*)bt[0];
		d=(test*)bt[0];
		E=(test*)bt[0];
	}
	clock_gettime(CLOCK_MONOTONIC,&e);
	printf("%lx,%lx,%lx,%lx,%lx\n",a,b,C,d,E);
	printf("%ld\n",(e.tv_sec-s.tv_sec)*1000000000+e.tv_nsec-s.tv_nsec);
#endif

}



