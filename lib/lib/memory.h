#ifndef  __KERNEL_MEMORY_H
#define  __KERNEL_MEMORY_H
#include "stdio.h"
#include "bitmap.h"
#include "list.h"
// 虚拟地址池，用于虚拟地址管理
struct virtual_addr{
    struct bitmap vaddr_bitmap;   // 虚拟地址用到的位图结构
    uint32_t vaddr_start;    // 虚拟地址的起始地址
};
extern struct pool kernel_pool, user_pool;
void mem_init(void);

enum pool_flags {
    PF_KERNEL = 1,   // 内核内存池
    PF_USER = 2         // 用户内存池
};

#define  PG_P_1 1  //页表项或者页目录项存在属性位
#define  PG_P_0 0  
#define  PG_RW_R 0  // R/W属性位，读执行
#define  PG_RW_W 2  // 读写执行
#define  PG_US_S 0  // U/S属性位系统级，，012特权级可以访问
#define  PG_US_U 4  // U/S属性位用户级，所有用户都可以访问
extern struct pool kernel_pool, user_pool;
void mem_init(void);
void* get_kernel_pages(uint32_t pg_cnt);
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void malloc_init(void);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
void* get_a_page(enum pool_flags pf, uint32_t vaddr);
void* get_user_pages(uint32_t pg_cnt);
//void block_desc_init(struct mem_block_desc*);
void* sys_malloc(uint32_t size);
void mfree_page(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt);
void pfree(uint32_t pg_phy_addr);
void sys_free(void* ptr);
void* get_a_page_without_opvaddrbitmap(enum pool_flags pf, uint32_t vaddr);
// 内存块
struct mem_block {
    struct list_elem free_elem;
};

// 内存块描述符
struct mem_block_desc {
    uint32_t block_size;    // 内存块的大小
    uint32_t blocks_per_arena;   // 本arena中可容纳此mem_block的数量,这个数量是一个arena可以提供的数量
    struct list free_list;       // 目前可用的mem_block链表,可以由多个arena提供
};

#define  DESC_CNT 7

#endif
