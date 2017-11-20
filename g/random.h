/*
 * random.h
 *
 *  Created on: 2017年11月3日
 *      Author: liwei
 */

#ifndef G_RANDOM_H_
#define G_RANDOM_H_
//copy from LevelDB ,used in c program
#include <stdint.h>
typedef uint32_t t_random;
#define RANDOM_M (uint32_t)2147483647L
#define RANDOM_A (uint64_t)16807
#define init_random(r,s) do  \
{\
    (r)=(s)&0x7fffffffu;\
    if (r == 0 || r == 2147483647L) \
      r = 1;\
}while(0)
#define next_random(r) \
do { \
    uint64_t product = (r) * RANDOM_A;\
    (r) = (uint32_t)((product >> 31) + (product & RANDOM_M));\
    if ((r) > RANDOM_M) \
        (r) -= RANDOM_M;\
}while(0);
#endif /* G_RANDOM_H_ */
