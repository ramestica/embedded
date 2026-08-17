/*
 * Sources:
 *
 * http://hertaville.com/2012/06/29/a-sample-linker-script/
 * Energia
 * Miro Samek
 * http://robotics.mcmanis.com/src/arm-blink/linker-script.html
 * http://www.meatandnetworking.com/tutorials/arm-cortex-mx-quickstart/
 */

ENTRY(ResetISR)

MEMORY
{
    /*
     * FLASH memory
     */       
    ROM (rx) : ORIGIN = 0x0, LENGTH = 256K

    /*
     * SRAM memory
     */
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

PROVIDE (_rom_origin = ORIGIN(ROM));

PROVIDE (_ram_origin = ORIGIN(RAM));

#ifdef LOAD_TO_ROM
REGION_ALIAS("REGION_TEXT", ROM);
#else
REGION_ALIAS("REGION_TEXT", RAM);
#endif

SECTIONS
{
    .text :
    {
        . = ALIGN(4);

        _text = .;

        KEEP(*(.isr_vector))

        . = ALIGN(4);
        
        *(.text .text* .gnu.linkonce.t.*)

        *(.glue_7t) *(.glue_7)

        *(.rodata .rodata* .gnu.linkonce.r.*)

        *(.ARM.extab* .gnu.linkonce.armextab.*)

        . = ALIGN(4);

        KEEP(*(.init))

        . = ALIGN(4);

        __preinit_array_start = .;

        KEEP (*(SORT(.preinit_array*)))

        KEEP (*(.preinit_array))

        __preinit_array_end = .;

        . = ALIGN(4);

        __init_array_start = .;

        KEEP (*(SORT(.init_array.*)))

        KEEP (*(.init_array))

        __init_array_end = .;
        
        /* 
         * FIXME: how useful are global c++ destructors? The application is
         * just going away. Would them be important if they are shutting down
         * something critical?
         */
       
        . = ALIGN(4);

        KEEP(*(.fini))

        . = ALIGN(4);

        __fini_array_start = .;

        KEEP (*(.fini_array))

        KEEP (*(SORT(.fini_array.*)))

        __fini_array_end = .;

        . = ALIGN(4);

        __exidx_start = .;

        *(.ARM.exidx* .gnu.linkonce.armexidx.*)

        __exidx_end = .;

        . = ALIGN(4);

        _etext = .;

    } > REGION_TEXT

    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        . = ALIGN(4);

        _data = .;

        *(vtable)

        *(.data .data* .gnu.linkonce.d.*)

        _edata = .;

    } > RAM

    .bss :
    {
        . = ALIGN(4);

        _bss = .; 

        *(.bss .bss*) 

        *(COMMON)

        . = ALIGN(4); 

        _ebss = .; 

        . = ALIGN(8);

    } > RAM

    . = ALIGN(4);

    _end = . ;

}

/* end of allocated ram is start of heap, heap grows up towards stack */
PROVIDE(end = _end);

/* top of stack starts at end of ram, stack grows down towards heap */
PROVIDE (_estack = ORIGIN(RAM) + LENGTH(RAM) - 4);

/*___oOo___*/
