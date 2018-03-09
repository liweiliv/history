/*
 * g_sharp.cpp
 *
 *  Created on: 2018年1月15日
 *      Author: liwei
 */
#include <string.h>
#include "g_sharp.h"

template<>
void g_sharp<g_pos_3d<GLdouble>>::set_vbo_data()
{
    glNormalPointer(GL_DOUBLE, 0, (void*)(sizeof(GLdouble)*m_point_size));
    glColorPointer(sizeof(g_color<GLfloat>)/sizeof(GLfloat), GL_FLOAT, 0, (void*)((sizeof(GLdouble)+sizeof(GLdouble))*m_point_size));
    glVertexPointer(3, GL_DOUBLE, 0, 0);
}
template<>
void g_sharp<g_pos_3d<GLdouble>>::set_normally_data()
{
    glNormalPointer(GL_DOUBLE, 0, m_normals);
    glColorPointer(4, GL_FLOAT, 0, m_colors);
    glVertexPointer(3, GL_DOUBLE, 0, m_vectors);
}






