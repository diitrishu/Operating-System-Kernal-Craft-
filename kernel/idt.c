#include "idt.h"
#include "io.h"
#include "string.h"
#include "vga.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define IDT_SIZE   256
#define NUM_STUBS  48          /* 32 exceptions + 16 IRQs */

static struct idt_entry idt[IDT_SIZE];
static struct idt_ptr   idtp;
static interrupt_handler_t handlers[IDT_SIZE];

extern uint32_t isr_stub_table[NUM_STUBS];     /* boot/interrupts.S */

static const char *exception_names[32] = {
    "Divide error", "Debug", "Non-maskable interrupt", "Breakpoint",
    "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available",
    "Double fault", "Coprocessor segment overrun", "Invalid TSS", "Segment not present",
    "Stack-segment fault", "General protection fault", "Page fault", "Reserved",
    "x87 floating-point error", "Alignment check", "Machine check", "SIMD floating-point error",
    "Virtualization error", "Control protection error", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Security exception", "Reserved"
};

static void idt_set_gate(int n, uint32_t base) {
    idt[n].base_low  = base & 0xFFFF;
    idt[n].base_high = (base >> 16) & 0xFFFF;
    idt[n].selector  = 0x08;      /* kernel code segment */
    idt[n].zero      = 0;
    idt[n].flags     = 0x8E;      /* present, ring 0, 32-bit interrupt gate */
}

/* Remap the 8259 PICs so IRQs 0-15 land on vectors 32-47 (not on CPU exceptions). */
static void pic_remap(void) {
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, IRQ_BASE);     io_wait();
    outb(0xA1, IRQ_BASE + 8); io_wait();
    outb(0x21, 0x04); io_wait();      /* master: slave on IRQ2 */
    outb(0xA1, 0x02); io_wait();      /* slave: cascade identity 2 */
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();
    outb(0x21, 0xFC);                 /* unmask only IRQ0 (timer) and IRQ1 (keyboard) */
    outb(0xA1, 0xFF);
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    for (int i = 0; i < NUM_STUBS; i++)
        idt_set_gate(i, isr_stub_table[i]);

    pic_remap();

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}

void register_interrupt_handler(int vector, interrupt_handler_t handler) {
    handlers[vector] = handler;
}

static void exception_panic(struct regs *r) {
    vga_set_color(VGA_WHITE, VGA_RED);
    vga_puts("\n*** KERNEL PANIC: ");
    vga_puts(exception_names[r->int_no]);
    vga_puts(" ***");
    vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
    vga_puts("\n  vector=");  vga_put_uint(r->int_no);
    vga_puts("  error=");     vga_put_hex(r->err_code);
    vga_puts("\n  eip=");     vga_put_hex(r->eip);
    vga_puts("  cs=");        vga_put_hex(r->cs);
    vga_puts("  eflags=");    vga_put_hex(r->eflags);
    vga_puts("\nSystem halted.\n");
    for (;;) { irq_disable(); cpu_halt(); }
}

/* Called from isr_common for every exception and IRQ. */
void interrupt_handler(struct regs *r) {
    if (handlers[r->int_no])
        handlers[r->int_no](r);
    else if (r->int_no < 32)
        exception_panic(r);

    if (r->int_no >= IRQ_BASE && r->int_no < IRQ_BASE + 16) {
        if (r->int_no >= IRQ_BASE + 8)
            outb(0xA0, 0x20);         /* EOI to the slave PIC */
        outb(0x20, 0x20);             /* EOI to the master PIC */
    }
}
