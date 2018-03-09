/*
 * g_point.h
 *
 *  Created on: 2017年12月25日
 *      Author: liwei
 */

#ifndef G_G_POINT_H_
#define G_G_POINT_H_
#include <float.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#pragma pack(1)
template<typename GT_TYPE>
struct g_color
{
	GT_TYPE r;
	GT_TYPE g;
	GT_TYPE b;
	GT_TYPE a;
};
typedef struct g_color<GLfloat> g_color_f;
#define _pai 3.14159265358979323846L

#define INIT_POS_2D(p,xv,yv) (p)->x=(xv);(p)->y=(yv);
template<typename GT_TYPE>
struct g_pos_2d
{
	GT_TYPE x;
	GT_TYPE y;
	const g_pos_2d<GT_TYPE> & operator =(const g_pos_2d<GT_TYPE>& s)
	{
		this->x = s.x;
		this->y = s.y;
		return *this;
	}
	g_pos_2d<GT_TYPE> operator -(const g_pos_2d<GT_TYPE>& s)
	{
		return g_pos_2d<GT_TYPE>(this->x - s.x, this->y - s.y);
	}
	g_pos_2d<GT_TYPE> operator +(const g_pos_2d<GT_TYPE>& s)
	{
		return g_pos_2d<GT_TYPE>(this->x + s.x, this->y + s.y);
	}
};
/*计算向量的差*/
#define SUB_3D_VECTOR(d,s,bs)  (d).x=(s).x-(bs).x;(d).y=(s).y-(bs).y;(d).z=(s).z-(bs).z;

/*定义一个值为s与d的向量差的向量*/
/*decltype(s)在s为左值时，实际上上s的引用类型，需要使用std::remove_reference<decltype(s)>::type解引用*/
#define D_SUB_3D_VECTOR(d,s,bs) std::remove_reference<decltype(s)>::type d = {(s).x-(bs).x,(s).y-(bs).y,(s).z-(bs).z}
/*在s的类型是未定的模板时，std::remove_reference<decltype(s)>::type无法使用，只能使用非解引用的版本*/
#define D_SUB_3D_VECTOR_TEMPLATE(d,s,bs) decltype(s) d = {(s).x-(bs).x,(s).y-(bs).y,(s).z-(bs).z}

#define ADD_3D_VECTOR(a,ba,d) (d).x = do {(d).x=(a).x+(ba).x;(d).y=(a).y+(ba).y;(d).z=(a).z+(ba).z;}while(0)
#define D_ADD_3D_VECTOR(d,a,ba) std::remove_reference<decltype(s)>::type d = {(a).x+(ba).x,(a).y+(ba).y,(a).z+(ba).z}
#define D_ADD_3D_VECTOR_TEMPLATE(d,a,ba) decltype(a) d = {(a).x+(ba).x,(a).y+(ba).y,(a).z+(ba).z}

/*计算三点坐标为p1，p2，p3的三角形对应的平行四边形的面积----也就是三角形面积的两倍放到a中，根据三点的顺序，如果是顺时针，值为正数，反之为负数*/
#define CALCULATE_DOUBLE_TRANGLER_AREA(a,p1,p2,p3) do {D_SUB_3D_VECTOR(__tmp_p12,p2,p1);\
	D_SUB_3D_VECTOR(__tmp_p13,p3,p1);\
	a=(__tmp_p12.y*(__tmp_p13.z-__tmp_p13.x)+__tmp_p13.y*(__tmp_p12.x-__tmp_p12.z)+__tmp_p13.x*__tmp_p12.z+__tmp_p13.z*__tmp_p12.x);}while(0);

#define CALCULATE_TRANGLER_AREA(a,p1,p2,p3) CALCULATE_DOUBLE_TRANGLER_AREA(a,p1,p2,p3);(a)=abs((a)/(decltype(a))2);
template<typename GT_TYPE>
struct g_pos_3d //:public g_pos_2d<GT_TYPE>
{
	GT_TYPE x;
	GT_TYPE y;
	GT_TYPE z;
	const g_pos_3d<GT_TYPE> & operator =(const g_pos_3d<GT_TYPE>& s)
	{
		this->x = s.x;
		this->y = s.y;
		this->z = s.z;
		return *this;
	}
	g_pos_3d<GT_TYPE> operator -(const g_pos_3d<GT_TYPE>& s)
	{
		return g_pos_3d<GT_TYPE>(this->x - s.x, this->y - s.y, this->z - s.z);
	}
	g_pos_3d<GT_TYPE> operator +(const g_pos_3d<GT_TYPE>& s)
	{
		return g_pos_3d<GT_TYPE>(this->x + s.x, this->y + s.y, this->z + s.z);
	}
};
#pragma pack()
#define INIT_POS_3D(p,xv,yv,zv) (p)->x=xv;(p)->y=(yv);(p)->z=(zv);
#define INIT_G_POS(p) memset(&(p),0,sizeof(p))

#define VECTOR_3D_CROSS_MULTIPLY(m,v1,v2)     (m).x = (v1).y * (v2).z - (v1).z * (v2).y;\
(m).y = (v1).z * (v2).x - (v1).x * (v2).z;\
(m).z = (v1).x * (v2).y - (v1).y * (v2).x;
#define D_VECTOR_3D_CROSS_MULTIPLY(m,v1,v2) std::remove_reference<decltype(v1)>::type m = {(v1).y * (v2).z - (v1).z * (v2).y,(v1).z * (v2).x - (v1).x * (v2).z,(v1).x * (v2).y - (v1).y * (v2).x};
#define D_VECTOR_3D_CROSS_MULTIPLY_TEMPLATE(m,v1,v2) decltype(v1) m = {(v1).y * (v2).z - (v1).z * (v2).y,(v1).z * (v2).x - (v1).x * (v2).z,(v1).x * (v2).y - (v1).y * (v2).x};

