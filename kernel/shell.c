#include "shell.h"
#include "fs.h"
#include "io.h"
#include "keyboard.h"
#include "string.h"
#include "timer.h"
#include "version.h"
#include "vga.h"

#define LINE_MAX 256

static uint32_t mem_lower_kb, mem_upper_kb;

void shell_set_meminfo(uint32_t lower_kb, uint32_t upper_kb) {
    mem_lower_kb = lower_kb;
    mem_upper_kb = upper_kb;
}

/* ---------- output helpers ---------- */

static void error(const char *msg) {
    vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
    vga_puts(msg);
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_putc('\n');
}

static void usage(const char *text) {
    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_puts("Usage: ");
    vga_puts(text);
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_putc('\n');
}

static void fs_error(int code) {
    switch (code) {
    case FS_ERR_NAME:   error("Error: invalid file name (max 31 characters)"); break;
    case FS_ERR_EXISTS: error("Error: file already exists"); break;
    case FS_ERR_FULL:   error("Error: file system is full"); break;
    case FS_ERR_NOENT:  error("Error: no such file"); break;
    case FS_ERR_SIZE:   error("Error: file would exceed 1024 bytes"); break;
    default:            error("Error: file system error"); break;
    }
}

/* ---------- argument parsing ---------- */

static char *skip_spaces(char *s) {
    while (*s == ' ') s++;
    return s;
}

/* Split the next space-separated word off *p (modifies the buffer). */
static char *next_token(char **p) {
    char *s = skip_spaces(*p);
    if (!*s) { *p = s; return NULL; }
    char *start = s;
    while (*s && *s != ' ') s++;
    if (*s) *s++ = '\0';
    *p = s;
    return start;
}

/* ---------- commands ---------- */

typedef void (*cmd_fn)(char *args);

struct command {
    const char *name;
    const char *usage;      /* NULL hides the command from `help` (aliases) */
    const char *help;
    cmd_fn      fn;
};

static const struct command commands[];

static void cmd_help(char *args) {
    (void)args;
    vga_puts("Available commands:\n");
    for (int i = 0; commands[i].name; i++) {
        if (!commands[i].usage) continue;
        vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
        vga_puts("  ");
        vga_puts(commands[i].usage);
        vga_set_color(VGA_WHITE, VGA_BLACK);
        vga_puts(" - ");
        vga_puts(commands[i].help);
        vga_putc('\n');
    }
}

static void cmd_clear(char *args) {
    (void)args;
    vga_clear();
}

static void cmd_ls(char *args) {
    (void)args;
    int n = fs_count();
    if (n == 0) { vga_puts("No files.\n"); return; }
    for (int i = 0; i < FS_MAX_FILES; i++) {
        const file_t *f = fs_entry(i);
        if (!f) continue;
        vga_puts("  ");
        vga_puts(f->name);
        vga_puts("  (");
        vga_put_uint(f->size);
        vga_puts(" bytes)\n");
    }
    vga_put_uint(n);
    vga_puts(n == 1 ? " file\n" : " files\n");
}

static void cmd_create(char *args) {
    char *name = next_token(&args);
    if (!name) { usage("create <filename>"); return; }
    int r = fs_create(name);
    if (r != FS_OK) { fs_error(r); return; }
    vga_puts("Created ");
    vga_puts(name);
    vga_putc('\n');
}

static void write_common(char *args, int append) {
    char *name = next_token(&args);
    if (!name) { usage(append ? "append <filename> <text>" : "write <filename> <text>"); return; }
    char *text = skip_spaces(args);

    int r = fs_write(name, text, strlen(text), append);
    if (r == FS_ERR_NOENT) {                 /* writing to a new file creates it */
        r = fs_create(name);
        if (r == FS_OK) r = fs_write(name, text, strlen(text), append);
    }
    if (r != FS_OK) { fs_error(r); return; }
    vga_puts(append ? "Appended to " : "Wrote ");
    vga_puts(name);
    vga_putc('\n');
}

static void cmd_write(char *args)  { write_common(args, 0); }
static void cmd_append(char *args) { write_common(args, 1); }

static void cmd_read(char *args) {
    char *name = next_token(&args);
    if (!name) { usage("read <filename>"); return; }
    const uint8_t *data;
    uint32_t size;
    int r = fs_read(name, &data, &size);
    if (r != FS_OK) { fs_error(r); return; }
    for (uint32_t i = 0; i < size; i++) vga_putc((char)data[i]);
    if (size == 0 || data[size - 1] != '\n') vga_putc('\n');
}

