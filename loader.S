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
SELECTOR_CODE equ (0x0001<<3) +TI_GDT +RPL0
SELECTOR_DATA equ (0x0002<<3) + TI_GDT +RPL0
SELECTOR_VIDEO equ (0x0003<<3) +TI_GDT+RPL0

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
.error_hlt:
    hlt


.mem_get_ok:
    mov [total_mem_bytes],edx
    mov sp,LOADER_STACK_TOP
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
    ;读取内核
    mov eax,KERNEL_START_SECTOR
    mov ebx,KERNEL_BIN_BASE_ADDR
    mov ecx,200
    call rd_disk_m_32
    ;建立页表
    call set_up_page
    sgdt [gdt_ptr]
    mov ebx,[gdt_ptr + 2]
    or dword [ebx + 0x18 +4],0xc0000000
    add dword [gdt_ptr + 2],0xc0000000
    add esp,0xc0000000
    mov eax,PAGE_DIR_TABLE_POS
    mov cr3,eax
    mov eax,cr0
    or eax,0x80000000
    mov cr0,eax
    lgdt [gdt_ptr]
    jmp SELECTOR_CODE:enter_kernel
enter_kernel:
    call kernel_init
    mov esp,0xc009f000
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    mov byte [gs:160],'P'
    mov byte [gs:161],0xa4
    mov byte [gs:162],'H'
    mov byte [gs:163],0xa5
    jmp SELECTOR_CODE: KERNEL_ENTRY_POINT

;-----------创建页目录及页表
set_up_page:
    mov ecx,4096
    mov esi,0
.clear_page_dir:    
    mov byte [PAGE_DIR_TABLE_POS + esi],0
    inc esi
    loop .clear_page_dir
.create_pde:     
    mov eax,PAGE_DIR_TABLE_POS
    add eax,0x1000
    mov ebx,eax
    or eax,PG_US_U | PG_RW_W | PG_P
    mov [PAGE_DIR_TABLE_POS+0x0],eax
    mov [PAGE_DIR_TABLE_POS+0xc00],eax
    sub eax,0x1000
    mov [PAGE_DIR_TABLE_POS + 4092],eax   ; 使最后一个目录项指向页目录表自己的地址

    mov ecx,256   
    mov esi,0
    mov edx,PG_US_S | PG_RW_W | PG_P

.create_pte:
    mov [ebx+esi*4],edx
    add edx,4096
    inc esi
    loop .create_pte

    mov eax,PAGE_DIR_TABLE_POS
    add eax,0x2000
    or eax,PG_US_U | PG_RW_W |PG_P
    mov ebx,PAGE_DIR_TABLE_POS
    mov ecx,254
    mov esi,769
.create_kernel_pde:
    mov [ebx+esi*4],eax
    inc esi
    add eax,0x1000
    loop .create_kernel_pde
    ret

;------------------------------
;功能，读取硬盘n个扇区
rd_disk_m_32:
;------------------------------
;eax = lba扇区
;ebx = 将数据写入的内存地址
;ecx = 读入的扇区数
    mov esi,eax   ; 备份eax
    mov di,cx     ;备份cx

;1.设置读取的扇区数量
    mov dx,0x1f2
    mov al,cl
    out dx,al

    mov eax,esi   ; 恢复ax

;2.将lba地址存入0x1f3~0x1f6
    mov dx,0x1f3
    out dx,al

    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al

    shr eax,cl
    mov dx,0x1f5
    out dx,al

    shr eax,cl
    mov dx,0x1f5
    out dx,al

    shr eax,cl
    and al,0x0f
    or al,0xe0
    mov dx,0x1f6
    out dx,al

;3.向0x1f7端口写入命令,0x20
    mov dx,0x1f7
    mov al,0x20
    out dx,al
;4检测硬件状态
.not_ready:    ; 由于status是0x1f7所以这里dx不需要重新赋值
    nop         ; nop是空操作，什么也不做
    in al,dx     
    and al,0x88   ; 保留第4位置和第7位置
    cmp al,0x08    ;测试第4位置是否已经准备好了
    jnz .not_ready ; 没有准备好，继续等待

;第5布:从0x1f0端口数据
    mov ax,di    ; di中保存了需要读取的扇区的数量
    mov dx,256    
    mul dx
    mov ecx,eax    
    mov dx,0x1f0     
.go_on_read:
    in ax,dx
    mov [ebx],ax
    add ebx,2
    loop .go_on_read
    ret

kernel_init:
    xor eax,eax 
    xor ebx,ebx   ; ebx记录程序头表的地址
    xor ecx,ecx   ; cx记录程序头表的program header数量
    xor edx,edx   ; dx记录proagrm header尺寸，即e_phentsze
    mov dx, [KERNEL_BIN_BASE_ADDR + 42]   ; 一个seg的大小
    mov ebx,[KERNEL_BIN_BASE_ADDR+28]     
    add ebx,KERNEL_BIN_BASE_ADDR          ;这样就获得了程序头表的物理地址
    mov cx,[KERNEL_BIN_BASE_ADDR+44]      ; 获得程序头的数量
.each_segment:
    cmp byte [ebx+0],PT_NULL
    je .PTNULL
    push dword [ebx+16]
    mov eax,[ebx+4]
    add eax,KERNEL_BIN_BASE_ADDR
    push eax
    push dword [ebx+8]
    call mem_cpy
    add esp,12
.PTNULL:
    add ebx,edx
    loop .each_segment
    ret


mem_cpy:
    cld 
    push ebp
mov ebp,esp
    push ecx
    mov edi,[ebp+8]
    mov esi,[ebp+12]
    mov ecx,[ebp+16]
    rep movsb
    pop ecx
    pop ebp
    ret
