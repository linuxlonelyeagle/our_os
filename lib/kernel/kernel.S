[bits 32]
%define ERROR_CODE nop   ; nop什么也不做，就是表示占位，因为这个宏必须要传入两个参数
;因为有的中断会自动压入错误码
%define ZERO push 0
extern idt_table   ; idt_table 是c中注册的中断处理程序
extern put_str ; 声明外部函数

section .data
intr_str db "interrupt occur!",0xa,0
global intr_entry_table
intr_entry_table:

%macro VECTOR 2
section .text 
intr%1entry:     
    %2
    push ds   ; 因为我们的中断处理程序是在c语言中运行，所以必须要保护上下文环境
    push es
    push fs
    push gs
    pushad
    ; 如果是从片上进入的中断，除了往从片上发送eoi，还要王主片上发送
    mov al,0x20              ;中断结束命令,ocw2操作，因为我们设置了手动结束，所以我们要在程序中手动发送eoi         
    out 0xa0,al
    out 0x20,al
    push %1
    call [idt_table + %1*4]                    
    jmp intr_exit

section .data
    dd intr%1entry   ;用于记录每个终端程序的地址       
%endmacro 

section .text
global intr_exit 
intr_exit:
add esp,4
popad 
pop gs
pop fs
pop es
pop ds
add esp,4   ; 跳过erro_code
iretd

VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
VECTOR 0x20,ZERO   ; 时钟中断
VECTOR 0x21,ZERO   ; 键盘中断
VECTOR 0x22,ZERO
VECTOR 0x23,ZERO
VECTOR 0x24,ZERO
VECTOR 0x25,ZERO
VECTOR 0x26,ZERO
VECTOR 0x27,ZERO 
VECTOR 0x28,ZERO
VECTOR 0x29,ZERO
VECTOR 0x2a,ZERO
VECTOR 0x2b,ZERO
VECTOR 0x2c,ZERO
VECTOR 0x2d,ZERO
VECTOR 0x2e,ZERO
VECTOR 0x2f,ZERO


[bits 32] 
extern syscall_table
section .text
global syscall_handler
syscall_handler:
; 保存上下文
push 0
push ds
push es
push fs
push gs
pushad
push 0x80
push edx   ; 系统调用第3个参数
push ecx   ; 第二个参数
push ebx    ; 第三个参数

call [syscall_table + eax*4]   ;根据二进制编程接口 abi 约定，寄存器 eax 用来存储返回值。经过上面第 122 行的 call 函数调用，如
;果有返回值的话，eax 的值已经变成了返回值（如果没有返回值也没关系，编译器会保证函数返回后 eax
;的值不变），
add esp,12  ; 跨过上面的三个参数
mov [esp+8*4],eax
jmp intr_exit   ; 恢复上下文

