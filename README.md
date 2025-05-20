# MyOS - A Simple Operating System

A basic operating system implementation with keyboard input and file handling capabilities.

## Requirements

### Development Tools
1. **NASM (Netwide Assembler)**
   - Version: 2.16.03 or later
   - Purpose: Assembly code compilation for bootloader
   - Why: NASM provides simple syntax and good support for x86 architecture

2. **LLVM/Clang**
   - Version: 20.1.2 or later
   - Purpose: C code compilation for kernel
   - Why: Modern compiler with good cross-compilation support and clean error messages

3. **LLD (LLVM Linker)**
   - Version: 20.1.2 or later
   - Purpose: Linking kernel components
   - Why: Compatible with LLVM/Clang and supports various output formats

4. **GRUB (Grand Unified Bootloader)**
   - Version: 2.0 or later
   - Purpose: Bootloader for loading kernel
   - Why: Industry standard, supports multiboot specification

5. **QEMU**
   - Version: Latest
   - Purpose: OS emulation and testing
   - Why: Provides good debugging capabilities and x86 emulation

### System Requirements
- Linux/Unix environment (WSL recommended for Windows users)
- At least 1GB RAM for development
- x86/x86_64 processor

## Installation (Ubuntu/WSL)

```bash
# Update package list
sudo apt update

# Install required packages
sudo apt install build-essential nasm qemu-system-x86 grub-pc xorriso mtools gcc-multilib clang llvm lld
```

## Project Structure

```
myos/
├── boot/
│   └── boot.asm       # Bootloader code
├── kernel/
│   ├── kernel.c       # Kernel main code
│   └── linker.ld      # Linker script
├── iso/
│   └── boot/
│       └── grub/
│           └── grub.cfg    # GRUB configuration
└── Makefile           # Build automation
```

## Features

1. **Bootloader**
   - 16-bit real mode initialization
   - Protected mode switching
   - Kernel loading

2. **Kernel**
   - Text mode display (80x25 characters)
   - Keyboard input handling
   - Basic in-memory file system
   - Command-line interface

3. **File System**
   - In-memory storage (non-persistent)
   - Maximum 100 files
   - Maximum file size: 1024 bytes
   - Basic file operations (create, read, write)

## Limitations

1. **File System**
   - No persistent storage (files are lost on reboot)
   - Limited file size (1024 bytes)
   - No directories/folders
   - No file permissions

2. **Display**
   - Text-mode only (no graphics)
   - Limited to 80x25 characters
   - Basic white-on-black color scheme

3. **Input**
   - Basic keyboard support
   - No mouse support
   - Limited special key support

## Building and Running

1. **Build the OS**
```bash
make clean
make
```

2. **Run in QEMU**
```bash
qemu-system-i386 -cdrom myos.iso
```

## Available Commands

1. `help` - Display available commands
2. `create <filename>` - Create a new file
   ```
   myos> create test.txt
   ```

3. `write <filename> <content>` - Write content to a file
   ```
   myos> write test.txt Hello World!
   ```

4. `read <filename>` - Read content from a file
   ```
   myos> read test.txt
   ```

5. `ls` - List all files
   ```
   myos> ls
   ```

## Technical Details

1. **Memory Layout**
   - Kernel loaded at 1MB (0x100000)
   - VGA buffer at 0xB8000
   - Stack at 0x7C00

2. **Hardware Communication**
   - Direct port I/O for keyboard
   - VGA text mode for display
   - Uses x86 protected mode

## Future Improvements

1. Add persistent storage support
2. Implement proper file system (FAT32/ext2)
3. Add process management
4. Implement memory management
5. Add color support
6. Add more shell commands
7. Implement proper error handling

## Contributing

Feel free to contribute to this project by:
1. Reporting bugs
2. Suggesting new features
3. Submitting pull requests

## License

This project is open source and available under the MIT License. 