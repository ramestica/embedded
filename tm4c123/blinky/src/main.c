#include "TM4C123GH6PM.h"

#define LED_RED (1U << 1)   /* PF1 */

static void delay(volatile uint32_t n) {
    while (n--) __NOP();
}

int main(void) {
    /* Enable clock to GPIOF, wait for it to be ready */
    SYSCTL->RCGCGPIO |= (1U << 5);
    while ((SYSCTL->PRGPIO & (1U << 5)) == 0) {}

    /* PF1 as digital output, 8 mA drive */
    GPIOF->DIR  |= LED_RED;
    GPIOF->DR8R |= LED_RED;
    GPIOF->DEN  |= LED_RED;

    for (;;) {
        GPIOF->DATA ^= LED_RED;
        delay(500000);
    }
}
