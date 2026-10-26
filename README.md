# MyOS - Kernel Craft

A small 32-bit x86 operating system written from scratch in C and assembly. It boots through GRUB (Multiboot), sets up its own descriptor tables and interrupt handling, and drops into a colour text-mode shell with an in-memory file system.

**Current version: 0.2.0**

## Features

- **Boot:** GRUB (Multiboot) loads the kernel at 1 MiB; a short assembly stub sets up the stack and passes the Multiboot info to C.
- **CPU setup:** own GDT (flat 4 GiB model) and an IDT covering all 32 CPU exceptions plus 16 hardware IRQs.
- **Interrupts:** remapped 8259 PICs; unhandled CPU exceptions show a kernel panic screen (vector, error code, `eip`, `cs`, `eflags`) and halt.
- **Timer:** PIT at 100 Hz, used for `uptime`.
- **Keyboard:** interrupt-driven PS/2 driver with a ring buffer, Shift, Caps Lock and Ctrl (`Ctrl+L` clears the screen, `Ctrl+C` cancels the current line).
- **Display:** VGA text mode (80x25) with colours, hardware cursor, backspace and **scrolling**.
- **Memory info:** reads the RAM size reported by the bootloader.
- **File system:** in-memory, up to 100 files, 1024 bytes each, names up to 31 characters.
- **Shell:** `myos>` prompt with a command table, usage messages and colour-coded errors.

## Commands

| Command | Description |
| --- | --- |
| `help` | List available commands |
| `ls` | List files and their sizes |
| `create <file>` | Create an empty file |
| `write <file> <text>` | Write text to a file (creates it if it does not exist) |
| `append <file> <text>` | Append text to a file |
| `read <file>` (or `cat`) | Print a file |
| `rm <file>` | Delete a file |
| `echo <text>` | Print text |
| `clear` | Clear the screen (`Ctrl+L`) |
| `uptime` | Time since boot |
| `mem` | Memory size and file system usage |
| `ver` | Show the OS version |
| `reboot` | Restart the machine |
| `halt` | Stop the CPU |

```
myos> write notes.txt Hello, World!
Wrote notes.txt
myos> append notes.txt  More text
Appended to notes.txt
myos> read notes.txt
Hello, World!More text
myos> uptime
Uptime: 0h 1m 12s
```

## Project structure

```
.
├── boot/
│   ├── entry.S             # Multiboot header, stack setup, _start
│   └── interrupts.S        # ISR/IRQ stubs, common handler, GDT reload
├── kernel/
│   ├── kernel.c            # kernel_main: initialisation order
│   ├── gdt.c / gdt.h       # Global Descriptor Table
│   ├── idt.c / idt.h       # IDT, PIC remap, exception panic, handler registry
│   ├── timer.c / timer.h   # PIT timer (100 Hz tick counter)
│   ├── keyboard.c / .h     # Interrupt-driven keyboard driver
│   ├── vga.c / vga.h       # Text-mode display: colours, cursor, scrolling
│   ├── fs.c / fs.h         # In-memory file system
│   ├── shell.c / shell.h   # Command-line shell
│   ├── string.c / string.h # strlen, strcmp, memcpy, memset, ...
│   ├── io.h                # Port I/O and CPU helpers
│   ├── types.h             # Fixed-size integer types
│   ├── version.h           # OS version string
│   └── linker.ld           # Linker script (kernel at 1 MiB)
├── iso/boot/grub/grub.cfg  # GRUB menu entry
├── Makefile
├── LICENSE
└── README.md
```

## Requirements

Linux or WSL (Ubuntu) on an x86/x86_64 machine.

```bash
sudo apt update
sudo apt install build-essential gcc-multilib binutils grub-pc-bin xorriso mtools qemu-system-x86
```

## Build and run

```bash
make          # builds iso/boot/kernel.bin and myos.iso
make run      # boots myos.iso in QEMU
make clean    # removes build output
```

You can also start it manually with `qemu-system-i386 -cdrom myos.iso`.

## How it boots

1. BIOS starts GRUB from the ISO.
2. GRUB finds the Multiboot header (first thing in the kernel image) and loads the kernel at 1 MiB in 32-bit protected mode, passing a magic number and a pointer to the boot information.
3. `_start` (`boot/entry.S`) sets up a 16 KiB stack and calls `kernel_main(magic, info)`.
4. `kernel_main` initialises the GDT, IDT/PIC, timer, keyboard and file system, enables interrupts, and starts the shell.
5. The shell sleeps with `hlt` between keystrokes and is woken by the keyboard interrupt.

## Memory layout

| Region | Address |
| --- | --- |
| Kernel image | `0x100000` (1 MiB) |
| VGA text buffer | `0xB8000` |
| Kernel stack | 16 KiB, in `.bss` |

## Limitations

- Files live in memory and are lost on reboot; no directories or permissions.
- Shell input lines are limited to 255 characters.
- Keyboard: US layout only; arrow keys, command history and tab completion are not implemented.
- No memory management (no `malloc`/paging), no processes or multitasking, no user mode.
- Text mode only; no mouse, serial or disk drivers.

## Roadmap

- Physical memory manager and a kernel heap (`kmalloc`)
- Paging and virtual memory
- Command history and arrow-key editing
- Persistent storage: ATA/IDE driver and a simple on-disk file system (FAT or ext2)
- Processes, a scheduler and user mode with system calls
- Serial port output for logging and easier debugging

## License

Released under the MIT License. See [LICENSE](LICENSE).
