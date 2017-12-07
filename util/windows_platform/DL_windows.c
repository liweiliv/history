/*
 * DL_windows.c
 *
 *  Created on: 2017年10月18日
 *      Author: liwei
 */
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
int createDllHandle(void **dllHandle,const char *dllPath)
{
    HINSTANCE hInstance = LoadLibrary(dllPath);
    if(hInstance == NULL)
    {
        printf("LoadLibrary from %s failed,errno:%d\n",GetLastError());
        *dllHandle = NULL;
        return -1;
    }
    *dllHandle = (void*)(uint64_t)hInstance;
    return 0;
}
void * getFuncFromDLL(const char * funcName,void *dllHandle)
{
    if(!funcName||!dllHandle)
        return NULL;
    return GetProcAddress((HMODULE)dllHandle,funcName);
}
void destroyDllHandle(void *dllHandle)
{
    HINSTANCE hInstance = (uint64_t)dllHandle;
    FreeLibrary(hInstance);
}


