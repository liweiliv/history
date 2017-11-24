/*
 * frame.cpp
 *
 *  Created on: 2017年11月2日
 *      Author: liwei
 */



//#include "logic_obj.h"
#include "skip_list.h"
#include "g_map.h"
#include "world.h"
#include "g_obj.h"
#include "g_sharp.h"
world::world()
{
    m_active_objects = new struct _skip_list ;
    init_skip_list(m_active_objects,NULL);
    m_map=NULL;
}
world::~world()
{
    if(m_active_objects)
    {
        skip_list_iterator  iterator ;
        for(begin(m_active_objects,&iterator);SKIP_ITER_VALID(&iterator);next(&iterator))
        {
            delete static_cast<g_obj<GLfloat> *>(iterator.node->value);
        }
        destroy_skip_list(m_active_objects);
        delete m_active_objects;
    }
    if(m_map)
        delete m_map;
}
int world::draw()
{
    skip_list_iterator  iterator ;
    for(begin(m_active_objects,&iterator);SKIP_ITER_VALID(&iterator);next(&iterator))
    {
        g_obj<GLfloat> * obj = static_cast<g_obj<GLfloat> *>(iterator.node->value);
        obj->draw();
    }
    return 0;
}
int world::eliminate()
{
    return 0;
}
int world::frame()
{
    return draw();
}
void world::load_data(const char * file)
{

}
