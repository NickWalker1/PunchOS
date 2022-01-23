C_SOURCES = $(wildcard src/kernel/*.c src/drivers/*.c src/lib/*.c)
HEADERS   = $(wildcard src/kernel/*.h src/drivers/*.h src/lib/*.h)

ASM_SOURCES = $(wildcard src/boot/*.asm)

ASM_OBJ =${ASM_SOURCES:.asm=.o}
OBJ=${C_SOURCES:.c=.o}

all: os.iso

GCC=gcc
LD =ld

#Local
QEMU=qemu-system-i386
GRUB=grub-mkrescue


#DCS
#QEMU=/usr/libexec/qemu-kvm
#GRUB=/usr/bin/grub2-mkrescue




GCC_FLAGS= -m32 -std=gnu99 -ffreestanding -O2 -nostdlib -Wall -Wextra 
LD_FLAGS = -melf_i386


#Uncomment run when running not at DCS
run: all
	$(QEMU) -cdrom os.iso

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
