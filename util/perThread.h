/*
 * perThread.h
 *
 *  Created on: 2018年6月7日
 *      Author: liwei
 */

#ifndef UTIL_PERTHREAD_H_
#define UTIL_PERTHREAD_H_
#include <stdint.h>
extern thread_local uint32_t _threadID;
#define MAX_THREAD_NUM 1024
#define PER_THREAD_T(varname,TYPE) TYPE varname[MAX_THREAD_NUM]
//#define PER_THREAD_V(varname) (_threadID!=0xffffffffu?((varname)[_threadID]):(initThreadID(),(varname)[_threadID]))
#define PER_THREAD_V(varname) ((varname)[_threadID])
void initThreadID();
uint32_t getMaxThreadID();
#define FOR_EACH_PER_THREAD_V(iterator,varname) for(decltype((varname)[0]) &iterator = (varname)[0];\
	&iterator!=&(varname)[getMaxThreadID()];\
	iterator=(varname)[1+(((uint8_t*)&iterator)-((uint8_t*)&(varname)[0]))/sizeof(varname[0])])
#endif /* UTIL_PERTHREAD_H_ */
