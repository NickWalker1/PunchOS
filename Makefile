C_SOURCES = $(wildcard src/kernel/*.c src/drivers/*.c src/lib/*.c src/gdt/*.c src/interrupt/*.c src/memory/*.c src/sync/*.c src/processes/*.c src/shell/functions/*.c src/shell/*.c src/threads/*.c src/tests/*.c)
HEADERS   = $(wildcard src/kernel/*.h src/drivers/*.h src/lib/*.h src/gdt/*.h src/interrupt/*.h src/memory/*.h src/sync/*.h src/processes/*.h src/shell/functions/*.h src/shell/*.h src/threads/*.h src/tests/*.h )

ASM_SOURCES = $(wildcard src/boot/*.asm src/gdt/*.asm src/interrupt/*.asm src/memory/*.asm src/processes/*.asm src/threads/*.asm)

ASM_OBJ =${ASM_SOURCES:.asm=.o}
OBJ=${C_SOURCES:.c=.o}


GCC_FLAGS= -m32 -std=gnu99 -ffreestanding -O0 -nostdlib -Wall -Wextra 
LD_FLAGS = -melf_i386

all: os.iso

GCC=gcc
LD =ld

#Local
QEMU=qemu-system-i386
GRUB=grub-mkrescue


#DCS
#QEMU=/local/java/qemu-i386-softmmu/bin/qemu-system-386
#GRUB=/usr/bin/grub2-mkrescue



run: all
	$(QEMU) -cdrom os.iso -D ./log.txt -d cpu_reset


os.iso: os.bin
	cp $< isodir/boot/os.bin
	$(GRUB) -o $@ isodir

os.bin: $(OBJ) $(ASM_OBJ)
	$(LD) -T link.ld $(LD_FLAGS)

%.o: %.asm
	nasm -felf32 $< -o $@

%.o : %.c ${HEADERS} 
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

clean:
	rm -f *.iso *.bin 
	rm -f src/*/*.o
	rm -f isodir/boot/*.bin
