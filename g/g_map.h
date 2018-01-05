/*
 * g_map.h
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#ifndef G_G_MAP_H_
#define G_G_MAP_H_

//map
//object in map
//history data of object in map
//lazy load
//lazy caculate
//lazy save
//lazy create
/*
 * g_map.cpp
 *
 *  Created on: 2017年7月20日
 *      Author: liwei
 */

//map
//object in map
//history data of object in map
//lazy load
//lazy caculate
//lazy save
//lazy create
#include <string.h>
#include <stdint.h>
class map_obj;
struct _quadtree;
struct _quadtree_node ;

template<typename GT_TYPE>
struct g_pos_3d;
#define _pai 3.14159265358979323846L
#define angle2rad(x) (_pai*(x)/180.0f)
class g_map
{

protected:
    struct top_map
    {
        map_obj *m_map;
        _quadtree *m_map_tree;
    };
    int m_diameter;
    float m_obliguity;
    int m_rotation_period;
    float m_orbital_inclination ;
    int m_orbital_period;
    char m_map_index_file[256];
    struct top_map m_maps[20];
public:
    constexpr static const double icosohedron_x = 0.525731112119133606L;
    constexpr static const double icosohedron_z = 0.850650808352039932L;
public:
	class map_view
	{
	public:
		g_map * m_parent_map;
		struct g_pos_3d<float> * m_view_pos;
		struct top_map m_maps[20];
		struct _quadtree_node * m_atomic_node;
	};
private:
    static void destroy_map(void* v);
    void create_sub_triangler(struct _quadtree * tree,struct _quadtree_node * node);
public:
    g_map(int diameter,float obliguity,int rotation_period,float orbital_inclination,int orbital_period,const char* map_index_file);
    ~g_map();
    void draw(unsigned int level = 3);
    struct _quadtree_node * coordinate2quadtree_node(float longitude ,float latitude,int level,struct _quadtree_node **node_stack);
    struct _quadtree_node * get_sub_trangler_by_pos(struct _quadtree * tree,_quadtree_node * node,g_pos_3d<float>  *p);
};




#endif /* G_G_MAP_H_ */
