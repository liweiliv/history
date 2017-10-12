/*
 * g_obj.cpp
 *
 *  Created on: 2017年7月20日
 *      Author: liwei
 */
enum{
	GT_TREE
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
	g_shape<GT_TYPE> * m_shape;
	bool m_shape_share;
	g_color * m_color;
	char * m_source;
public:
	const g_obj_type m_type;
	g_obj(g_obj_type type) :m_shape(NULL),m_shape_share(false),m_color(NULL),m_source(NULL),m_type(type){}
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
};



