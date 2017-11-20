/*
 * g_obj.h
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#ifndef G_G_OBJ_H_
#define G_G_OBJ_H_

enum{
    GT_TREE,
    GT_STORN,
    GT_HUMAN,
    GT_WATER,
    GT_ARMOR,
    GT_WEAPON
}g_obj_type;
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "g_sharp.h"
template <typename GT_TYPE>
class g_obj
{
protected:
   uint32_t m_id;
    g_sharp<GT_TYPE,g_pos_3d> * m_sharp;
    bool m_shape_share;
    char * m_source;
public:
    const g_obj_type m_type;
    g_obj(g_obj_type type,uint32_t id) :m_id(id),m_sharp(NULL),m_shape_share(false),m_source(NULL),m_type(type){}
    virtual ~g_obj()
    {
        if(m_shape_share&&m_sharp)
            delete m_sharp;
    }
    void set_shape(const g_sharp<GT_TYPE,g_pos_3d> * shape)
    {
        if(m_sharp!=NULL&&m_shape_share)
            delete m_sharp;
        m_sharp = shape;
        m_shape_share = true;
    }
    void set_shape_by_copy(const g_sharp<GT_TYPE,g_pos_3d> * shape)
    {
        if(m_sharp!=NULL&&m_shape_share)
            delete m_sharp;
        m_sharp = new g_sharp<GT_TYPE,g_pos_3d>(shape);
        m_shape_share = false;
    }
    virtual void draw( ) const =0;
};
#if 0
template <typename GT_TYPE>
class g_tree :public g_obj<GT_TYPE>
{
protected:
    uint16_t m_age;
    g_color * m_color_of_month;
public:
    g_tree(uint32_t id):g_obj(GT_TREE,(id&0x0000ffff)|(((uint32_t)GT_TREE)<<16)),m_age(0),m_color_of_month(NULL)
    {
    }
};
#endif





#endif /* G_G_OBJ_H_ */
