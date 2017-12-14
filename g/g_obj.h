/*
 * g_obj.h
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#ifndef G_G_OBJ_H_
#define G_G_OBJ_H_

enum g_obj_type
{
    GT_TREE, GT_STORN, GT_HUMAN, GT_WATER, GT_ARMOR, GT_WEAPON,GT_MAP
} ;
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "file_opt.h"
#include "g_sharp.h"
#define GL_OBJ_DOUBLE 0x01UL
#define GL_OBJ_FLOAT 0x02UL
#define GL_OBJ_INT 0x03UL
#define GL_OBJ_UINT 0x04UL
#define GL_OBJ_CHAR 0x05UL
#define GL_OBJ_UCHAR 0x06UL
#define GL_OBJ_SHORT 0x07UL
#define GL_OBJ_USHORT 0x08UL
#define GL_OBJ_LONG 0x09UL
#define GL_OBJ_ULONG 0x0aUL

#define GL_OBJ_OFF 60
#define GL_OBJ_MASk (0xfUL <<GL_OBJ_OFF)
#define GL_OBJ_TYPE(obj) ((obj)>>GL_OBJ_OFF)
#define GET_GL_OBJ(obj) ((void*)(((uint64_t)(obj))&(~GL_OBJ_MASk)))
#define CREATE_GL_OBJ(obj,type) ((void*)((obj)|((type)<<GL_OBJ_OFF)))
template<typename GT_TYPE>
class g_obj
{
public:
    uint32_t m_id;
    g_sharp< g_pos_3d<GT_TYPE>> * m_sharp;
    bool m_shape_share;
    char * m_source;
public:
    const g_obj_type m_type;
    g_obj(g_obj_type type, uint32_t id) :
            m_id(id), m_sharp(NULL), m_shape_share(false), m_source(NULL), m_type(
                    type)
    {
    }
    virtual ~g_obj()
    {
        if (m_shape_share && m_sharp)
            delete m_sharp;
    }
    void set_shape(const g_sharp<g_pos_3d<GT_TYPE>> * shape)
    {
        if (m_sharp != NULL && m_shape_share)
            delete m_sharp;
        m_sharp = shape;
        m_shape_share = true;
    }
    void set_shape_by_copy(const g_sharp< g_pos_3d<GT_TYPE>> * shape)
    {
        if (m_sharp != NULL && m_shape_share)
            delete m_sharp;
        m_sharp = new g_sharp<g_pos_3d<GT_TYPE>>(shape);
        m_shape_share = false;
    }
    inline uint32_t get_id() {return m_id;}
    virtual void draw()  =0;
    enum OBJ_LOAD_STATUS
    {
        OLS_HEAD,
        OLS_VECTOR,
        OLS_NOM
    };
    static g_obj<GT_TYPE> *load_from_mem(const char * data, size_t size)
    {
        return NULL;
    }

    static g_obj<GT_TYPE> *load_from_file(const char *file)
    {
#if 0
        FD_TYPE fd = open_file(file,F_RDONLY);
        if(fd<0)
            return NULL;
        char buf[1024]={0};
        int readed_count = 0,read_count=1023;
        g_obj<GT_TYPE> * obj = new g_obj<GT_TYPE>(0,0);
        while(0>=(read_count=file_read(fd,(unsigned char *)buf,read_count)))
        {

        }
        return NULL;
#endif
    }
};

#if 0
template <typename GT_TYPE>
class g_tree :public g_obj<GT_TYPE>
{
protected:
    uint16_t m_age;
    g_color * m_color_of_month;
public:
    g_tree(uint32_t id):g_obj(GT_TREE,(id&0x0000ffff)|(((uint32_t)GT_TREE)<<16)),m_age(0),m_color_of_month(NULL)
    {
    }
};
#endif

#endif /* G_G_OBJ_H_ */
