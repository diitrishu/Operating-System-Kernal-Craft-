#include "fs.h"
#include "gdt.h"
#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
#include "version.h"
#include "vga.h"

#define MULTIBOOT_BOOT_MAGIC 0x2BADB002
#define MBI_FLAG_MEM         (1 << 0)

/* The start of the Multiboot information structure GRUB hands us. */
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;      /* KiB below 1 MiB */
    uint32_t mem_upper;      /* KiB above 1 MiB */
};

static void status(const char *what) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("[ ok ] ");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_puts(what);
    vga_putc('\n');
}

void kernel_main(uint32_t magic, struct multiboot_info *mbi) {
    vga_init();
    vga_clear();

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("MyOS v" MYOS_VERSION " - Kernel Craft\n");
    vga_set_color(VGA_DARK_GREY, VGA_BLACK);
    vga_puts("--------------------------------------\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);

    gdt_init();       status("GDT loaded");
    idt_init();       status("IDT and PIC initialised");
    timer_init();     status("Timer running at 100 Hz");
    keyboard_init();  status("Keyboard driver (interrupt-driven)");
    fs_init();        status("In-memory file system");

    if (magic == MULTIBOOT_BOOT_MAGIC && mbi && (mbi->flags & MBI_FLAG_MEM))
        shell_set_meminfo(mbi->mem_lower, mbi->mem_upper);

    irq_enable();
    vga_puts("\nType 'help' for a list of commands.\n\n");

    shell_run();
}
