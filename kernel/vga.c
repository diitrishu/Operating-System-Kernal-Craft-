#include "vga.h"
#include "io.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

static volatile uint16_t *const vga = (volatile uint16_t *)VGA_MEMORY;
static int cursor_x, cursor_y;
static uint8_t attr = (VGA_BLACK << 4) | VGA_WHITE;

static uint16_t cell(char c) {
    return ((uint16_t)attr << 8) | (uint8_t)c;
}

static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F); outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E); outb(0x3D5, (pos >> 8) & 0xFF);
}

static void scroll(void) {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
        vga[i] = vga[i + VGA_WIDTH];
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
        vga[i] = cell(' ');
    cursor_y = VGA_HEIGHT - 1;
}

static void put_cell(char c) {
    vga[cursor_y * VGA_WIDTH + cursor_x] = cell(c);
    if (++cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
}

void vga_init(void) {
    /* Show a block-ish underline cursor (scanlines 13-15). */
    outb(0x3D4, 0x0A); outb(0x3D5, (inb(0x3D5) & 0xC0) | 13);
    outb(0x3D4, 0x0B); outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    attr = (bg << 4) | fg;
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = cell(' ');
    cursor_x = cursor_y = 0;
    update_cursor();
}

void vga_putc(char c) {
    switch (c) {
    case '\n': cursor_x = 0; cursor_y++; break;
    case '\r': cursor_x = 0; break;
    case '\b':
        if (cursor_x > 0) cursor_x--;
        else if (cursor_y > 0) { cursor_y--; cursor_x = VGA_WIDTH - 1; }
        vga[cursor_y * VGA_WIDTH + cursor_x] = cell(' ');
        break;
    case '\t': {
        int n = 4 - (cursor_x % 4);
        while (n--) put_cell(' ');
        break;
    }
    default: put_cell(c); break;
    }
    if (cursor_y >= VGA_HEIGHT) scroll();
    update_cursor();
}

void vga_puts(const char *s) {
    while (*s) vga_putc(*s++);
}

void vga_put_uint(uint32_t v) {
    char buf[11];
    int i = 10;
    buf[i] = '\0';
    if (v == 0) buf[--i] = '0';
    while (v) { buf[--i] = '0' + (v % 10); v /= 10; }
    vga_puts(&buf[i]);
}

void vga_put_hex(uint32_t v) {
    static const char digits[] = "0123456789ABCDEF";
    vga_puts("0x");
    for (int shift = 28; shift >= 0; shift -= 4)
        vga_putc(digits[(v >> shift) & 0xF]);
}
