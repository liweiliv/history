/*
 * consciousness.cpp
 *
 *  Created on: 2017年10月18日
 *      Author: liwei
 */
#include "consciousness.h"
#include "file_opt.h"
#include "DLMake.h"
#include "INIParser.h"
using namespace std;

consciousness_sys::consciousness_sys()
{
    pthread_rwlock_init(&lock, NULL);
}
consciousness_sys::~consciousness_sys()
{
    for(map<uint32_t,struct consciousness *>::iterator iter = consciousness_map.begin();iter!=consciousness_map.end();iter++)
        delete iter->second;
    for(map<string,void*>::iterator iter = dll_map.begin();iter!=dll_map.end();iter++)
        destroyDllHandle(iter->second);
    pthread_rwlock_destroy(&lock);
}
consciousness * consciousness_sys::get_consciousness(uint32_t id)
{
    map<uint32_t, struct consciousness *>::iterator iter;
    consciousness * c = NULL;
    pthread_rwlock_rdlock(&lock);
    if ((iter = consciousness_map.find(id)) != consciousness_map.end())
        c = iter->second;
    else
        c = NULL;
    pthread_rwlock_unlock(&lock);
    return c;
}
int consciousness_sys::add_consciousness(uint32_t id, const char * name,
        const char * limit, int level, consciousness_func func)
{
    consciousness * con = new consciousness;
    con->func = func;
    con->id = id;
    //todo limit
    con->level = level;
    con->name = name;
    pthread_rwlock_wrlock(&lock);
    if (!consciousness_map.insert(
            pair<uint32_t, struct consciousness *>(id, con)).second)
    {
        delete con;
        pthread_rwlock_unlock(&lock);
        return -1;
    }
    pthread_rwlock_unlock(&lock);
    return 0;
}
int consciousness_sys::add_consciousness_from_dll(uint32_t id,
        const char * name, const char * limit, int level,
        const char * func_name, const char * libPath)
{
    map<string, void*>::iterator dll_iter;
    void * dll_handle = NULL;
    consciousness_func func;
    pthread_rwlock_rdlock(&lock);
ADD_DLL:
    if (dll_map.end() == (dll_iter = dll_map.find(libPath)))
    {
        pthread_rwlock_unlock(&lock);
        if (createDllHandle(&dll_handle, libPath) != 0)
        {
            return -1;
        }
        pthread_rwlock_wrlock(&lock);
        if (!dll_map.insert(pair<string, void*>(libPath, dll_handle)).second)
        {
            pthread_rwlock_unlock(&lock);
            destroyDllHandle(dll_handle);
            goto ADD_DLL;
        }
    }
    else
        dll_handle = dll_iter->second;
    pthread_rwlock_unlock(&lock);
    if (NULL
            == (func = (consciousness_func) getFuncFromDLL(func_name,dll_handle)))
    {
        return -2;
    }
    if (add_consciousness(id, name, limit, level, func) != 0)
    {
        return -3;
    }
    return 0;
}
int consciousness_sys::load_consciousness_from_file(const char * conf_file)
{
    utility::INIParser parser(conf_file);
    utility::sectionSet sets;
    parser.get_sections(sets);
    for (utility::sectionSet::const_iterator iter = sets.begin();
            iter != sets.end(); iter++)
    {
        const char * libPath = NULL;
        const char * funcName = NULL;
        uint32_t id;
        int level;
        const char * limit;
        if ((libPath = parser.get_string((*iter).c_str(), "libPath", NULL))
                == NULL
                || (funcName = parser.get_string((*iter).c_str(), "funcName",
                        NULL)) == NULL
                || (id = parser.get_unsigned((*iter).c_str(), "id", 0xffffffff))
                        == 0xffffffff||
                        (level=parser.get_int((*iter).c_str(),"level",-1))==-1 ||
                        (limit=parser.get_string((*iter).c_str(),"limit",NULL))==NULL)
        {
            return -1;
        }
        if (add_consciousness_from_dll(id, (*iter).c_str(), limit, level,
                funcName, libPath) != 0)
        {
            return -2;
        }
    }
    return 0;
}

