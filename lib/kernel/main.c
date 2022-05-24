#include "print.h"
#include "./../lib/init.h"
#include "./../lib/debug.h"
#include "./../lib/memory.h"
#include "./../lib/thread.h"
#include "./../lib/interrupt.h"
#include "./../device/console.h"
#include "./../device/keyboard.h"
#include "./../device/ioqueue.h"
#include "./../lib/process.h"
#include "./../lib/syscall-init.h"
#include "./../lib/syscall.h"
#include "./../lib/stdio.h"
#include "./../fs/fs.h"
#include "./../fs/dir.h"
#include "./../lib/g.h"
#include "./../lib/fork.h"
#include "./../lib/shell.h"
#define  NULL 0

void init(void);
void pro_a(void);
void pro_b(void);


/*int main(void) {
//   put_str("I am kernel\n");
   init_all();
   cls_screen();
   //  process_execute(pro_a,"a");
   //  process_execute(pro_b,"b");
   //process_execute(init,"init");
  // put_str("[sen@localhost /]$ ");
   intr_enable();
   while(1);
   return 0;
}*/

int main(void) {
   put_str("I am kernel\n");
   init_all();
/********  测试代码  ********/
/********  测试代码  ********/
   process_execute(init,"init");
   my_shell();
   while(1);
   return 0;
}

/* init进程 */
void init(void) {
   uint32_t ret_pid = fork();
 //   ps();
   /*if(10) {
      printf("i am father, my pid is %d, child pid is %d\n", getpid(), ret_pid);
   } else {
      printf("i am child, my pid is %d, ret pid is %d\n", getpid(), ret_pid);
   }*/
   while(1);
}



void pro_a(void) {
    while (1) {
    }

}

void pro_b(void ) {
    while (1) {
        console_put_str("1111111111\n");
    }
}

/* init进程 */
/*void init(void) {
    my_shell();
    while(1);
}*/

