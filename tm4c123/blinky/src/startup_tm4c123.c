#include <stdint.h>

extern uint32_t _etext, _sdata, _edata, _sbss, _ebss, _estack;

int main(void);

void Reset_Handler(void);
void Default_Handler(void) { for (;;) {} }

#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))
void NMI_Handler(void)        WEAK_ALIAS;
void HardFault_Handler(void)  WEAK_ALIAS;

__attribute__((section(".isr_vector"), used))
const void *vector_table[] = {
    &_estack,              /* initial stack pointer (top of SRAM) */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    /* remaining vectors default to 0; fine for blinky */
};

void Reset_Handler(void)
{
    uint32_t *src = &_etext;
    uint32_t *dst = &_sdata;

    while (dst < &_edata)          /* copy .data from flash to RAM */
        *dst++ = *src++;
    for (dst = &_sbss; dst < &_ebss; )  /* zero .bss */
        *dst++ = 0;

    main();
    for (;;) {}
}
