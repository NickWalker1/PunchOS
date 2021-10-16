C_SOURCES = $(wildcard src/kernel/*.c)
HEADERS   = $(wildcard src/kernel/*.h)

OBJ=${C_SOURCES:.c=.o}

all: os.iso

GCC=i386-elf-gcc
LD =i386-elf-ld

run: all
	qemu-system-i386 -cdrom os.iso

os.iso: os.bin
	cp $< isodir/boot/os.bin
	grub-mkrescue -o $@ isodir

os.bin: bin/boot.o $(OBJ) 
	$(LD) -T link.ld

bin/boot.o: src/boot/boot.asm
	nasm -felf32 $< -o $@

%.o : %.c ${HEADERS} 
	$(GCC) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -nostdlib -Wall -Wextra


clean:
	rm -f *.iso *.bin 
	rm -f src/*/*.o
	rm -f isodir/boot/*.bin