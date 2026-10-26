/* Interrupt entry stubs, GDT reload.
 * Vectors 0-31 are CPU exceptions, 32-47 are the remapped hardware IRQs.
 * Every stub pushes a (real or dummy) error code and its vector number, then
 * jumps to isr_common, which calls interrupt_handler(struct regs *). */

.section .text

.macro ISR n, has_err
isr\n:
    .if \has_err == 0
    pushl $0
    .endif
    pushl $\n
    jmp isr_common
.endm

ISR 0, 0
ISR 1, 0
ISR 2, 0
ISR 3, 0
ISR 4, 0
ISR 5, 0
ISR 6, 0
ISR 7, 0
ISR 8, 1
ISR 9, 0
ISR 10, 1
ISR 11, 1
ISR 12, 1
ISR 13, 1
ISR 14, 1
ISR 15, 0
ISR 16, 0
ISR 17, 1
ISR 18, 0
ISR 19, 0
ISR 20, 0
ISR 21, 1
ISR 22, 0
ISR 23, 0
ISR 24, 0
ISR 25, 0
ISR 26, 0
ISR 27, 0
ISR 28, 0
ISR 29, 1
ISR 30, 1
ISR 31, 0
ISR 32, 0
ISR 33, 0
ISR 34, 0
ISR 35, 0
ISR 36, 0
ISR 37, 0
ISR 38, 0
ISR 39, 0
ISR 40, 0
ISR 41, 0
ISR 42, 0
ISR 43, 0
ISR 44, 0
ISR 45, 0
ISR 46, 0
ISR 47, 0

isr_common:
    pusha
    cld
    pushl %esp                          /* argument: pointer to saved registers */
    call interrupt_handler
    addl $4, %esp
    popa
    addl $8, %esp                       /* drop vector number and error code */
    iret

/* void gdt_flush(struct gdt_ptr *gp): load the GDT and reload all segments. */
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $.flush
.flush:
    ret

.section .data
.global isr_stub_table
isr_stub_table:
    .long isr0
    .long isr1
    .long isr2
    .long isr3
    .long isr4
    .long isr5
    .long isr6
    .long isr7
    .long isr8
    .long isr9
    .long isr10
    .long isr11
    .long isr12
    .long isr13
    .long isr14
    .long isr15
    .long isr16
    .long isr17
    .long isr18
    .long isr19
    .long isr20
    .long isr21
    .long isr22
    .long isr23
    .long isr24
    .long isr25
    .long isr26
    .long isr27
    .long isr28
    .long isr29
    .long isr30
    .long isr31
    .long isr32
    .long isr33
    .long isr34
    .long isr35
    .long isr36
    .long isr37
    .long isr38
    .long isr39
    .long isr40
    .long isr41
    .long isr42
    .long isr43
    .long isr44
    .long isr45
    .long isr46
    .long isr47

.section .note.GNU-stack,"",@progbits
