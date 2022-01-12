%include "include/boot.inc"
SECTION MBR vstart=0x7c00
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax,0xb800
    mov gs,ax
;清屏

    mov ax,0600h
    mov bx,0700h
    mov cx,0
    mov dx,184fh
    int 10h
    
    mov byte [gs:0x00],'1'
    mov byte [gs:0x01],0xa4
    mov byte [gs:0x02],' '
    mov byte [gs:0x03],0xa4
    mov byte [gs:0x04],'M'
    mov byte [gs:0x05],0xa4
    mov byte [gs:0x06],'B'
    mov byte [gs:0x07],0xa4
    mov byte [gs:0x08],'R'
    mov byte [gs:0x09],0xa4

    mov eax,LOADER_START_SECTOR   ; 起始扇区lba地址
    mov bx,LOADER_BASE_ADDR         ; 写入的地址
    mov cx,4                          ;等待写入的扇区数量
    call rd_disk_m_16
    jmp LOADER_BASE_ADDR


;------------------------------
;功能，读取硬盘n个扇区
rd_disk_m_16:
;------------------------------
;eax = lba扇区
;bx = 将数据写入的内存地址
;cx = 读入的扇区数
    mov esi,eax   ; 备份eax
    mov di,cx           ;备份cx

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
    mov cx,ax    
    mov dx,0x1f0     
.go_on_read:
    in ax,dx
    mov [bx],ax
    add bx,2
    loop .go_on_read
    ret

    times 510-($-$$) db 0
    db 0x55,0xaa
