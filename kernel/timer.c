#include "timer.h"
#include "idt.h"
#include "io.h"

static volatile uint32_t ticks;

static void timer_irq(struct regs *r) {
    (void)r;
    ticks++;
}

void timer_init(void) {
    register_interrupt_handler(IRQ_BASE + 0, timer_irq);

    /* Program PIT channel 0 as a square-wave generator at TIMER_HZ. */
    uint32_t divisor = 1193182 / TIMER_HZ;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

uint32_t timer_ticks(void) {
    return ticks;
}
