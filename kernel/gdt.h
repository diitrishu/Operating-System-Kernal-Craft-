#ifndef GDT_H
#define GDT_H

/* Flat 4 GiB memory model: null, kernel code (0x08), kernel data (0x10). */
void gdt_init(void);

#endif
