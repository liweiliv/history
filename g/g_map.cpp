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
#include <math.h>
#include "quadtree.h"
#include "g_map.h"
#include "g_obj.h"
#include "objs.h"
#include "Log_r.h"
#include "bitmap.h"

#define max_cos_value_in_sub_trangle (0.79465447229176611l+0.000001l)
constexpr static const uint8_t icosohedron_faces[20][3] =
{
{ 1, 4, 0 },
{ 4, 9, 0 },
{ 4, 5, 9 },
{ 8, 5, 4 },
{ 1, 8, 4 },
{ 1, 10, 8 },
{ 10, 3, 8 },
{ 8, 3, 5 },
{ 3, 2, 5 },
{ 3, 7, 2 },
{ 3, 10, 7 },
{ 10, 6, 7 },
{ 6, 11, 7 },
{ 6, 0, 11 },
{ 6, 1, 0 },
{ 10, 1, 6 },
{ 11, 0, 9 },
{ 2, 11, 9 },
{ 5, 2, 9 },
{ 11, 2, 7 } };
constexpr static const g_pos_3d<GLdouble> source_point =
{ 0.0f, 0.0f, 0.0f };
void g_map::destroy_map(void* v)
{
	delete static_cast<map_obj*>(v);
}
static inline void calculate_point_in_ball(double diameter,
		g_pos_3d<GLdouble>* point)
{
	double mod = sqrt(
			diameter * diameter
					/ (point->x * point->x + point->y * point->y
							+ point->z * point->z));
	point->x *= mod;
	point->y *= mod;
	point->z *= mod;
}
void g_map::create_sub_triangler(struct _quadtree * tree, quadtree_node * node)
{
	g_pos_3d<GLdouble> v01, v12, v20;
	map_obj * m = static_cast<map_obj*>(node->v);

	v01.x = (m->m_sharp->m_vectors[0].x + m->m_sharp->m_vectors[1].x);
	v01.y = (m->m_sharp->m_vectors[0].y + m->m_sharp->m_vectors[1].y);
	v01.z = (m->m_sharp->m_vectors[0].z + m->m_sharp->m_vectors[1].z);
	calculate_point_in_ball((double) m_diameter, &v01);

	v12.x = (m->m_sharp->m_vectors[1].x + m->m_sharp->m_vectors[2].x);
	v12.y = (m->m_sharp->m_vectors[1].y + m->m_sharp->m_vectors[2].y);
	v12.z = (m->m_sharp->m_vectors[1].z + m->m_sharp->m_vectors[2].z);

	calculate_point_in_ball((double) m_diameter, &v12);

	v20.x = (m->m_sharp->m_vectors[2].x + m->m_sharp->m_vectors[0].x);
	v20.y = (m->m_sharp->m_vectors[2].y + m->m_sharp->m_vectors[0].y);
	v20.z = (m->m_sharp->m_vectors[2].z + m->m_sharp->m_vectors[0].z);

	calculate_point_in_ball((double) m_diameter, &v20);

	if (!node->child[0])
	{
		map_obj * m_0_01_20 = new map_obj(0, 0, m->m_sharp->m_vectors[0], v01,
				v20);
		m_0_01_20->m_map_id = insert(tree, node, 0, m_0_01_20, NULL);
	}
	if (!node->child[1])
	{
		map_obj * m_01_1_12 = new map_obj(gen_Id(node->id, 1),
				gen_Id(node->id, 1), v01, m->m_sharp->m_vectors[1], v12);
		m_01_1_12->m_map_id = insert(tree, node, 1, m_01_1_12, NULL);
	}
	if (!node->child[2])
	{
		map_obj * m_20_2_12 = new map_obj(gen_Id(node->id, 2),
				gen_Id(node->id, 2), v20, m->m_sharp->m_vectors[2], v12);
		m_20_2_12->m_map_id = insert(tree, node, 2, m_20_2_12, NULL);
	}
	if (!node->child[3])
	{
		map_obj * m_01_12_20 = new map_obj(gen_Id(node->id, 3),
				gen_Id(node->id, 3), v01, v12, v20);
		m_01_12_20->m_map_id = insert(tree, node, 3, m_01_12_20, NULL);
	}
}
g_map::g_map(int diameter, float obliguity, int rotation_period,
		float orbital_inclination, int orbital_period,
		const char* map_index_file) :
		m_diameter(diameter), m_obliguity(obliguity), m_rotation_period(
				rotation_period), m_orbital_inclination(orbital_inclination), m_orbital_period(
				orbital_period)
{
	strncpy(m_map_index_file, map_index_file, 255);

	m_id_bitmap = (unsigned char*)malloc((m_max_id=32)/8);
	m_prev_alloced_id = 0;
	GLdouble r_X = icosohedron_x * m_diameter;
	GLdouble r_Z = icosohedron_z * m_diameter;
	g_pos_3d<GLdouble> vectors[12];
	INIT_POS_3D(&vectors[0], -r_X, 0.0, r_Z);
	INIT_POS_3D(&vectors[1], r_X, 0.0, r_Z);
	INIT_POS_3D(&vectors[2], -r_X, 0.0, -r_Z);
	INIT_POS_3D(&vectors[3], r_X, 0.0, -r_Z);
	INIT_POS_3D(&vectors[4], 0.0, r_Z, r_X);
	INIT_POS_3D(&vectors[5], 0.0, r_Z, -r_X);
	INIT_POS_3D(&vectors[6], 0.0, -r_Z, r_X);
	INIT_POS_3D(&vectors[7], 0.0, -r_Z, -r_X);
	INIT_POS_3D(&vectors[8], r_Z, r_X, 0.0);
	INIT_POS_3D(&vectors[9], -r_Z, r_X, 0.0);
	INIT_POS_3D(&vectors[10], r_Z, -r_X, 0.0);
	INIT_POS_3D(&vectors[11], -r_Z, -r_X, 0.0);

	m_maps[0].m_map = new map_obj(0, 0, vectors[icosohedron_faces[0][0]],
			vectors[icosohedron_faces[0][1]], vectors[icosohedron_faces[0][2]]);
	m_maps[1].m_map = new map_obj(1, 1, vectors[icosohedron_faces[1][0]],
			vectors[icosohedron_faces[1][1]], vectors[icosohedron_faces[1][2]]);
	m_maps[2].m_map = new map_obj(2, 2, vectors[icosohedron_faces[2][0]],
			vectors[icosohedron_faces[2][1]], vectors[icosohedron_faces[2][2]]);
	m_maps[3].m_map = new map_obj(3, 3, vectors[icosohedron_faces[3][0]],
			vectors[icosohedron_faces[3][1]], vectors[icosohedron_faces[3][2]]);
	m_maps[4].m_map = new map_obj(4, 4, vectors[icosohedron_faces[4][0]],
			vectors[icosohedron_faces[4][1]], vectors[icosohedron_faces[4][2]]);
	m_maps[5].m_map = new map_obj(5, 5, vectors[icosohedron_faces[5][0]],
			vectors[icosohedron_faces[5][1]], vectors[icosohedron_faces[5][2]]);
	m_maps[6].m_map = new map_obj(6, 6, vectors[icosohedron_faces[6][0]],
			vectors[icosohedron_faces[6][1]], vectors[icosohedron_faces[6][2]]);
	m_maps[7].m_map = new map_obj(7, 7, vectors[icosohedron_faces[7][0]],
			vectors[icosohedron_faces[7][1]], vectors[icosohedron_faces[7][2]]);
	m_maps[8].m_map = new map_obj(8, 8, vectors[icosohedron_faces[8][0]],
			vectors[icosohedron_faces[8][1]], vectors[icosohedron_faces[8][2]]);
	m_maps[9].m_map = new map_obj(9, 9, vectors[icosohedron_faces[9][0]],
			vectors[icosohedron_faces[9][1]], vectors[icosohedron_faces[9][2]]);
	m_maps[10].m_map = new map_obj(10, 10, vectors[icosohedron_faces[10][0]],
			vectors[icosohedron_faces[10][1]],
			vectors[icosohedron_faces[10][2]]);
	m_maps[11].m_map = new map_obj(11, 11, vectors[icosohedron_faces[11][0]],
			vectors[icosohedron_faces[11][1]],
			vectors[icosohedron_faces[11][2]]);
	m_maps[12].m_map = new map_obj(12, 12, vectors[icosohedron_faces[12][0]],
			vectors[icosohedron_faces[12][1]],
			vectors[icosohedron_faces[12][2]]);
	m_maps[13].m_map = new map_obj(13, 13, vectors[icosohedron_faces[13][0]],
			vectors[icosohedron_faces[13][1]],
			vectors[icosohedron_faces[13][2]]);
	m_maps[14].m_map = new map_obj(14, 14, vectors[icosohedron_faces[14][0]],
			vectors[icosohedron_faces[14][1]],
			vectors[icosohedron_faces[14][2]]);
	m_maps[15].m_map = new map_obj(15, 15, vectors[icosohedron_faces[15][0]],
			vectors[icosohedron_faces[15][1]],
			vectors[icosohedron_faces[15][2]]);
	m_maps[16].m_map = new map_obj(16, 16, vectors[icosohedron_faces[16][0]],
			vectors[icosohedron_faces[16][1]],
			vectors[icosohedron_faces[16][2]]);
	m_maps[17].m_map = new map_obj(17, 17, vectors[icosohedron_faces[17][0]],
			vectors[icosohedron_faces[17][1]],
			vectors[icosohedron_faces[17][2]]);
	m_maps[18].m_map = new map_obj(18, 18, vectors[icosohedron_faces[18][0]],
			vectors[icosohedron_faces[18][1]],
			vectors[icosohedron_faces[18][2]]);
	m_maps[19].m_map = new map_obj(19, 19, vectors[icosohedron_faces[19][0]],
			vectors[icosohedron_faces[19][1]],
			vectors[icosohedron_faces[19][2]]);
	//初始状态细分4次
	for (int i = 0; i < 20; i++)
	{
		m_maps[i].m_map_tree = new quadtree;
		init_quadtree(m_maps[i].m_map_tree, destroy_map);
		m_maps[i].m_map_tree->root.v = m_maps[i].m_map;
		quadtree_node *n = &m_maps[i].m_map_tree->root;
		uint8_t idx = 0;
		for (;;)
		{
			if (QT_LEVEL(n->id) > 5)
				goto BACK_TO_PARENT;
			if (n->child[idx] != NULL)
			{
				if (++idx > 3)
					goto BACK_TO_PARENT;
				else
					continue;
			}
			else
			{
				create_sub_triangler(m_maps[i].m_map_tree, n);
				idx = 0;
				n = n->child[0];
				continue;
			}
			BACK_TO_PARENT: idx = QT_GET_IDX(n->id);
			n = n->parent;
			if (n == NULL) //root
				break;

			if (idx++ == 3)
				goto BACK_TO_PARENT;
		}
	}

}
g_map::~g_map()
{
	for (int i = 0; i < 20; i++)
	{
		destroy_quadtree(m_maps[i].m_map_tree);
		delete m_maps[i].m_map_tree;
	}

}
void g_map::draw(const unsigned int level,const glm::vec3 & camera)
{
#if 1
	g_pos_3d<GLdouble> p = {camera.x,camera.y,camera.z};
	struct _quadtree_node *node_stack[16], *n = coordinate2quadtree_node_by_pos(&p,level, node_stack) ;
	if (n != NULL)
	{
		for (int i = 0; i < 20; i++)
		{
			quadtree_node *n = &m_maps[i].m_map_tree->root;
			static_cast<map_obj*>(n->v)->draw();
		}
		for (uint i = 0 ;i<level;i++)
		{
			if (node_stack[i] != NULL)
			{
				//map_obj * m = static_cast<map_obj*>(node_stack[i]->v);
				static_cast<map_obj*>(node_stack[i]->child[0]->v)->draw();
				static_cast<map_obj*>(node_stack[i]->child[1]->v)->draw();
				static_cast<map_obj*>(node_stack[i]->child[2]->v)->draw();
				static_cast<map_obj*>(node_stack[i]->child[3]->v)->draw();
			}
			else
				break;
		}
		if(node_stack[0] !=NULL )
			static_cast<map_obj*>(node_stack[0]->v)->draw();
	}
#else
	int c = 0;
	for (int i = 0; i < 20; i++)
	{
		quadtree_node *n = &m_maps[i].m_map_tree->root;
		uint8_t idx = 0;
		for (;;)
		{
			if (QT_LEVEL(n->id) == level)
			{
				map_obj * m = static_cast<map_obj*>(n->v);
				m->draw();
				goto BACK_TO_PARENT;
			}
			if (n->child[idx] != NULL)
			{
				n=n->child[idx];
				idx = 0;
				continue;
			}
			else
			{
				create_sub_triangler(m_maps[i].m_map_tree, n);
				idx = 0;
				n = n->child[0];
				continue;
			}
BACK_TO_PARENT:
			idx = QT_GET_IDX(n->id);
			n = n->parent;
			if (n == NULL) //root
				break;
			if (idx++ == 3)
				goto BACK_TO_PARENT;
		}
		c+=m_maps[i].m_map_tree->node_count;
	}
	Log_r::Notice("all node count is %d",c);
#endif
}
struct _quadtree_node *
g_map::get_sub_trangler_by_pos(struct _quadtree * tree, _quadtree_node * node,
		const g_pos_3d<GLdouble> *p)
{
	for (int i = 0; i < 4; i++)
	{
		if (node->child[i] == NULL)
		{
			create_sub_triangler(tree, node);
			assert(node->child[i]!=NULL);
		}
		const g_pos_3d<GLdouble> *  _vlist = (static_cast<map_obj*>(node->child[i]->v))->m_sharp->m_vectors;
		if (IsIntersectTriangle(source_point, *p, _vlist[0],_vlist[1], _vlist[2]))
			return node->child[i];
	}
	Log_r::Error("level %ld is not contain point %f,%f,%f",QT_LEVEL(node->id),p->x,p->y,p->z);
	return NULL;
}
template <typename GT_TYPE>
static inline bool vector_in_the_same_dir(const g_pos_3d<GT_TYPE> &v,  const g_pos_3d<GT_TYPE> &v1)
{
	D_SUB_3D_VECTOR_TEMPLATE(tmp1,v,v1);
	D_ADD_3D_VECTOR_TEMPLATE(tmp2,v,v1);
	return ((tmp1.x*tmp1.x+tmp1.y*tmp1.y+tmp1.z*tmp1.z)<(tmp2.x*tmp2.x+tmp2.y*tmp2.y+tmp2.z*tmp2.z));
}
struct _quadtree_node * g_map::coordinate2quadtree_node_by_pos(const g_pos_3d<GLdouble> *p,int level, struct _quadtree_node **node_stack)
{
	glBegin(GL_POINTS);
	glColor3f(0.1f,0.2f,0.3f);
	glVertex3dv((double*)p);
	glEnd();
	for (int i = 0; i < 20; i++)
	{
		if(!vector_in_the_same_dir(*p,m_maps[i].m_map->m_center))
		{
			continue;
		}
		if (IsIntersectTriangle(source_point, *p,
				m_maps[i].m_map->m_sharp->m_vectors[0],
				m_maps[i].m_map->m_sharp->m_vectors[1],
				m_maps[i].m_map->m_sharp->m_vectors[2]))
		{
			struct _quadtree_node * node = &m_maps[i].m_map_tree->root;
			if (node_stack)
			{
				memset(node_stack,0,sizeof(struct _quadtree_node *)*level);
				node_stack[0] = node;
			}
			while (QT_LEVEL(node->id) <= (uint) level)
			{
				//(static_cast<map_obj*>(node->v))->draw();
				struct _quadtree_node * tmp_node = get_sub_trangler_by_pos(
						m_maps[i].m_map_tree, node, p);
				if (tmp_node == NULL)
				{
					if(node_stack!=NULL)
					for (int j=0;j<level;j++)
					{
						if(node_stack[j]!=NULL)
							static_cast<map_obj*>(node_stack[j]->v)->draw();
					}

					for(int j=0;j<4;j++)
					{
						(static_cast<map_obj*>(node->child[j]->v))->draw();
					}
					return NULL;
				}
				node = tmp_node;
				//assert(node!=NULL);
				if (node_stack)
					node_stack[QT_LEVEL(node->id)] = node;
			}
			return node;
		}
	}
	return NULL;
}
/*longitude */
struct _quadtree_node *
g_map::coordinate2quadtree_node(float longitude, float latitude, int level,
		struct _quadtree_node **node_stack)
{
	if (longitude > 180.0f || longitude < -180.0f || latitude > 90.0f
			|| latitude < -90.0f)
	{
		Log_r::Error("invalid argv %f %f in coordinate2quadtree_node",
				longitude, latitude);
		return NULL;
	}
	double rad_longitude = angle2rad(longitude), rad_latitude = angle2rad(
			latitude);
	g_pos_3d<GLdouble> p =
	{ sin(rad_longitude),  sin(rad_latitude),  cos(
			rad_longitude) };
	return coordinate2quadtree_node_by_pos(&p,level,node_stack);
}
int g_map::gen_view_id()
{
	m_prev_alloced_id=(m_prev_alloced_id+1)%m_max_id;
	int roolback_id = m_prev_alloced_id;
	do
	{
		if(test_bitmap(m_id_bitmap,m_prev_alloced_id))
			m_prev_alloced_id=(m_prev_alloced_id+1)%m_max_id;
		else
		{
			set_bitmap(m_id_bitmap,m_prev_alloced_id);
			return m_prev_alloced_id;
		}
	}while(m_prev_alloced_id!=roolback_id);
	m_prev_alloced_id = m_max_id;
	if(ALIGN(m_max_id*5/4,8)<m_max_id)
		m_max_id += 32;
	else
		m_max_id = ALIGN(m_max_id*5/4,8);
	m_id_bitmap = (unsigned char*)realloc(m_id_bitmap,m_max_id/8);
	set_bitmap(m_id_bitmap,m_prev_alloced_id);
	return m_prev_alloced_id;
}
g_map::map_view * g_map::create_view(int detail_level,const g_pos_3d<GLdouble> *p)
{
	int id  = gen_view_id();
	struct _quadtree_node *node = coordinate2quadtree_node_by_pos(p,detail_level,NULL);
	if(node == NULL)
		return NULL;
	for(struct _quadtree_node * tmp_node = node ;tmp_node!=NULL;tmp_node=tmp_node->parent)
		static_cast<map_obj*>(tmp_node->v)->rigister_user(id);
	//double max_distance = sqrt((double)(1.0f+static_cast<map_obj*>(node->v)->x))
	return NULL;
}
