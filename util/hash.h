/*
 * hash.h
 *
 *  Created on: 2016年12月9日
 *      Author: liwei
 */

#ifndef LIB_CALCULATE_ENGINE_HASH_H_
#define LIB_CALCULATE_ENGINE_HASH_H_


uint32_t c_hash(const char* data, size_t n, uint32_t seed);
uint32_t hash64_32shift(int64_t key);

#endif /* LIB_CALCULATE_ENGINE_HASH_H_ */
