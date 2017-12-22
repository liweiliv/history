/*
 * g_sharp.h
 *
 *  Created on: 2017年9月14日
 *      Author: liwei
 */
#ifndef G_SHARP_H_
#define G_SHARP_H_
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#pragma pack(1)
template <typename GT_TYPE>
struct g_color{
    GT_TYPE r;
    GT_TYPE g;
    GT_TYPE b;
    GT_TYPE a;
};
typedef  struct g_color<GLfloat> g_color_f;

#define INIT_POS_2D(p,xv,yv) (p)->x=(xv);(p)->y=(yv);
template <typename GT_TYPE>
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
    g_pos_2d<GT_TYPE>  operator -(const g_pos_2d<GT_TYPE>& s)
   {
        return g_pos_2d<GT_TYPE> (this->x - s.x,this->y - s.y);
   }
    g_pos_2d<GT_TYPE>  operator +(const g_pos_2d<GT_TYPE>& s)
   {
        return g_pos_2d<GT_TYPE> (this->x + s.x,this->y + s.y);
   }
};
/*计算向量的差*/
#define SUB_3D_VECTOR(s,bs,d) (d).x = do {(d).x=(s).x-(bs).x;(d).y=(s).y-(bs).y;(d).z=(s).z-(bs).z;}while(0)

/*定义一个值为s与d的向量差的向量*/
/*decltype(s)在s为左值时，实际上上s的引用类型，需要使用std::remove_reference<decltype(s)>::type解引用*/
#define D_SUB_3D_VECTOR(d,s,bs) std::remove_reference<decltype(s)>::type d = {(s).x-(bs).x,(s).y-(bs).y,(s).z-(bs).z}
/*在s的类型是未定的模板时，std::remove_reference<decltype(s)>::type无法使用，只能使用非解引用的版本*/
#define D_SUB_3D_VECTOR_TEMPLATE(d,s,bs) decltype(s) d = {(s).x-(bs).x,(s).y-(bs).y,(s).z-(bs).z}


#define ADD_3D_VECTOR(a,ba,d) (d).x = do {(d).x=(a).x+(ba).x;(d).y=(a).y+(ba).y;(d).z=(a).z+(ba).z;}while(0)
#define D_ADD_3D_VECTOR(d,a,ba) std::remove_reference<decltype(s)>::type d = {(a).x+(ba).x,(a).y-(ba).y,(a).z-(ba).z}
#define D_ADD_3D_VECTOR_TEMPLATE(d,a,ba) decltype(s) d = {(a).x+(ba).x,(a).y-(ba).y,(a).z-(ba).z}

/*计算三点坐标为p1，p2，p3的三角形对应的平行四边形的面积----也就是三角形面积的两倍放到a中，根据三点的顺序，如果是顺时针，值为正数，反之为负数*/
#define CALCULATE_DOUBLE_TRANGLER_AREA(a,p1,p2,p3) do {D_SUB_3D_VECTOR(__tmp_p12,p2,p1);\
	D_SUB_3D_VECTOR(__tmp_p13,p3,p1);\
	a=(__tmp_p12.y*(__tmp_p13.z-__tmp_p13.x)+__tmp_p13.y*(__tmp_p12.x-__tmp_p12.z)+__tmp_p13.x*__tmp_p12.z+__tmp_p13.z*__tmp_p12.x);}while(0);

