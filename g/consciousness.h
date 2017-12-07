/*
 * consciousness.h
 *
 *  Created on: 2017年10月19日
 *      Author: liwei
 */

#ifndef G_CONSCIOUSNESS_H_
#define G_CONSCIOUSNESS_H_
#include <pthread.h>
#include <stdint.h>
#include <string>
#include <map>
#include "db_chain.h"
class logic_obj;
struct history_data
{
    uint64_t id;
    void * data;
    uint64_t time;
};
enum purpose_level
{
    PL_FULL,
    PL_MISTINESS,
    PL_IGNORE
};
enum range{

};
struct consciousness_limit
{
    range r;
};
typedef void * (*consciousness_func) (struct history_data * data,logic_obj *obj);
struct  consciousness
{
    //todo
    uint32_t id;
    std::string name;
    purpose_level level;
    consciousness_limit limit;
    chain_node cn;
    consciousness_func func;
};
class  consciousness_sys
{
private:
    std::map<uint32_t,struct consciousness *> consciousness_map;
    std::map<std::string,void*> dll_map;
    pthread_rwlock_t lock;
public:
    consciousness_sys();
    ~consciousness_sys();
    consciousness * get_consciousness(uint32_t id);
    int add_consciousness(uint32_t id,const char * name,const char * limit,purpose_level level,consciousness_func func);
    int add_consciousness_from_dll(uint32_t id,const char * name,const char * limit,purpose_level level,const char * func_name,const char * libPath);
    int load_consciousness_from_file(const char * conf_file);
};

#endif /* G_CONSCIOUSNESS_H_ */
