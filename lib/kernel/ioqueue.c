#include "./../device/ioqueue.h"
#include "./../lib/interrupt.h"
#include "./../lib/global.h"
#include "./../lib/debug.h"

#define  bool int
#define  NULL 0

// 初始化io队列
void ioqueue_init(struct ioqueue* ioq) {
    lock_init(&ioq->lock);
    ioq->producer = ioq->consumer = NULL;
    ioq->head = ioq->tail = 0;
}

// 返回pos在缓冲区中的下一个位置
static int32_t next_pos(int32_t pos) {
    return (pos+1) % bufsize;
}


// 判断队列是否已满
bool ioq_full(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    return next_pos(ioq->head)== ioq->tail;
}

//判断队列是否以空
 bool ioq_empty(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    return ioq->head == ioq->tail;
}
// 页就是说*wait是null就代表没有休眠，如果不是null就表示在休眠
// 使当前生产者或者消费者在此缓冲区上等待
 void ioq_wait(struct task_struct** waiter) {
    ASSERT(*waiter == NULL && waiter!=NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}
// 唤醒waiter
static void wakeup(struct task_struct** waiter) {
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter);
    *waiter = NULL;
}

// 消费者从ioq队列中获取一个字符,这个函数是专门给消费者调用的
char ioq_getchar(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    // 如果为空就等待，如果不为空就可以消费
    // 因为可能有多个消费者，所以这里要用while循环
    while (ioq_empty(ioq)) {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->consumer);
        lock_release(&ioq->lock);
    }
    char byte = ioq->buf[ioq->tail];
    ioq->tail = next_pos(ioq->tail);   // 因为腾出了一个空间
    if (ioq->producer != NULL) {    //如果生产者在休眠
        wakeup(&ioq->producer);   // 唤醒生产者
    }
    return byte;
}

// 生存者往ioq队列中写入一个字符byte
void ioq_putchar(struct ioqueue* ioq, char byte) {
    ASSERT(intr_get_status() == INTR_OFF);
    while (ioq_full(ioq)) {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->producer);
        lock_release(&ioq->lock);
    }
    ioq->buf[ioq->head] = byte;
    ioq->head = next_pos(ioq->head);   // 判断消费者是不是在休眠
    if (ioq->consumer != NULL) {
        wakeup(&ioq->consumer);
    }
}

// 返回幻形缓冲区中的数据长度
uint32_t ioq_length(struct ioqueue* ioq) {
    uint32_t len = 0;
    if (ioq->head >= ioq->tail) {
        len = ioq->head - ioq->tail;
    } else {
        len = bufsize - (ioq->tail - ioq->head);
    }
    return len;
}


