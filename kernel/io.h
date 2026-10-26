#ifndef IO_H
#define IO_H

#include "types.h"

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Short delay for slow legacy hardware (writes to an unused port). */
static inline void io_wait(void) { outb(0x80, 0); }

static inline void irq_enable(void)  { __asm__ volatile ("sti"); }
static inline void irq_disable(void) { __asm__ volatile ("cli"); }
static inline void cpu_halt(void)    { __asm__ volatile ("hlt"); }

#endif
