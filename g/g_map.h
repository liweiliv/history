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
    constexpr static const double icosohedron_x = 0.525731112119133606f;
    constexpr static const double icosohedron_z = 0.850650808352039932f;
    constexpr static const uint8_t icosohedron_faces[20][3] = {
            {1,4,0},
            {4,9,0},
            {4,5,9},
            {8,5,4},
            {1,8,4},
            {1,10,8},
            {10,3,8},
            {8,3,5},
            {3,2,5},
            {3,7,2},
            {3,10,7},
            {10,6,7},
            {6,11,7},
            {6,0,11},
            {6,1,0},
            {10,1,6},
            {11,0,9},
            {2,11,9},
            {5,2,9},
            {11,2,7}
    };
private:
    static void destroy_map(void* v);
    void create_sub_triangler(struct _quadtree * tree,struct _quadtree_node * node);
public:
    g_map(int diameter,float obliguity,int rotation_period,float orbital_inclination,int orbital_period,const char* map_index_file);
    ~g_map();
    void draw(int level = 3);
    struct _quadtree_node * coordinate2quadtree_node(float longitude ,float latitude,int level);
};




#endif /* G_G_MAP_H_ */
