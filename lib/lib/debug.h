#ifndef  __KERNEL_DEBUG_H
#define  __KERNEL_DEBUG_H
void panic_spin(char *filename,int line,const char *func,const char *condidtion);
//                              被编译的文件名，被编译文件中的行号，被编译的函数名
#define  PANIC(...) panic_spin(__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef NDEBUG 
    #define  ASSERT(CONDITION) ((void)0) // 让此assert成为空0,也就是什么都不是，这样就相当于删除了assert
#else 
#define ASSERT(CONDITION) if (CONDITION) {} else {PANIC(#CONDITION);}
#endif // __NDBUG
#endif
