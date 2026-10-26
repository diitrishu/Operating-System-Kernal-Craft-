# MyOS - Kernel Craft

A small 32-bit x86 operating system written from scratch in C and assembly. It boots through GRUB (Multiboot), shows a text-mode shell, and has a simple in-memory file system.

## Features

- **Boot:** GRUB loads the kernel in 32-bit protected mode (Multiboot); a short assembly stub sets up the stack and calls the kernel.
- **Display:** VGA text mode, 80x25, white on black.
- **Input:** Keyboard read directly from the PS/2 controller (ports `0x60` / `0x64`, polled).
- **Shell:** `myos>` prompt with a command parser.
- **File system:** in-memory only, up to 100 files, 1024 bytes per file, names up to 31 characters.

## Commands

| Command | Description |
| --- | --- |
| `help` | List available commands |
| `create <filename>` | Create an empty file |
| `write <filename> <content>` | Write content to an existing file |
| `read <filename>` | Print a file's content |
| `ls` | List files and their sizes |

```
myos> create test.txt
myos> write test.txt Hello World!
myos> read test.txt
File contents: Hello World!
```

## Project structure

```
.
├── boot/
│   └── entry.S             # Entry point: sets up the stack, calls kernel_init
├── kernel/
│   ├── kernel.c            # Kernel: Multiboot header, VGA, keyboard, shell, file system
│   └── linker.ld           # Linker script (kernel at 1 MiB)
├── iso/
│   └── boot/grub/grub.cfg  # GRUB menu entry
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
2. GRUB finds the Multiboot header (the first thing in the kernel image) and loads the kernel at 1 MiB (`0x100000`) in 32-bit protected mode.
3. `_start` in `boot/entry.S` sets `ESP` to a 16 KiB stack in `.bss` and calls `kernel_init()`.
4. `kernel_init()` clears the screen, initialises the file system and loops, polling the keyboard.

## Memory layout

| Region | Address |
| --- | --- |
| Kernel image | `0x100000` (1 MiB) |
| VGA text buffer | `0xB8000` |
| Kernel stack | 16 KiB, in `.bss` |

## Limitations

- Files live in memory and are lost on reboot; no directories or permissions.
- Text mode only; the screen wraps back to the top instead of scrolling.
- Keyboard input is polled; there is no IDT or interrupt handling yet. Only lowercase letters, digits and basic punctuation are supported (no Shift or Caps Lock).
- No memory management, processes or multitasking.

## Future improvements

- Interrupt handling (GDT/IDT, keyboard IRQ)
- Screen scrolling and Shift/Caps Lock support
- Persistent storage and a real file system (FAT32/ext2)
- Memory management and process management
- More shell commands and better error handling

## License

Released under the MIT License. See [LICENSE](LICENSE).
