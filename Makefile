start: all kernel.bak
	mv kernel.bak kernel.asm 
	nasm -o kernel.bat kernel.asm
kernel.bak: 
	mv kernel.asm kernel.bak
all : $(subst .c,.func,$(wildcard *.c))

%.func : %.asm
	~/extractFunc.py $<

%.asm : %.o 
	objconv -fnasm $<
%.o : %.c
	cp kernel.asm kernel.bak
	gcc -fno-pic -fno-asynchronous-unwind-tables -m32  -s -c -o $@ $<

clean :
	cp kernel.asm kernel.bak
	rm -rf *.asm *.o *.func *.bat
	mv kernel.bak kernel.asm 
