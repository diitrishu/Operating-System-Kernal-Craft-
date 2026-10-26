# MyOS build: GRUB (Multiboot) loads a 32-bit kernel linked at 1 MiB.
CC      = gcc
LD      = ld
CFLAGS  = -m32 -march=i686 -mno-sse -mno-mmx -O2 -Wall -Wextra \
	-ffreestanding -fno-builtin -fno-pie -fno-stack-protector \
	-fno-tree-loop-distribute-patterns -fno-asynchronous-unwind-tables \
	-fcf-protection=none -nostdlib -nostdinc -Ikernel -MMD -MP
LDFLAGS = -melf_i386 -z noexecstack -T kernel/linker.ld

KERNEL_BIN = iso/boot/kernel.bin
ISO_FILE   = myos.iso

SRCS_S = $(sort $(wildcard boot/*.S))
SRCS_C = $(sort $(wildcard kernel/*.c))
OBJS   = $(SRCS_S:.S=.o) $(SRCS_C:.c=.o)

all: $(ISO_FILE)

$(ISO_FILE): $(KERNEL_BIN) iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso

$(KERNEL_BIN): $(OBJS) kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(KERNEL_BIN) $(ISO_FILE)

-include $(OBJS:.o=.d)

.PHONY: all run clean
