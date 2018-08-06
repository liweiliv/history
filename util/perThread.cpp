/*
 * perThread.cpp
 *
 *  Created on: 2018年6月7日
 *      Author: liwei
 */
#include <assert.h>
#include <atomic>
#include "perThread.h"
using namespace std;
thread_local uint32_t _threadID = 0xffffffffu;
static atomic<uint32_t> _REISTER_THREAD_ID_NUM(0);
void initThreadID()
{
	if(_threadID == 0xffffffffu)
		_threadID = ++_REISTER_THREAD_ID_NUM;
	assert(_threadID<MAX_THREAD_NUM);
}
uint32_t getMaxThreadID()
{
	return _REISTER_THREAD_ID_NUM;
}