static void cmd_rm(char *args) {
    char *name = next_token(&args);
    if (!name) { usage("rm <filename>"); return; }
    int r = fs_delete(name);
    if (r != FS_OK) { fs_error(r); return; }
    vga_puts("Removed ");
    vga_puts(name);
    vga_putc('\n');
}

static void cmd_echo(char *args) {
    vga_puts(skip_spaces(args));
    vga_putc('\n');
}

static void cmd_uptime(char *args) {
    (void)args;
    uint32_t s = timer_ticks() / TIMER_HZ;
    vga_puts("Uptime: ");
    vga_put_uint(s / 3600);        vga_puts("h ");
    vga_put_uint((s / 60) % 60);   vga_puts("m ");
    vga_put_uint(s % 60);          vga_puts("s\n");
}

static void cmd_mem(char *args) {
    (void)args;
    if (mem_lower_kb == 0 && mem_upper_kb == 0) {
        vga_puts("Memory size unknown (bootloader did not report it)\n");
    } else {
        vga_puts("Memory: about ");
        vga_put_uint((mem_upper_kb + 1024) / 1024);
        vga_puts(" MB (lower ");
        vga_put_uint(mem_lower_kb);
        vga_puts(" KB, upper ");
        vga_put_uint(mem_upper_kb);
        vga_puts(" KB)\n");
    }
    vga_puts("Files: ");
    vga_put_uint(fs_count());
    vga_puts(" / ");
    vga_put_uint(FS_MAX_FILES);
    vga_putc('\n');
}

static void cmd_ver(char *args) {
    (void)args;
    vga_puts("MyOS v" MYOS_VERSION " (x86, 32-bit, Multiboot)\n");
}

static void cmd_reboot(char *args) {
    (void)args;
    vga_puts("Rebooting...\n");
    outb(0x64, 0xFE);              /* pulse the CPU reset line via the keyboard controller */
    for (;;) { irq_disable(); cpu_halt(); }
}

static void cmd_halt(char *args) {
    (void)args;
    vga_puts("System halted. You can close the emulator window.\n");
    for (;;) { irq_disable(); cpu_halt(); }
}

static const struct command commands[] = {
    { "help",   "help",                      "show this list",                    cmd_help   },
    { "ls",     "ls",                        "list files",                        cmd_ls     },
    { "create", "create <file>",             "create an empty file",              cmd_create },
    { "write",  "write <file> <text>",       "write text (creates the file)",     cmd_write  },
    { "append", "append <file> <text>",      "append text to a file",             cmd_append },
    { "read",   "read <file>",               "print a file",                      cmd_read   },
    { "cat",    NULL,                        NULL,                                cmd_read   },
    { "rm",     "rm <file>",                 "delete a file",                     cmd_rm     },
    { "echo",   "echo <text>",               "print text",                        cmd_echo   },
    { "clear",  "clear",                     "clear the screen (Ctrl+L)",         cmd_clear  },
    { "uptime", "uptime",                    "time since boot",                   cmd_uptime },
    { "mem",    "mem",                       "memory and file system usage",      cmd_mem    },
    { "ver",    "ver",                       "show the OS version",               cmd_ver    },
    { "reboot", "reboot",                    "restart the machine",               cmd_reboot },
    { "halt",   "halt",                      "stop the CPU",                      cmd_halt   },
    { NULL, NULL, NULL, NULL }
};

static void execute(char *line) {
    char *p = line;
    char *cmd = next_token(&p);
    if (!cmd) return;

    for (int i = 0; commands[i].name; i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            commands[i].fn(p);
            return;
        }
    }
    vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
    vga_puts("Unknown command: ");
    vga_puts(cmd);
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_puts(" (type 'help')\n");
}

static void prompt(void) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("myos> ");
    vga_set_color(VGA_WHITE, VGA_BLACK);
}

void shell_run(void) {
    char line[LINE_MAX];
    int len = 0;

    prompt();
    for (;;) {
        char c = keyboard_getchar();

        if (c == '\n') {
            vga_putc('\n');
            line[len] = '\0';
            execute(line);
            len = 0;
            prompt();
        } else if (c == '\b') {
            if (len > 0) { len--; vga_putc('\b'); }
        } else if (c == 3) {                 /* Ctrl+C: abandon the current line */
            vga_puts("^C\n");
            len = 0;
            prompt();
        } else if (c == '\f') {              /* Ctrl+L: clear screen */
            vga_clear();
            len = 0;
            prompt();
        } else if (c >= 32 && c < 127) {
            if (len < LINE_MAX - 1) { line[len++] = c; vga_putc(c); }
        }
    }
}
