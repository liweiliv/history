/*
 * bitmap.h
 *
 *  Created on: 2018年1月8日
 *      Author: liwei
 */

#ifndef UTIL_BITMAP_H_
#define UTIL_BITMAP_H_
typedef unsigned char* bitmap;

#define set_bitmap(m,idx)  (m)[(idx)>>3] |= (1<<((idx)&0x07))
#define unset_bitmap(m,idx) (m)[(idx)>>3] &= (~(1<<((idx)&0x07)))
#define test_bitmap(m,idx) ((m)[(idx)>>3]&(1<<((idx)&0x07)))

#endif /* UTIL_BITMAP_H_ */
