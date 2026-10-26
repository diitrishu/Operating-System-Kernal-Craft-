/* Kernel entry point and Multiboot header.
 *
 * GRUB loads the kernel in 32-bit protected mode with:
 *   EAX = 0x2BADB002 (Multiboot magic), EBX = pointer to the Multiboot info.
 * ESP is undefined, so we set up our own stack before calling into C. */

.set MB_MAGIC,    0x1BADB002
.set MB_FLAGS,    0x00000003            /* bit 0: page-align modules, bit 1: memory info */
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)

.section .multiboot, "a"
.align 4
    .long MB_MAGIC
    .long MB_FLAGS
    .long MB_CHECKSUM

.section .bss
.align 16
stack_bottom:
    .skip 16384                         /* 16 KiB kernel stack */
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    cli
    mov $stack_top, %esp
    xor %ebp, %ebp
    push %ebx                           /* arg 2: multiboot info pointer */
    push %eax                           /* arg 1: multiboot magic */
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

.section .note.GNU-stack,"",@progbits
