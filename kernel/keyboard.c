#include "keyboard.h"
#include "idt.h"
#include "io.h"

#define KB_DATA_PORT   0x60
#define KB_STATUS_PORT 0x64
#define KB_BUFFER_SIZE 256

/* Scancode set 1, indexed by scancode (make codes 0x00-0x39). */
static const char normal_map[0x3A] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char shift_map[0x3A] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

static volatile char buffer[KB_BUFFER_SIZE];
static volatile uint32_t head, tail;
static int shift, caps, ctrl, extended;

static void push(char c) {
    uint32_t next = (head + 1) % KB_BUFFER_SIZE;
    if (next != tail) {               /* drop input if the buffer is full */
        buffer[head] = c;
        head = next;
    }
}

static void keyboard_irq(struct regs *r) {
    (void)r;
    uint8_t sc = inb(KB_DATA_PORT);

    if (sc == 0xE0) { extended = 1; return; }   /* arrows, etc.: not handled yet */
    if (extended)   { extended = 0; return; }

    int release  = sc & 0x80;
    uint8_t code = sc & 0x7F;

    if (code == 0x2A || code == 0x36) { shift = !release; return; }
    if (code == 0x1D)                 { ctrl  = !release; return; }
    if (release) return;
    if (code == 0x3A)                 { caps = !caps; return; }
    if (code >= sizeof(normal_map)) return;

    char base = normal_map[code];
    if (!base) return;

    if (ctrl) {
        if (base == 'l') push('\f');
        else if (base == 'c') push(3);
        return;
    }

    char c;
    if (base >= 'a' && base <= 'z')
        c = (shift != caps) ? (char)(base - 32) : base;
    else
        c = shift ? shift_map[code] : base;
    push(c);
}

void keyboard_init(void) {
    while (inb(KB_STATUS_PORT) & 1) inb(KB_DATA_PORT);   /* flush stale bytes */
    register_interrupt_handler(IRQ_BASE + 1, keyboard_irq);
}

char keyboard_getchar(void) {
    while (head == tail) cpu_halt();      /* sleep until the next interrupt */
    char c = buffer[tail];
    tail = (tail + 1) % KB_BUFFER_SIZE;
    return c;
}
