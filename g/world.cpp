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
int world::draw()
{
    skip_list_iterator  iterator ;
    for(begin(m_active_objects,&iterator);SKIP_ITER_VALID(&iterator);next(&iterator))
    {
        g_obj<GLfloat> * obj = static_cast<g_obj<GLfloat> *>(iterator.node->value);
        obj->draw();
    }
}
