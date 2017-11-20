/*
 * logic_obj.cpp
 *
 *  Created on: 2017年10月17日
 *      Author: liwei
 */
#include <stdint.h>
#include <assert.h>
#include <map>
using namespace std;
#include "db_chain.h"
#include "consciousness.h"
class g_obj;
class logic_obj;
struct history_data
{
    uint64_t id;
    void * data;
    uint64_t timestamp;
    uint64_t sequence;
};
class logic
{
    uint32_t m_id;
    consciousness **  m_purposes_full;
    consciousness **  m_purposes_mistiness;
    logic(uint32_t id):m_id(id)
    {
        m_purposes_full = NULL ;
        m_purposes_mistiness = NULL;
    }
    ~logic()
    {
        if(m_purposes_full)
            free(m_purposes_full);
        if(m_purposes_mistiness)
            free(m_purposes_mistiness);
    }
};
class logic_obj
{
public:
    uint64_t m_id;
    g_obj * m_obj;
    logic * m_logic;
    history_data * m_data;
    logic_obj(uint64_t id):m_id(id),m_obj(NULL),m_logic(NULL),m_data(NULL){}
    void set_logic(logic * logic)
    {
        m_logic = logic;
    }
    void set_data(history_data *data )
    {
        m_data = data;
    }
};


