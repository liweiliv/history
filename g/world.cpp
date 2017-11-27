/*
 * frame.cpp
 *
 *  Created on: 2017年11月2日
 *      Author: liwei
 */



//#include "logic_obj.h"
#include "world.h"

#include "skip_list.h"
#include "g_map.h"
#include "g_obj.h"
#include "g_sharp.h"
#include "objs.h"
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
            delete static_cast<g_obj<GLfloat> *>(GET_GL_OBJ(iterator.node->value));
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
        switch(GL_OBJ_TYPE((uint64_t)iterator.node->value))
        {
        case GL_OBJ_FLOAT:
        {
            g_obj<GLfloat> * obj = static_cast<g_obj<GLfloat> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_INT:
        {
            g_obj<GLint> * obj = static_cast<g_obj<GLint> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_DOUBLE:
        {
            g_obj<GLdouble> * obj = static_cast<g_obj<GLdouble> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_UINT:
        {
            g_obj<GLuint> * obj = static_cast<g_obj<GLuint> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_CHAR:
        {
            g_obj<GLbyte> * obj = static_cast<g_obj<GLbyte> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_UCHAR:
        {
            g_obj<GLubyte> * obj = static_cast<g_obj<GLubyte> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_LONG:
        {
            g_obj<GLint64> * obj = static_cast<g_obj<GLint64> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_ULONG:
        {
            g_obj<GLulong> * obj = static_cast<g_obj<GLulong> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_SHORT:
        {
            g_obj<GLshort> * obj = static_cast<g_obj<GLshort> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        case GL_OBJ_USHORT:
        {
            g_obj<GLushort> * obj = static_cast<g_obj<GLushort> *>(GET_GL_OBJ(iterator.node->value));
            obj->draw();
        }
        break;
        default:
            abort();
        }
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
bool world::load_data(const char * file)
{
    regular_icosohedron * o = new regular_icosohedron(1.0f,1);
    insert(m_active_objects,o->get_id(),CREATE_GL_OBJ((uint64_t)o,GL_OBJ_DOUBLE));
    return true;
}