#define VECTOR_3D_DOT_MULTIPLY(v1,v2) ((v1).x*(v2).x+(v1).y*(v2).y+(v1).z*(v2).z)

#define CALCULATE_COS(v,v1) (VECTOR_3D_DOT_MULTIPLY(v,v1)/sqrt((double)((v).x*(v).x+(v).y*(v).y+(v).z*(v).z+(v1).x*(v1).x+(v1).y*(v1).y+(v1).z*(v1).z)))

template<typename GT_TYPE>
static void calculate_normal(g_pos_3d<GT_TYPE> & normal,
		const g_pos_3d<GT_TYPE> &v, const g_pos_3d<GT_TYPE> &v1,
		const g_pos_3d<GT_TYPE> &v2)
{
	D_SUB_3D_VECTOR_TEMPLATE(v1_tmp, v1, v);
	D_SUB_3D_VECTOR_TEMPLATE(v2_tmp, v2, v);

	normal.x = v1_tmp.y * v2_tmp.z - v1_tmp.z * v2_tmp.y;
	normal.y = v1_tmp.z * v2_tmp.x - v1_tmp.x * v2_tmp.z;
	normal.z = v1_tmp.x * v2_tmp.y - v1_tmp.y * v2_tmp.x;
	GT_TYPE mode = GT_TYPE(1)
			/ sqrt(
					static_cast<double>(normal.x * normal.x
							+ normal.y * normal.y + normal.z * normal.z));
	normal.x *= mode;
	normal.y *= mode;
	normal.z *= mode;
}

// Determine whether a ray intersect with a triangle
// Parameters
// orig: origin of the ray
// dir: direction of the ray
// v0, v1, v2: vertices of triangle
// t(out): weight of the intersection for the ray
// u(out), v(out): barycentric coordinate of intersection
template<typename GT_TYPE>
static bool IntersectTriangle(const g_pos_3d<GT_TYPE>& orig,
		const g_pos_3d<GT_TYPE>& dir, const g_pos_3d<GT_TYPE>& v0,
		const g_pos_3d<GT_TYPE>& v1, const g_pos_3d<GT_TYPE>& v2, float* t, float* u,
		float* v)
{
	// E1
	D_SUB_3D_VECTOR_TEMPLATE(E1, v1, v0);

	// E2
	D_SUB_3D_VECTOR_TEMPLATE(E2, v2, v0);

	// P
	D_VECTOR_3D_CROSS_MULTIPLY_TEMPLATE(P, dir, E2);

	// determinant
	GT_TYPE det = VECTOR_3D_DOT_MULTIPLY(E1, P);

	// keep det > 0, modify T accordingly
	g_pos_3d<GT_TYPE> T;
	if (det > 0)
	{
		SUB_3D_VECTOR(T, orig, v0);
	}
	else
	{
		SUB_3D_VECTOR(T, v0, orig);
		det = -det;
	}

	// If determinant is near zero, ray lies in plane of triangle
	if (det < 0.00001f)
		return false;

	// Calculate u and make sure u <= 1
	*u = VECTOR_3D_DOT_MULTIPLY(T, P);
	if (*u < 0.0f || *u > det)
		return false;

	// Q
	D_VECTOR_3D_CROSS_MULTIPLY_TEMPLATE(Q, T, E1);

	// Calculate v and make sure u + v <= 1
	*v = VECTOR_3D_DOT_MULTIPLY(dir, Q);
	if (*v < 0.0f || *u + *v > det)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	*t = VECTOR_3D_DOT_MULTIPLY(E2, Q);

	float fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return true;
}

template<typename GT_TYPE>
GT_TYPE type_min(const GT_TYPE &t);

template<>
GLfloat type_min(const GLfloat &t);
template<>
GLdouble type_min(const GLdouble &t);
template<typename GT_TYPE>
static bool IsIntersectTriangle(const g_pos_3d<GT_TYPE>& orig,
		const g_pos_3d<GT_TYPE>& dir, const g_pos_3d<GT_TYPE>& v0,
		const g_pos_3d<GT_TYPE>& v1, const g_pos_3d<GT_TYPE>& v2)
{
	// E1
	D_SUB_3D_VECTOR_TEMPLATE(E1, v1, v0);

	// E2
	D_SUB_3D_VECTOR_TEMPLATE(E2, v2, v0);

	// P
	D_VECTOR_3D_CROSS_MULTIPLY_TEMPLATE(P, dir, E2);

	// determinant
	GT_TYPE det = VECTOR_3D_DOT_MULTIPLY(E1, P);

	// keep det > 0, modify T accordingly
	g_pos_3d<GT_TYPE> T;
	if (det > 0)
	{
		SUB_3D_VECTOR(T, orig, v0);
	}
	else
	{
		SUB_3D_VECTOR(T, v0, orig);
		det = -det;
	}

	// If determinant is near zero, ray lies in plane of triangle
	if (det < type_min(det))
		return false;

	// Calculate u and make sure u <= 1
	float u = VECTOR_3D_DOT_MULTIPLY(T, P);
	if (u < 0.0f || u > det)
		return false;
	// Q
	D_VECTOR_3D_CROSS_MULTIPLY_TEMPLATE(Q, T, E1);
	// Calculate v and make sure u + v <= 1
	float v = VECTOR_3D_DOT_MULTIPLY(dir, Q);
	if (v < 0.0f || u + v > det)
		return false;
	return true;
}
#endif /* G_G_POINT_H_ */