#define CALCULATE_TRANGLER_AREA(a,p1,p2,p3) CALCULATE_DOUBLE_TRANGLER_AREA(a,p1,p2,p3);(a)=abs((a)/(decltype(a))2);
template <typename GT_TYPE>
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
     g_pos_3d<GT_TYPE>  operator -(const g_pos_3d<GT_TYPE>& s)
    {
         return g_pos_3d<GT_TYPE> (this->x - s.x,this->y - s.y,this->z -s.z);
    }
     g_pos_3d<GT_TYPE>  operator +(const g_pos_3d<GT_TYPE>& s)
    {
         return g_pos_3d<GT_TYPE> (this->x + s.x,this->y + s.y,this->z + s.z);
    }
};
#pragma pack()
#define INIT_POS_3D(p,xv,yv,zv) (p)->x=xv;(p)->y=(yv);(p)->z=(zv);
#define INIT_G_POS(p) memset(&(p),0,sizeof(p))
template<typename GT_TYPE>
static void calculate_normal(g_pos_3d<GT_TYPE> & normal,const g_pos_3d<GT_TYPE> &v,const g_pos_3d<GT_TYPE> &v1,const g_pos_3d<GT_TYPE> &v2)
{
	D_SUB_3D_VECTOR_TEMPLATE(v1_tmp,v1,v);
	D_SUB_3D_VECTOR_TEMPLATE(v2_tmp,v2,v);

    normal.x = v1_tmp.y * v2_tmp.z - v1_tmp.z * v2_tmp.y;
    normal.y = v1_tmp.z * v2_tmp.x - v1_tmp.x * v2_tmp.z;
    normal.z = v1_tmp.x * v2_tmp.y - v1_tmp.y * v2_tmp.x;
    GT_TYPE mode = GT_TYPE(1)/sqrt(static_cast<double>(normal.x*normal.x+normal.y*normal.y+normal.z*normal.z));
    normal.x*=mode;
    normal.y*=mode;
    normal.z*=mode;
}
template < typename POS_TYPE>
class g_sharp
{
public:
    POS_TYPE * m_vectors;
    POS_TYPE  *m_normals;
    g_color<GLfloat> * m_colors;
    GLubyte *m_indices;
    uint32_t m_index_size;
    void * m_mem;
    uint32_t m_mem_size;
    uint32_t m_point_size;
    GLuint  m_vbo_buf_id;
    uint8_t m_draw_type;
    bool m_use_vbo;

