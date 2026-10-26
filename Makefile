# MyOS build: GRUB (Multiboot) loads a 32-bit kernel linked at 1 MiB.
CC      = gcc
LD      = ld
CFLAGS  = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nostdinc \
	-fno-builtin -fno-exceptions -fno-leading-underscore -Wall -Wextra
LDFLAGS = -melf_i386 -z noexecstack -T kernel/linker.ld

KERNEL_BIN = iso/boot/kernel.bin
ISO_FILE   = myos.iso
OBJS       = boot/entry.o kernel/kernel.o

all: $(ISO_FILE)

$(ISO_FILE): $(KERNEL_BIN) iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso

$(KERNEL_BIN): $(OBJS) kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

boot/entry.o: boot/entry.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE)

clean:
	rm -f $(OBJS) $(KERNEL_BIN) $(ISO_FILE)

.PHONY: all run clean
