/* Kernel entry point.
 *
 * GRUB (Multiboot) loads the kernel in 32-bit protected mode but leaves ESP
 * undefined, so we set up our own stack before jumping into C. */

.section .bss
.align 16
stack_bottom:
    .skip 16384             /* 16 KiB kernel stack */
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    cli
    mov $stack_top, %esp
    xor %ebp, %ebp
    call kernel_init
.hang:
    hlt
    jmp .hang
