#ifndef IDT_H
#define IDT_H

#include "types.h"

/* Register state saved by isr_common (boot/interrupts.S). */
struct regs {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;   /* pusha */
    uint32_t int_no, err_code;                         /* pushed by the stub */
    uint32_t eip, cs, eflags;                          /* pushed by the CPU */
};

typedef void (*interrupt_handler_t)(struct regs *r);

#define IRQ_BASE 32      /* hardware IRQ n is delivered as vector 32 + n */

void idt_init(void);
void register_interrupt_handler(int vector, interrupt_handler_t handler);

#endif
