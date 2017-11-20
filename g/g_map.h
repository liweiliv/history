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
#include <stdint.h>
struct zone
{
    uint64_t longitude;
    uint64_t latitude;
    uint64_t diameter;
};
enum event_type
{

};
struct event
{
    uint16_t type;
    uint16_t length;
    uint64_t event_id;
    union{
        uint64_t group_id;
        uint64_t object_id;
    };
    char event_data[1];
};
struct time_line_block
{
    uint64_t start_time;
    uint64_t last_update_time;
    uint64_t next_data_offset;
    char time_line_data[1];
};
struct time_line
{
    struct zone zone;
    uint32_t default_buf_size;
    time_line_block frtst_block;
};

class g_map
{
    int m_diameter;
    float m_obliguity;
    int m_rotation_period;
    float m_orbital_inclination ;
    int m_orbital_period;
    char m_map_index_file[256];
};




#endif /* G_G_MAP_H_ */
