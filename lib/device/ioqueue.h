#ifndef  __DEVICE_IOQUEUE_H
#define  __DEVICE_IOQUEUE_H
#include "./../lib/stdint.h"
#include "./../lib/thread.h"
#include "./../lib/sync.h"
#define  bool int

#define  bufsize 64
// 环形队列
struct ioqueue {
    struct lock lock;   // 本缓冲区的锁，每次对缓存区操作时都要申请这个锁，从而保证缓存区操作互斥
    struct task_struct* producer;
    struct task_struct* consumer;
    char buf[bufsize];
    int32_t head;
    int32_t tail;
};

void ioqueue_init(struct ioqueue* ioq);
bool ioq_full(struct ioqueue* ioq);
 bool ioq_empty(struct ioqueue* ioq);
char ioq_getchar(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq, char byte);
uint32_t ioq_length(struct ioqueue* ioq);
#endif
