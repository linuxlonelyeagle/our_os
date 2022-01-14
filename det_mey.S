%include "./include/boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

;构建gdt及其内部的描述符
GDT_BASE:dd 0x00000000
         dd 0x00000000
CODE_DESC: dd 0x0000FFFF
           dd DESC_CODE_HIGH4


DATA_STACK_DESC:dd 0x0000FFFF      ;这个是栈段和数据段的描述符
                dd DESC_DATA_HIGH4

VIDEO_DESC:dd 0x80000007  ;(0xbffff-0xb8000)/4k=0x07  (0x07+1)*0x1000-1=实际的空间
           dd DESC_VIDEO_HIGH4

GDT_SIZE equ $-GDT_BASE
GDT_LIMIT equ GDT_SIZE-1
times 60 dq 0  ; 预留60个描述符空位，8个字节
; total_mem_bytes 地址是0xb00
total_mem_bytes dd 0   ; 用于保存内存容量(4+60)*8=512=0x200  

gdt_ptr dw GDT_LIMIT   ;定义gdt的指针，前2个字节是gdt界限，后4字节是gdt起始地址
        dd GDT_BASE          

;人工对齐
ards_buf times 244 db 0          
ards_nr dw 0     ; 用于记录ARDS结构体数量

loader_start:  ; 文件中的偏移地址为0x300

;int 15h eax = 0000E820h  ,edx = 534D4150h   ('SMAP') 获取内存布局

    xor ebx,ebx
    mov edx,534D4150h
    mov di,ards_buf      ; ards缓冲区

.e820_mem_get_loop:
    mov eax,0000E820h   ; 因为每次执行int 15h之后都会让eax = 534D4150h
    mov ecx,20          ;ards地址范围描述符结构大小是20字节
    int 0x15
    jc .e820_failed_so_try_e801  ;如果cf = 1就跳转
    add di,cx
    inc word [ards_nr]
    cmp ebx,0
    jnz .e820_mem_get_loop
              
    mov cx,[ards_nr]
    mov ebx,ards_buf
    xor edx,edx

.find_max_mem_area:
    mov eax,[ebx]     ;BaseAddrLow
    add eax,[ebx+8]   ;LengthLow
    add ebx,20
    cmp edx,eax        

;冒泡排序，找出最大,edx寄存器始终是最大的内存容量
    jge .next_ards  ; 大于或者等于就跳过
    mov edx,eax
.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok

;------ int 15h ax = E801h获取内存大小，最大获取4g
.e820_failed_so_try_e801:
    mov ax,0xe801
    int 0x15
    jc .e801_failed_so_try88

    mov cx,0x400   ; ax和cx中是以kb为单位的内存数量，将其转换为以bytes为单位
    mul cx         ; 高位在dx中，低位在ax中 
    shl edx,16     
    and eax,0x0000FFFF
    or edx,eax
    add edx,0x100000         ; 获得的内存世纪上比实际大小少1mb
    mov esi,edx

    xor eax,eax
    mov ax,bx
    mov ecx,0x10000
    mul ecx

    add esi,eax
    mov edx,esi
    jmp .mem_get_ok

;--------- int 15h ah = 0x88 获取内存大小,只能获取64mb之内
.e801_failed_so_try88:
    mov ah,0x88
    int 0x15
    jc .error_hlt
    and eax,0x0000FFFF
    mov cx,0x400
    mul cx
    shl edx,16
    or edx,eax
    add edx,0x100000

.mem_get_ok:
    mov [total_mem_bytes],edx

.error_hlt:
    hlt
    