/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)
 
KERNEL_VIRT_BASE = 0xC0000000;

INPUT
(
    src/boot/boot.o 
    src/kernel/kernel.o
    src/drivers/low_level.o
    src/lib/int.o
    src/lib/screen.o
    src/lib/string.o
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

    
	/* Main text section where all the kernel code is loaded */
    TEXT_SECTION_START = .;  
	.text ALIGN(4K) : AT(ADDR(.text) )
    {
        *(.text)
    }
    TEXT_SECTION_END = .;


    /* Section for read only data */
    RODATA_SECTION_START = .;
	.rodata ALIGN(4K) : AT(ADDR(.rodata) )
    {
        *(.rodata)
    }
    RODATA_SECTION_END = .;


    /* Section for extra data */
    DATA_SECTION_START = .;
	.data ALIGN(4K) : AT(ADDR(.data) )
    {
        *(.data)
    }
    DATA_SECTION_END = .;

    /* Section for statically allocated variables */
    BSS_SECTION_START = .;
	.bss ALIGN(4K) : AT(ADDR(.bss) )
    {
        *(.bss)
    }
    BSS_SECTION_END = .;

    _KERNEL_END_ = .;
}