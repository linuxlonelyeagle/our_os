%include "./include/boot.inc"
SECTION loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

;构造gdt及其内部的描述符
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
times 60 dq 0  ; 预留60个描述符空位
SELECTOR_CODE equ (0x0001<<3) +TI_GDT +RPL0
SELECTOR_DATA equ (0x0002<<3) + TI_GDT +RPL0
SELECTOR_VIDEO equ (0x0003<<3) +TI_GDT+RPL0

;以下是gdt的指针，前2字节是gdt界限，后4字节是gdt起始地址

gdt_ptr  dw GDT_LIMIT
         dd GDT_BASE
loadermsg db '2 loader in read.'

loader_start:

    mov sp,LOADER_STACK_TOP
    mov bp,loadermsg
    mov cx,17
    mov ax,0x1301
    mov bx,0x001f
    mov dx,0x1800
    int 0x10
;打开a20
    in al,0x92
    or al,0000_0010b
    out 0x92,al
;加载gdt
    lgdt [gdt_ptr]
;cr0第0位置1
    mov eax,cr0
    or eax,0x00000001
    mov cr0,eax
    jmp dword SELECTOR_CODE:p_mode_start

[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    mov byte [gs:160],'P'
    mov byte [gs:161],0xa4
    mov byte [gs:162],'H'
    mov byte [gs:163],0xa5
    jmp $
