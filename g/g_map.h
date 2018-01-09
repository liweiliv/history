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
struct _quadtree_node;

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
	float m_orbital_inclination;
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
		const unsigned int m_id;
		const g_map * m_map;
		int m_detail_level;
	private:
		explicit map_view(const g_map * m,int id,int detail_level):m_map(m),m_id(id),m_detail_level(detail_level){}
	public:
		~map_view()
		{
			assert(m_map);
		}
		void draw()
		{
			m_map->draw(m_id);
		}
	};
private:
	int m_max_id;
	unsigned char * m_id_bitmap;
	int m_prev_alloced_id;
	int gen_view_id();
	static void destroy_map(void* v);
	void create_sub_triangler(struct _quadtree * tree,
			struct _quadtree_node * node);
public:
	g_map(int diameter, float obliguity, int rotation_period,
			float orbital_inclination, int orbital_period,
			const char* map_index_file);
	~g_map();
	void draw(const unsigned int level = 3);
	struct _quadtree_node * coordinate2quadtree_node(float longitude,
			float latitude, int level, struct _quadtree_node **node_stack);
	struct _quadtree_node * coordinate2quadtree_node_by_pos(const g_pos_3d<float> *p,int level, struct _quadtree_node **node_stack);
	struct _quadtree_node * get_sub_trangler_by_pos(struct _quadtree * tree,
			_quadtree_node * node, const g_pos_3d<float> *p);

	map_view * create_view(int detail_level,float longitude,float latitude);
	map_view * create_view(int detail_level,const g_pos_3d<float> *p);
};

#endif /* G_G_MAP_H_ */
