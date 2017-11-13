/*
 * g_obj.cpp
 *
 *  Created on: 2017年7月20日
 *      Author: liwei
 */
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
struct g_color;
class g_shape;
template <typename GT_TYPE>
class g_obj
{
protected:
   uint32_t m_id;
	g_shape<GT_TYPE> * m_shape;
	bool m_shape_share;
	g_color * m_color;
	char * m_source;
public:
	const g_obj_type m_type;
	g_obj(g_obj_type type,uint32_t id) :m_id(id),m_shape(NULL),m_shape_share(false),m_color(NULL),m_source(NULL),m_type(type){}
	virtual ~g_obj()
	{
	    if(m_shape_share&&m_shape)
	        delete m_shape;
	}
	void set_shape(const g_shape<GT_TYPE> * shape)
	{
	    if(m_shape!=NULL&&m_shape_share)
	        delete m_shape;
	    m_shape = shape;
	    m_shape_share = true;
	}
    void set_shape_by_copy(const g_shape<GT_TYPE> * shape)
    {
        if(m_shape!=NULL&&m_shape_share)
            delete m_shape;
        m_shape = new g_shape<GT_TYPE>(shape);
        m_shape_share = false;
    }
    virtual void draw( ) const =0;
};
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

