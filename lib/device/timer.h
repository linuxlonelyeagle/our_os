#ifndef  __DEVICE__TIME_H
#define  __DEVICE__TIME_H
#include "../lib/stdint.h"
static void intr_timer_handler(void);
void mtime_sleep(uint32_t m_seconds);
void timer_init(void);
#endif
