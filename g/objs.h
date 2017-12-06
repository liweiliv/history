/*
 * objs.h
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#ifndef G_OBJS_H_
#define G_OBJS_H_
#include "g_obj.h"
#include <math.h>
static const GLdouble icosohedron_x = 0.525731112119133606f;
static const GLdouble icosohedron_z = 0.850650808352039932f;

static const GLubyte icosohedron_faces[] = {
		1,4,0,
		4,9,0,
		4,5,9,
		8,5,4,
		1,8,4,
		1,10,8,
		10,3,8,
		8,3,5,
		3,2,5,
		3,7,2,
		3,10,7,
		10,6,7,
		6,11,7,
		6,0,11,
		6,1,0,
		10,1,6,
		11,0,9,
		2,11,9,
		5,2,9,
        11,2,7
};
class regular_icosohedron :public g_obj<GLfloat>
{
public:
    regular_icosohedron(GLfloat d,int id): g_obj<GLfloat>(GT_TREE,id)
    {
        m_sharp = new g_sharp<GLfloat,g_pos_3d>(12,20*3,false);
        memcpy(m_sharp->m_indices,icosohedron_faces,sizeof(icosohedron_faces));
        GLdouble r_X = icosohedron_x*d;
        GLdouble r_Z = icosohedron_z*d;
        INIT_POS_3D(&m_sharp->m_vectors[0],-r_X, 0.0, r_Z);
        INIT_POS_3D(&m_sharp->m_vectors[1],r_X, 0.0, r_Z);
        INIT_POS_3D(&m_sharp->m_vectors[2],-r_X, 0.0, -r_Z);
        INIT_POS_3D(&m_sharp->m_vectors[3],r_X, 0.0, -r_Z);
        INIT_POS_3D(&m_sharp->m_vectors[4],0.0, r_Z, r_X);
        INIT_POS_3D(&m_sharp->m_vectors[5],0.0, r_Z, -r_X);
        INIT_POS_3D(&m_sharp->m_vectors[6],0.0, -r_Z, r_X);
        INIT_POS_3D(&m_sharp->m_vectors[7],0.0, -r_Z, -r_X);
        INIT_POS_3D(&m_sharp->m_vectors[8],r_Z, r_X, 0.0);
        INIT_POS_3D(&m_sharp->m_vectors[9],-r_Z, r_X, 0.0);
        INIT_POS_3D(&m_sharp->m_vectors[10],r_Z, -r_X, 0.0);
        INIT_POS_3D(&m_sharp->m_vectors[11],-r_Z, -r_X, 0.0);
        for(int i=0;i<20*3;i++)
        {
            m_sharp->m_colors[i].r=m_sharp->m_colors[i].g = m_sharp->m_colors[i].b=0.2f*i;
        }
    }
    void draw()
    {
       g_pos_3d<GLfloat> c ,d;
       INIT_POS_3D(&c,0,0,0);
       INIT_POS_3D(&d,0,0,0);
       m_sharp->draw(&c,&d);
    }
};

class map_obj :public g_obj<GLfloat>
{
public:
    g_pos_3d<GLfloat> m_center;
    uint64_t m_map_id;
    map_obj(uint64_t map_id,int id,const g_pos_3d<GLfloat> & triangler_point_1,const g_pos_3d<GLfloat> & triangler_point_2,const g_pos_3d<GLfloat> & triangler_point_3):
        g_obj<GLfloat>(GT_MAP,id),m_map_id(map_id)
     {

        m_center.x=(triangler_point_1.x+triangler_point_2.x+triangler_point_3.x)/3;
        m_center.y=(triangler_point_1.y+triangler_point_2.y+triangler_point_3.y)/3;
        m_center.z=(triangler_point_1.z+triangler_point_2.z+triangler_point_3.z)/3;
        m_sharp = new g_sharp<GLfloat,g_pos_3d>(3,0,false);
        m_sharp->m_vectors[0] = triangler_point_1;
        m_sharp->m_vectors[1] = triangler_point_2;
        m_sharp->m_vectors[2] = triangler_point_3;
        m_sharp->m_colors[0].r= rand()%100*0.01f;
        m_sharp->m_colors[0].g= rand()%100*0.01f;
        m_sharp->m_colors[0].b= rand()%100*0.01f;
     }
    void draw()
    {
        g_pos_3d<GLfloat> c ,d;
        INIT_POS_3D(&c,0,0,0);
        INIT_POS_3D(&d,0,0,0);
        m_sharp->draw(&c,&d);
    }
};

#endif /* G_OBJS_H_ */
