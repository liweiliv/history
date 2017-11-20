/*
 * objs.h
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#ifndef G_OBJS_H_
#define G_OBJS_H_
#include "g_obj.h"
template <typename GT_TYPE>
class regular_icosohedron :public g_obj<GT_TYPE>
{
private:

public:
    regular_icosohedron(GT_TYPE d)
    {
        m_sharp->draw();
    }
};



#endif /* G_OBJS_H_ */
