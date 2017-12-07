/*
 * bloom_filter.h
 *
 *  Created on: 2016年12月12日
 *      Author: liwei
 */

#ifndef LIB_CALCULATE_ENGINE_BLOOM_FILTER_H_
#define LIB_CALCULATE_ENGINE_BLOOM_FILTER_H_

typedef struct _bloom_filter
{
    size_t bits_per_key;
    size_t k;
    size_t size;
    char bits[1];
} bloom_filter;

void init_bloom_filter(bloom_filter * filter, int bits_per_key, size_t key_size);
void insertFilter(bloom_filter * filter, const char * key, size_t size);
int KeyMayMatch(const bloom_filter * filter, const char * key, size_t size);

#endif /* LIB_CALCULATE_ENGINE_BLOOM_FILTER_H_ */
