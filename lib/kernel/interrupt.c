#include "../lib/interrupt.h"
#include "../lib/stdint.h"
#include "../lib/g.h"
#include "../lib/io.h"
#include "print.h"

#define  PIC_M_CTRL 0x20    //主片的控制端口 
#define  PIC_M_DATA 0x21    //主片的数据端口
#define  PIC_S_CTRL 0xa0    //从片的控制端口
#define  PIC_S_DATA 0xa1    //从片的数据端口
#define  IDT_DESC_CNT 0x81  // 目前一共支持的中断数量

// 用来获取中断的状态
#define  EFLAGS_IF 0x00000200   // eflags 寄存器中的if位为1
// "=g" 用内存或者寄存器来作约束,最后的eflags_var会获得eflags的值
#define  GET_EFLAGS(EFLAGS_VAR) asm volatile ("pushfl; popl %0":"=g"(EFLAGS_VAR))

extern  uint32_t syscall_handler(void);

//中断描述符结构体
extern void put_str(char *str);
struct gate_desc {
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t dcount; //双字记数字段，是门描述符中的第4字节
    uint8_t attribute;
    uint16_t func_offset_high_word;
};

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];

char* intr_name[IDT_DESC_CNT];  //用来记录每一项异常的名称
intr_handler idt_table[IDT_DESC_CNT];  //
extern intr_handler  intr_entry_table[IDT_DESC_CNT];  // 声明引用定义在kernel.S，中断处理函数入口

//无论是主片还是从片，都必须按顺序依次写入icw1,icw2,icw3,icw4
static void pic_init(void) {
    // 初始化主片
    outb(PIC_M_CTRL,0x11);           
    outb(PIC_M_DATA,0x20);         //写入的icw的值为0x20,即32,说明主片的起始中断向量号为0x20

    outb(PIC_M_DATA,0x04);         // 0x04 00000100  用ir2引角作为主片级联从片的接口
    outb(PIC_M_DATA,0x01);       
    //初始化从片
    outb(PIC_S_CTRL,0x11);   
    outb(PIC_S_DATA,0x28);        
    outb(PIC_S_DATA,0x02);
    outb(PIC_S_DATA,0x01);      

    
    //打开主片上的IR0,IR1,也就是说目前只接受时钟中断,键盘中断
    outb(PIC_M_DATA,0xf8);  // 如果是法上在从片上的中断，必须主片和从片都要发送eoi，下次
    outb(PIC_S_DATA,0xbf);  // 中断才能生效
    put_str("pic_init done\n");
}

//创建中断门描述符
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function){
    p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;
    p_gdesc->dcount = 0;
    p_gdesc->selector = SELECTOR_K_CODE;  
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}

// 初始化中断描述符表

static void idt_desc_init(void) {
    int i;
    int lastindex = IDT_DESC_CNT - 1;
    for (i = 0; i < IDT_DESC_CNT ;i++) {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    // syscall_handler 就是系统调用中断的历程
    make_idt_desc(&idt[lastindex], IDT_DESC_ATTR_DPL3, syscall_handler);
    put_str("idt_desc_init done\n");
}


static void general_intr_handler(uint8_t vec_nr) {
    if (vec_nr == 0x27 || vec_nr == 0x2f) {   // 因为这两个无法通过芯片屏蔽，所以在软件层面直接略过
        // IRQ7和IRQ15会产生伪中断(spurious interrupt),不需要处理
        // 0x2f是从片8259a上最后一个irq引脚
        return;
    }
    set_cursor(0);
    int cursor_pos = 0;
    while (cursor_pos < 320) {
        put_char(' ');
        cursor_pos++;
    }
    set_cursor(0);
    put_str("excetion message begin\n");
    set_cursor(88);
    // 从第二行第八个字符开始打印
    put_str(intr_name[vec_nr]);
    if (vec_nr == 14) {  // 如果是页缺失，将缺失的地址打印出来
        int page_fault_vaddr = 0;
        asm ("movl %%cr2,%0":"=r"(page_fault_vaddr));// cr2 是存放page_fault的地址
        put_str("\npage fault addr is ");
        put_int(page_fault_vaddr);
    }
    // 当虚拟地址尚未在页表中分配物理页，这样会导致pagefault异常，导致
    // pagefault的虚拟地址会被放在控制寄存器cr2中
    put_str("\nexcetion message end\n");
    // 能进入中断，说明中断已经关闭了
    while (1);
}

//完成一般中断处理函数注册及异常名词注册
static void exception_init(void) {
    int i;
    for (i = 0; i < IDT_DESC_CNT;i++) {
        idt_table[i] = general_intr_handler;
        intr_name[i] = "unkown";
    }
   intr_name[0] = "#DE Divide Error";
   intr_name[1] = "#DB Debug Exception";
   intr_name[2] = "NMI Interrupt";
   intr_name[3] = "#BP Breakpoint Exception";
   intr_name[4] = "#OF Overflow Exception";
   intr_name[5] = "#BR BOUND Range Exceeded Exception";
   intr_name[6] = "#UD Invalid Opcode Exception";
   intr_name[7] = "#NM Device Not Available Exception";
   intr_name[8] = "#DF Double Fault Exception";
   intr_name[9] = "Coprocessor Segment Overrun";
   intr_name[10] = "#TS Invalid TSS Exception";
   intr_name[11] = "#NP Segment Not Present";
   intr_name[12] = "#SS Stack Fault Exception";
   intr_name[13] = "#GP General Protection Exception";
   intr_name[14] = "#PF Page-Fault Exception";
   // intr_name[15] 第15项是intel保留项，未使用
   intr_name[16] = "#MF x87 FPU Floating-Point Error";
   intr_name[17] = "#AC Alignment Check Exception";
   intr_name[18] = "#MC Machine-Check Exception";
   intr_name[19] = "#XF SIMD Floating-Point Exception";
}

//完成有关中断的所有初始化工作
void idt_init() {
    put_str("idt_init start\n");
    idt_desc_init();  //初始化中断描述符表
    exception_init();
    pic_init();       // 初始化8259a
    //加载 idt
    uint64_t idt_operand = ((sizeof(idt)-1) | ((uint64_t)(uint32_t)idt<<16));
    asm volatile("lidt %0"::"m"(idt_operand));  // %0代表的是idt_operand的地址
    put_str("idt_init done\n");
}
//开中断并返回中断前的状态
enum intr_status intr_enable()
{
    if (INTR_ON == intr_get_status())
    {
        enum intr_status old_status = INTR_ON;
        return old_status;
    } else {
        enum intr_status old_status = INTR_OFF;
        asm volatile("sti");
        return old_status;
    }
}
// 关中断，并且返回中断前的状态
enum intr_status intr_disable()
{
    enum intr_status old_status;
    if (INTR_ON == intr_get_status())
    {
        enum intr_status old_status = INTR_ON;
        asm volatile("cli" ::: "memory");
    } else {
        old_status = INTR_OFF;
        return old_status;
    }
}
// 将中断状态这是为status
enum intr_status intr_set_status(enum intr_status status)
{
    return status & INTR_ON ? intr_enable() : intr_disable();
}

// 获取当前中断状态
enum intr_status intr_get_status()
{
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

void register_handler(uint8_t vecter_no, intr_handler function) {
    idt_table[vecter_no] = function;
}
