#ifndef  __THREAD_SYNC_H
#define  __THREAD_SYNC_H
#include "list.h"
#include "stdint.h"
#include "thread.h"

// 信号量结构
// 信号量只是一个变成理念，是个程序设计结构，只是具备信号了初值
// 和等待线程这两个元素就可以
struct semaphore {
   uint8_t value;
   struct list waiters;    // 用它来纪录在此信号量上等待(阻塞)的所有线程
};

struct lock {
    //谁成功申请了锁，就应该纪录锁被谁持有
    struct task_struct* holder;   // 锁的持有者
    struct semaphore semaphore;    // 用二元量实现锁
    uint32_t holder_repeat_nr;    // 锁持有者重复申请锁的次数
};

void sema_init(struct semaphore* psema, uint8_t value);
void lock_init(struct lock* plock);
void sema_down(struct semaphore* psema);
void sema_up(struct semaphore* psema);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);
#endif
