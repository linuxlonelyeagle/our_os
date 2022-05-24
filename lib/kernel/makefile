ENTRY_POINT = -Ttext 0xc0001500
AS = nasm -f elf32
CC = gcc -m32 
LD = ld  -m elf_i386
BOCHS = ../..
OBJECTFILE = main.o print.o init.o interrupt.o kernel.o timer.o debug.o  bitmap.o memory.o string.o
# 编译
main.o:main.c print.h  ./../lib/init.h ./../lib/debug.h
	$(CC) -c $< -o $@ 
print.o:Print.S 
	$(AS)  $< -o  $@ 

init.o:init.c  $(LIB)/init.h ./../lib/interrupt.h ./../lib/timer.h  print.h ./../lib/memory.h
	$(CC) -c $< -o $@ 

interrupt.o:interrupt.c  ./../lib/interrupt.h ./../lib/stdint.h ./../lib/global.h ./../lib/io.h print.h
	$(CC) -c $< -o $@ 

kernel.o:kernel.S
	$(AS)  $< -o  $@ 
timer.o:print.h ./../lib/io.h ./../lib/timer.h
	$(CC) -c $< -o $@

debug.o:debug.c print.h ./../lib/interrupt.h
	$(CC) -c $< -o $@
memory.o:memory.c ./../lib/stdint.h print.h  ./../lib/memory.h
	$(CC) -c $< -o $@
bitmap.o:bitmap.c ./../lib/bitmap.h  ./../lib/stdint.h   ./../lib/string.h ./../lib/interrupt.h ./../lib/debug.h print.h
	$(CC) -c $< -o $@
string.o:string.c ./../lib/string.h ./../lib/global.h ./../lib/debug.h
	$(CC) -c $< -o $@
.PHONY:all
all:
	$(LD) $(ENTRY_POINT) -e main -o $(BOCHS)/kernel.bin  $(OBJECTFILE) 
	
