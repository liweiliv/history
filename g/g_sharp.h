/*
 * g_sharp.h
 *
 *  Created on: 2017年9月14日
 *      Author: liwei
 */
#include <stdlib.h>
#include <stdint.h>
#ifndef G_SHARP_H_
#define G_SHARP_H_
typedef float GT_FLOT;
typedef uint32_t GT_UINT;
typedef int32_t GT_INT;
typedef uint16_t GT_USHORT;
typedef int16_t GT_SHORT;
typedef uint8_t GT_UCHAR;
typedef int8_t GT_CHAR;

#pragma pack(1)
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
protected:
    POS_TYPE<GT_TYPE> * m_sharp;
    POS_TYPE<GT_TYPE>  m_sharp_pos;
    POS_TYPE<GT_TYPE>  m_direct_vector;
    uint32_t m_sharp_size;
    uint8_t m_draw_type;
public:
    g_sharp()
    {
        m_sharp = NULL;
        INIT_G_POS(m_sharp_pos);
        INIT_G_POS(m_direct_vector);
        m_sharp_size = 0;
        m_draw_type = 0;
    }
    g_sharp(const g_sharp<GT_TYPE,POS_TYPE> &s)
    {
        m_sharp_size = s.m_sharp_size;
        if(m_sharp_size)
        {
            if(!s.m_sharp)
                abort();
            m_sharp = (POS_TYPE<GT_TYPE> *)malloc(sizeof(POS_TYPE<GT_TYPE> )*m_sharp_size);
            memcpy(m_sharp,s.m_sharp,sizeof(POS_TYPE<GT_TYPE> )*m_sharp_size);
        }
        else
        {
            m_draw_type = s.m_draw_type;
            m_sharp = NULL;
        }
        m_sharp_pos = s.m_sharp_pos;
        m_direct_vector = s.m_direct_vector;
    }
    ~g_sharp()
    {
        if(m_sharp)
            free(m_sharp);
    }
    void set_sharp(POS_TYPE<GT_TYPE> * sharp,uint32_t sharp_size)
    {
        if(m_sharp)
            free(m_sharp);
        m_sharp_size = sharp_size;
        m_sharp = (POS_TYPE<GT_TYPE> *)malloc(sizeof(POS_TYPE<GT_TYPE> )*m_sharp_size);
        memcpy(m_sharp,sharp,sizeof(POS_TYPE<GT_TYPE> )*m_sharp_size);
    }
    void draw()
    {
        if(m_sharp_size&&m_sharp)
        {
            for(int i=0;i<m_sharp_size;i++)
            {
                for(int j=0;j<sizeof(POS_TYPE<GT_TYPE>)/sizeof(GT_TYPE);j++)
                    printf("%d ",((GT_TYPE*)&m_sharp[i])[j]);
                printf("\n");
            }
        }
    }
};




#endif /* G_SHARP_H_ */