    g_sharp(uint32_t point_size,uint32_t index_size = 0,bool use_vbo = false)
    {
        m_point_size = point_size;
        m_index_size = index_size;
        if(index_size)
            m_mem_size=sizeof(POS_TYPE)*m_point_size+(sizeof(g_color<GLfloat>)+sizeof(GLubyte)+sizeof(POS_TYPE))*index_size;
        else
            m_mem_size=(sizeof(POS_TYPE)+sizeof(POS_TYPE)+sizeof(g_color<GLfloat>))*m_point_size;
        m_mem = malloc(m_mem_size);
        m_vectors = (POS_TYPE *)m_mem;
        m_normals = &m_vectors[m_point_size];
        m_colors = (g_color<GLfloat> *)&m_normals[m_point_size];
        if(index_size)
            m_indices = (GLubyte*)&m_colors[index_size];
        else
            m_indices = NULL;
        m_vbo_buf_id = 0xffffffff;
        m_draw_type = 0;
        m_use_vbo = use_vbo;
    }
    g_sharp(const g_sharp<POS_TYPE> &s)
    {
        m_point_size = s.m_point_size;
        m_draw_type = s.m_draw_type;
        m_index_size = s.m_index_size;
        if(m_point_size)
        {
            assert(s.m_mem);
            assert(s.m_mem_size==(sizeof(POS_TYPE)+sizeof(POS_TYPE)+sizeof(g_color<GLfloat>))*s.m_point_size);
            m_mem_size = s.m_mem_size;
            m_mem = malloc(m_mem_size);
            memcpy(m_mem,s.m_mem,m_mem_size);
            m_vectors = (POS_TYPE *)m_mem;
            m_normals = &m_vectors[m_point_size];
            m_colors = (g_color<GLfloat> *)&m_normals[m_point_size];
            if(m_index_size)
                m_indices = (GLubyte*)&m_colors[m_index_size];
            else
                m_indices = NULL;
        }
        else
        {
            m_mem = NULL;
            m_vectors = NULL;
            m_normals = NULL;
            m_colors = NULL;
            m_indices = NULL;
        }
        m_use_vbo = s.m_use_vbo;
        //lazy create vbo buf
        m_vbo_buf_id = -1;
    }
    ~g_sharp()
    {
        if(m_vbo_buf_id == 0xffffffff)
            glDeleteBuffersARB(1, &m_vbo_buf_id);
        if(m_mem)
            free(m_mem);
    }
    void set_sharp(POS_TYPE * vectors,POS_TYPE *normals,g_color<GLfloat> colors,uint32_t point_size)
    {
        if(m_mem)
            free(m_mem);
        if(m_vbo_buf_id == -1)
        {
            glDeleteBuffersARB(1, &m_vbo_buf_id);
            m_vbo_buf_id = 0xffffffff;
        }
        m_point_size = point_size;
        m_mem_size = (sizeof(POS_TYPE)+sizeof(POS_TYPE)+sizeof(g_color<GLfloat>))*m_point_size;
        m_mem = malloc(m_mem_size);
        m_vectors = (POS_TYPE *)m_mem;
        m_normals = &m_vectors[m_point_size];
        m_colors = (g_color<GLfloat> *)&m_normals[m_point_size];
        memcpy(m_vectors,vectors,sizeof(POS_TYPE)*m_point_size);
        memcpy(m_normals,normals,sizeof(POS_TYPE)*m_point_size);
        memcpy(m_colors,colors,sizeof(g_color<GLfloat>)*m_point_size);
    }
private:
    inline int draw_by_vbo(POS_TYPE *pos,POS_TYPE *dirct)
    {
        if(m_vbo_buf_id == 0xffffffff)
        {
            glGenBuffersARB(1, &m_vbo_buf_id);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_mem_size, 0, GL_STATIC_DRAW_ARB);
        }
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo_buf_id);

        // enable vertex arrays
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        if(m_indices)
            glEnableClientState(GL_INDEX_ARRAY);
        // before draw, specify vertex and index arrays with their offsets
        glNormalPointer(GL_FLOAT, 0, (void*)(sizeof(POS_TYPE)*m_point_size));
        glColorPointer(sizeof(g_color<GLfloat>)/sizeof(GLfloat), GL_FLOAT, 0, (void*)((sizeof(POS_TYPE)+sizeof(POS_TYPE))*m_point_size));
        glVertexPointer(3, GL_FLOAT, 0, 0);
        if(m_indices)
        {
            glDrawElements(GL_TRIANGLES, m_index_size, GL_UNSIGNED_BYTE, m_indices);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, m_point_size);
        }
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if(m_indices)
            glDisableClientState(GL_INDEX_ARRAY);

        // it is good idea to release VBOs with ID 0 after use.
        // Once bound with 0, all pointers in gl*Pointer() behave as real
        // pointer, so, normal vertex array operations are re-activated
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        return 0;
    }
    inline int draw_normal(POS_TYPE *pos,POS_TYPE *dirct)
    {
        // enable vertex arrays
        //glEnableClientState(GL_NORMAL_ARRAY);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        if(m_indices)
            glEnableClientState(GL_INDEX_ARRAY);
        // before draw, specify vertex and index arrays with their offsets
        glNormalPointer(GL_FLOAT, 0, m_normals);
        glColorPointer(4, GL_FLOAT, 0, m_colors);
        glVertexPointer(3, GL_FLOAT, 0, m_vectors);
        if(m_indices)
        {
            glDrawElements(GL_TRIANGLES, m_index_size, GL_UNSIGNED_BYTE, m_indices);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, m_point_size);
        }
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if(m_indices)
            glDisableClientState(GL_INDEX_ARRAY);
        return 0;
    }
public:
    void draw(POS_TYPE *pos,POS_TYPE *dirct)
    {
        if(m_mem_size&&m_mem)
        {
            if(m_use_vbo)
                draw_by_vbo(pos,dirct);
            else
                draw_normal(pos,dirct);
        }
    }
};




#endif /* G_SHARP_H_ */
