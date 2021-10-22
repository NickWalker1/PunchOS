C_SOURCES = $(wildcard src/kernel/*.c)
HEADERS   = $(wildcard src/kernel/*.h)

OBJ=${C_SOURCES:.c=.o}

all: os.iso

GCC=gcc
LD =ld
GCC_FLAGS= -m32 -std=gnu99 -ffreestanding -O2 -nostdlib -Wall -Wextra 
LD_FLAGS = -melf_i386

run: all
	qemu-system-i386 -cdrom os.iso

os.iso: os.bin
	cp $< isodir/boot/os.bin
	grub-mkrescue -o $@ isodir

os.bin: bin/boot.o $(OBJ) 
	$(LD) -T link.ld $(LD_FLAGS)

bin/boot.o: src/boot/boot.asm
	nasm -felf32 $< -o $@

%.o : %.c ${HEADERS} 
	$(GCC) -c $< -o $@ $(GCC_FLAGS)


clean:
	rm -f *.iso *.bin 
	rm -f src/*/*.o
	rm -f isodir/boot/*.bin