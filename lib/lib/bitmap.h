#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H 
#include "global.h"   // 用来在位图中逐位判断
#define BITMAP_MASK 1 
struct bitmap {
    uint32_t btmp_bytes_len;    // 在遍历位图时，整体上以字节为单位，细节上以位为单位,代表位图的字节长度    
    uint8_t* bits;              //所以这里的指针是单个字节，位图的指针
};

void bitmap_init(struct bitmap *btemp);
int bitmap_scan_test(struct bitmap* btemp,uint32_t bit_idx);
int bitmap_scan(struct bitmap* btmp, uint32_t cnt);
void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value);
#endif
