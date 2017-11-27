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
};
#define INIT_POS_3D(p,xv,yv,zv) INIT_POS_2D(p,xv,yv) (p)->z=(zv);
template <typename GT_TYPE>
struct g_pos_3d :public g_pos_2d<GT_TYPE>
{
    GT_TYPE z;
    const g_pos_3d<GT_TYPE> & operator =(const g_pos_3d<GT_TYPE>& s)
    {
        this->g_pos_2d = s.g_pos_2d;
        this->y = s.y;
        return *this;
    }
};
#pragma pack()
#define INIT_G_POS(p) memset(&(p),0,sizeof(p))
template <class GT_TYPE,template <class GT_TYPE> class POS_TYPE>
class g_sharp
{
public:
    POS_TYPE<GT_TYPE> * m_vectors;
    POS_TYPE<GLfloat> *m_normals;
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
            m_mem_size=(sizeof(POS_TYPE<GT_TYPE>)+sizeof(POS_TYPE<GLfloat>))*m_point_size+(sizeof(g_color<GLfloat>)+sizeof(GLubyte))*index_size;
        else
            m_mem_size=(sizeof(POS_TYPE<GT_TYPE>)+sizeof(POS_TYPE<GLfloat>)+sizeof(g_color<GLfloat>))*m_point_size;
        m_mem = malloc(m_mem_size);
        m_vectors = (POS_TYPE<GT_TYPE> *)m_mem;
        m_normals = &m_vectors[m_point_size];
        m_colors = (g_color<GLfloat> *)&m_normals[m_point_size];
        if(index_size)
            m_indices = (GLubyte*)&m_colors[index_size];
        else
            m_indices = NULL;
        m_vbo_buf_id = 0xffffffff;
        m_point_size = 0;
        m_draw_type = 0;
        m_use_vbo = use_vbo;
    }
    g_sharp(const g_sharp<GT_TYPE,POS_TYPE> &s)
    {
        m_point_size = s.m_point_size;
        m_draw_type = s.m_draw_type;
        m_index_size = s.m_index_size;
        if(m_point_size)
        {
            assert(s.m_mem);
            assert(s.m_mem_size==(sizeof(POS_TYPE<GT_TYPE>)+sizeof(POS_TYPE<GLfloat>)+sizeof(g_color<GLfloat>))*s.m_point_size);
            m_mem_size = s.m_mem_size;
            m_mem = malloc(m_mem_size);
            memcpy(m_mem,s.m_mem,m_mem_size);
            m_vectors = (POS_TYPE<GT_TYPE> *)m_mem;
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
    void set_sharp(POS_TYPE<GT_TYPE> * vectors,POS_TYPE<GT_TYPE> *normals,g_color<GT_TYPE> colors,uint32_t point_size)
    {
        if(m_mem)
            free(m_mem);
        if(m_vbo_buf_id == -1)
        {
            glDeleteBuffersARB(1, &m_vbo_buf_id);
            m_vbo_buf_id = 0xffffffff;
        }
        m_point_size = point_size;
        m_mem_size = (sizeof(POS_TYPE<GT_TYPE>)+sizeof(POS_TYPE<GLfloat>)+sizeof(g_color<GLfloat>))*m_point_size;
        m_mem = malloc(m_mem_size);
        m_vectors = (POS_TYPE<GT_TYPE> *)m_mem;
        m_normals = &m_vectors[m_point_size];
        m_colors = (g_color<GLfloat> *)&m_normals[m_point_size];
        memcpy(m_vectors,vectors,sizeof(POS_TYPE<GT_TYPE>)*m_point_size);
        memcpy(m_normals,normals,sizeof(POS_TYPE<GLfloat>)*m_point_size);
        memcpy(m_colors,colors,sizeof(g_color<GLfloat>)*m_point_size);
    }
private:
    inline int draw_by_vbo(POS_TYPE<GT_TYPE> *pos,POS_TYPE<GT_TYPE> *dirct)
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
        glNormalPointer(GL_FLOAT, 0, (void*)(sizeof(POS_TYPE<GT_TYPE>)*m_point_size));
        glColorPointer(4, GL_FLOAT, 0, (void*)((sizeof(POS_TYPE<GT_TYPE>)+sizeof(POS_TYPE<GLfloat>))*m_point_size));
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
    inline int draw_normal(POS_TYPE<GT_TYPE> *pos,POS_TYPE<GT_TYPE> *dirct)
    {
        // enable vertex arrays
        //glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        if(m_indices)
            glEnableClientState(GL_INDEX_ARRAY);
        // before draw, specify vertex and index arrays with their offsets
        //glNormalPointer(GL_FLOAT, 0, m_normals);
        glColorPointer(3, GL_FLOAT, 0, m_colors);
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
        //glDisableClientState(GL_NORMAL_ARRAY);
        if(m_indices)
            glDisableClientState(GL_INDEX_ARRAY);
        return 0;
    }
public:
    void draw(POS_TYPE<GT_TYPE> *pos,POS_TYPE<GT_TYPE> *dirct)
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
