#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "../DLMake.h"
#if (!defined _ANDROID) || (!defined _IOS)
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
static int soID=0;
static void **dlList=NULL;
static int  dlListSize=0;
static int  dlListVolume=0;
static void ** funcList;
static int logFD=0;

void DLMakeInit()
{
    soID=0; 
    if(dlList!=NULL)
        free(dlList);
    if(funcList!=NULL)
        free(funcList);
    dlList=(void**)malloc(sizeof(void*)*10);
    memset(dlList,0,sizeof(void*)*10);
    funcList=(void**)malloc(sizeof(void*)*10);
    memset(funcList,0,sizeof(void*)*10);
    dlListVolume=10;
    dlListSize=0;
    if(logFD!=0)
        close(logFD);
    mkdir("DLlib",S_IRWXU|S_IRGRP|S_IROTH);
    logFD=open("./DLlib/complite.log",O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(logFD<0)
    {
       printf("can not open file for %d \n",errno);
       logFD=STDOUT_FILENO; //无法打开文件，则输出到标准输出
    }
}
void DLMakeDestroy()
{
    soID=0; 
    if(dlList!=NULL)
    {
        for(int idx=0;idx<dlListSize;idx++)
            dlclose(dlList[idx]);
        free(dlList);
        dlList=NULL;
    }
    if(funcList!=NULL)
    {
        free(funcList);
        funcList=NULL;
    }
    dlListVolume=0;
    dlListSize=0;
    if(logFD>STDOUT_FILENO)
       close(logFD);
    logFD=0;
}
static int checkFileExist(const char *filename)
{
    int fd=open(filename,O_RDONLY);
    if(fd>0)
    {
        close(fd);
        return -1;
    }
    else
    {
        if(errno==ENOENT)
        {
            return 0;
        }
        else
        {
            return errno;
        }
    }
}
int compile(const char * code ,int size)
{
    char filenameBuf[64]={0},sonameBuf[64]={0},compileCmdBuf[128]={0};
    sprintf(filenameBuf,"./DLlib/LIB_%d.c",soID);
    sprintf(sonameBuf,"./DLlib/LIB_%d.so",soID);
    remove(filenameBuf);
    remove(sonameBuf);
    int ret=0,fd=0;
    if((ret=checkFileExist(filenameBuf))!=0)
    {
        printf("%s is exist and remove it failed \n",filenameBuf);
        return -1;
    }
    fd=open(filenameBuf,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fd<=0)
    {
        printf("can not open file :%s ,errno:%d",filenameBuf,errno);
        return -1;
    }
    if(write(fd,code,size)!=size)
    {
        printf("write file :%s failed,errno:%d",filenameBuf,errno);
        close(fd);
        remove(filenameBuf);
        return -1;
    }
    close(fd);
    pid_t pid=fork();
    if(pid==0)
    {
         close(STDOUT_FILENO);
         close(STDERR_FILENO);
         dup2(logFD, STDOUT_FILENO);
         dup2(logFD, STDERR_FILENO);
         char logBuf[256]={0};
         time_t timep;
         time (&timep);
         snprintf(logBuf,256,"--------------------------------compile--------------------------------\n%sgcc -fpic -shared -Wall %s -o %s -g --std=c99\n",ctime(&timep),filenameBuf,sonameBuf);
         write(logFD,logBuf,strlen(logBuf));
         char * gcc_argv[] ={"gcc","-fpic","-shared", "-Wall",filenameBuf,"-o",sonameBuf,"-g","--std=c99", NULL};
         if(0>(ret=execvp("gcc",gcc_argv)))
         {
             printf("failed to call gcc ,return value:%d,errno:%d\n",ret,errno);
             exit(-1);
         }
    }
    else
    {
         waitpid(pid,&ret,0);
    }
    if(ret!=0||checkFileExist(sonameBuf)!=-1)
    {
        write(logFD,"compile failed\n",sizeof("compile failed\n")-1);
        return -1;
    }
    write(logFD,"compile success\n",sizeof("compile success\n")-1);
    return 0;
}
void *getFunc(const char *code,size_t size,const char * funcName)
{
    int ret=0;
    void *pdlHandle;
    char *pszErr;
    char path[128]={0};
    sprintf(path,"./DLlib/LIB_%d.so",soID);
    ret=compile(code,size);
    if(ret!=0)
        return NULL;
    pdlHandle = dlopen(path, RTLD_LAZY);
    pszErr = dlerror();
    if( !pdlHandle || pszErr )
        return NULL;
    void *func=dlsym(pdlHandle,funcName);
    if(func==NULL)
    {
        dlclose(pdlHandle);
        return NULL;
    }
    if(dlListSize>=dlListVolume)
    {
        void ** tmp=(void**)malloc(sizeof(void*)*(dlListVolume+10));
        memset(tmp,0,sizeof(void*)*(dlListVolume+10));
        memcpy(tmp,dlList,sizeof(void*)*dlListVolume);
        free(dlList);
        dlList=tmp;
        tmp=(void**)malloc(sizeof(void*)*(dlListVolume+10));
        memset(tmp,0,sizeof(void*)*(dlListVolume+10));
        memcpy(tmp,funcList,sizeof(void*)*dlListVolume);
        free(funcList);
        funcList=tmp;
        dlListVolume+=10;
    }
    dlList[dlListSize]=pdlHandle;
    funcList[dlListSize]=func;
    dlListSize++;
    soID++;
    return func;
}
#endif

int createDllHandle(void **dllHandle,const char *dllPath)
{
    void *pdlHandle;
    char *pszErr;
    *dllHandle = NULL;
    pdlHandle = dlopen(dllPath, RTLD_LAZY);
    pszErr = dlerror();
    if( !pdlHandle || pszErr )
    {
        printf("open dll from %s failed when call dlopen,error:%s\n",dllPath,pszErr==NULL?"NULL":pszErr);
        return -1;
    }
    *dllHandle = pdlHandle;
    return 0;
}
void * getFuncFromDLL(const char * funcName,void *dllHandle)
{
    if(!funcName||!dllHandle)
        return NULL;
    return dlsym(dllHandle,funcName);
}
void destroyDllHandle(void *dllHandle)
{
	dlclose(dllHandle);
}

