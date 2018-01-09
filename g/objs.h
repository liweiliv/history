/*
 * objs.h
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#ifndef G_OBJS_H_
#define G_OBJS_H_
#include "g_obj.h"
#include "bitmap.h"
#include <math.h>
#include <atomic>
class map_obj :public g_obj<GLfloat>
{
#define MAP_OBJ_DEFAULT_SIZE 4
public:
    g_pos_3d<GLfloat> m_center;
    g_pos_3d<GLfloat> m_normal;
    uint64_t m_map_id;

    std::atomic<int> m_ref;
    unsigned char * m_id_maps;
    int m_id_map_size;
    map_obj(uint64_t map_id,int id,const g_pos_3d<GLfloat> & triangler_point_1,const g_pos_3d<GLfloat> & triangler_point_2,const g_pos_3d<GLfloat> & triangler_point_3):
        g_obj<GLfloat>(GT_MAP,id),m_map_id(map_id)
     {

        m_center.x=(triangler_point_1.x+triangler_point_2.x+triangler_point_3.x)/3;
        m_center.y=(triangler_point_1.y+triangler_point_2.y+triangler_point_3.y)/3;
        m_center.z=(triangler_point_1.z+triangler_point_2.z+triangler_point_3.z)/3;
        m_sharp = new g_sharp<g_pos_3d<GLfloat>>(3,0,false);
        m_sharp->m_vectors[0] = triangler_point_1;
        m_sharp->m_vectors[1] = triangler_point_2;
        m_sharp->m_vectors[2] = triangler_point_3;
        calculate_normal(m_normal,m_sharp->m_vectors[0],m_sharp->m_vectors[1],m_sharp->m_vectors[2]);
        calculate_normal(m_sharp->m_normals[0],m_sharp->m_vectors[0],m_sharp->m_vectors[1],m_sharp->m_vectors[2]);
        calculate_normal(m_sharp->m_normals[1],m_sharp->m_vectors[1],m_sharp->m_vectors[2],m_sharp->m_vectors[0]);
        calculate_normal(m_sharp->m_normals[2],m_sharp->m_vectors[2],m_sharp->m_vectors[0],m_sharp->m_vectors[1]);
        m_sharp->m_colors[0].r= 0.5f+rand()%30*0.01f;
        m_sharp->m_colors[0].g= 0.6f+rand()%30*0.01f;
        m_sharp->m_colors[0].b= 0.5f+rand()%30*0.01f;
        m_ref.store(0);
        m_id_maps = (unsigned char *)malloc(MAP_OBJ_DEFAULT_SIZE);
        m_id_map_size = MAP_OBJ_DEFAULT_SIZE*8;
     }
    ~map_obj()
    {

    }
    void draw()
    {
        g_pos_3d<GLfloat> c ,d;
        INIT_POS_3D(&c,0,0,0);
        INIT_POS_3D(&d,0,0,0);
        m_sharp->draw(&c,&d);
    }
    void rigister_user(unsigned int id)
    {
    	if(id>m_id_map_size)
    	{
    		m_id_map_size = (id/24+id/8)*8;
    		m_id_maps = (unsigned char *)realloc(m_id_maps,m_id_map_size);
    	}
    	else
    	{
    		if(test_bitmap(m_id_maps,id))
    			return;
    	}
    	set_bitmap(m_id_maps,id);
    	m_ref++;
    }
};

#endif /* G_OBJS_H_ */
