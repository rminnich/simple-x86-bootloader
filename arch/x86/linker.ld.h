OUTPUT_FORMAT("elf32-i386")
/*OUTPUT_ARCH(i386)*/
ENTRY (_start)
STARTUP ("arch/x86/bootstrap.o")

#include "arch/x86/config_mem.h"

MEMORY {
    /* 40 sectors are allocated to the second stage bootloader.
     * */
    disk_mem(r) : ORIGIN = 0x0, LENGTH = 512 * BOOT1_NB_SECTORS

    /* The code and the data (.data, .rodata, .bss, heap and stack) are
     * located in two different memory area with a size of 64KB each one.
     **/
    segment_code(rx) : ORIGIN = CODE_BASE_OFFSET, LENGTH = SEGMENT_CODE_LENGTH
    segment_data(rwx) : ORIGIN = DATA_BASE_OFFSET, LENGTH = SEGMENT_DATA_LENGTH
}

REGION_ALIAS("REGION_TEXT", segment_code);
REGION_ALIAS("REGION_RODATA", segment_data);
REGION_ALIAS("REGION_DATA", segment_data);
REGION_ALIAS("REGION_BSS", segment_data);

SECTIONS{

    /DISCARD/ : {
        *(.note*)
        *(.comment*)
    }

    /*
     * CODE SEGMENT (64KB - 0x1000)
     **/

    .text ORIGIN(REGION_TEXT) : {
        boot1_text_load_start = . ;

        *(.text*)

        boot1_text_load_end = . ;
    } > REGION_TEXT


    /*
     * DATA SEGMENT (64KB)
     **/

    /* address used for the relocation of the following data sections */
    data_sections_beginning = . ;

    .rodata ORIGIN(segment_data) : AT(ADDR(.text) + SIZEOF(.text)) {
        boot1_rodata_load_start = . ;

        /* If the first rodata is referenced by a pointer, its address
         * is 0x0000. This address is used also to refer to the NULL
         * pointer. Thus to avoid a colision between the first rodata
         * and the NULL pointer, a 1-byte padding is added here.
         **/
        BYTE(0x0);

        *(.rodata*)

        boot1_rodata_load_end = . ;
    } > REGION_RODATA

    .data boot1_rodata_load_end : {
        boot1_data_load_start = . ;

        *(.data*)

        boot1_data_load_end = . ;
    } > REGION_DATA

    .bss boot1_data_load_end : {
        boot1_bss_load_start = .;

        *(.bss*)

        boot1_bss_load_end = .;
    } > REGION_BSS

    /DISCARD/ : { *(.eh_frame) }

    /* global size of the previous data sections */
    data_sections_size = ADDR(.bss) + SIZEOF(.bss) - ADDR(.rodata);

    /* beginning of the heap region */
    heap_addr_begin = .;

    /* check if the binary fits into the 17 sectors foreseen for the bootloader */
    BIN_SIZE_AFTER_OBJCPY = SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data) + SIZEOF(.bss);
    ASSERT( BIN_SIZE_AFTER_OBJCPY <= LENGTH(disk_mem), "boot1's code is too large, optimize it" )

}
