/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)
 
KERNEL_VIRT_BASE = 0xC0000000;

INPUT
(
    src/boot/boot.o 
    src/kernel/kernel.o
)
OUTPUT
(
    os.bin
)
/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;
 
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.multiboot.data :
	{
		*(.multiboot.data)
	}
	
	.multiboot.text :
	{
		*(.multiboot.text)
	}

    
	
    . += KERNEL_VIRT_BASE;

	
    TEXT_SECTION_START = .;  
	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRT_BASE)
    {
        *(.text)
    }
    TEXT_SECTION_END = .;


    RODATA_SECTION_START = .;
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRT_BASE)
    {
        *(.rodata)
    }
    RODATA_SECTION_END = .;


    DATA_SECTION_START = .;
	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRT_BASE)
    {
        *(.data)
    }
    DATA_SECTION_END = .;

    BSS_SECTION_START = .;
	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRT_BASE)
    {
        *(.bss)
    }
    BSS_SECTION_END = .;
}