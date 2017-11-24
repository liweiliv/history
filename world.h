/*
 * world.h
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#ifndef WORLD_H_
#define WORLD_H_

struct g_map;
struct _skip_list;
class world
{
private:
    struct _skip_list * m_active_objects;
    g_map * m_map;
    virtual int frame();
    virtual int eliminate();
public:
    virtual world();
    virtual ~world();
    virtual int draw();
    virtual void load_data(const char * file);
};



#endif /* WORLD_H_ */
