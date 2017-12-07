/*
 * hash.c
 *
 *  Created on: 2016年12月9日
 *      Author: liwei
 *
 */
#include <stdint.h>
#include <stddef.h>

//继承自leveldb
uint32_t c_hash(const char* data, size_t n, uint32_t seed) {
  // Similar to murmur hash
  const uint32_t m = 0xc6a4a793;
  const uint32_t r = 24;
  const char* limit = data + n;
  uint32_t h = seed ^ (n * m);

  // Pick up four bytes at a time
  while (data + 4 <= limit) {
    uint32_t w = *(uint32_t*)data;
    data += 4;
    h += w;
    h *= m;
    h ^= (h >> 16);
  }

  // Pick up remaining bytes
  switch (limit - data) {
    case 3:
      h += (unsigned char)(data[2]) << 16;
    case 2:
      h += (unsigned char)(data[1]) << 8;
    case 1:
      h += (unsigned char)(data[0]);
      h *= m;
      h ^= (h >> r);
      break;
  }
  return h;
}
//todo
uint32_t hash64_32shift(int64_t key)
{
  key = (~key) + (key << 18); // key = (key << 18) - key - 1;
  key = key ^ (key >> 31);
  key = key * 21; // key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (uint32_t) key;
}


