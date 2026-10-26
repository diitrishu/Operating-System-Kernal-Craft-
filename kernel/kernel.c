#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_FLAGS 0x0
#define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

// Multiboot header
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_MAGIC,
    MULTIBOOT_FLAGS,
    MULTIBOOT_CHECKSUM
};

// Define our own types
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;

// Constants
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define MAX_FILES 100
#define MAX_FILENAME 32
#define MAX_FILE_SIZE 1024
#define MAX_COMMAND_LENGTH 256

// VGA color attributes
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color_t;

// File system structures
typedef struct {
    char name[MAX_FILENAME];
    uint8_t data[MAX_FILE_SIZE];
    uint32_t size;
    int is_used;
} file_t;

// Forward declarations
static inline uint8_t inb(uint16_t port);
static inline void outb(uint16_t port, uint8_t val);
static char scancode_to_ascii(uint8_t scancode);

// Global variables
static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static file_t files[MAX_FILES];
static char command_buffer[MAX_COMMAND_LENGTH];
static int command_length = 0;

// Function prototypes
void clear_screen(void);
void print_char(char c);
void print_string(const char* str);
void handle_keyboard(void);
void init_file_system(void);
int create_file(const char* name);
int write_file(const char* name, const char* data, uint32_t size);
int read_file(const char* name, char* buffer, uint32_t* size);
void process_command(void);
void show_prompt(void);
void list_files(void);

// Initialize the kernel
void kernel_init(void) {
    clear_screen();
    print_string("MyOS Kernel Initialized\n");
    init_file_system();
    show_prompt();
    
    // Main kernel loop
    while (1) {
        handle_keyboard();
    }
}

// Show command prompt
void show_prompt(void) {
    print_string("myos> ");
}

// Process command
void process_command(void) {
    command_buffer[command_length] = '\0';
    
    // Skip if empty command
    if (command_length == 0) {
        show_prompt();
        return;
    }

    // Parse command
    if (command_buffer[0] == 'c' && command_buffer[1] == 'r' && 
        command_buffer[2] == 'e' && command_buffer[3] == 'a' &&
        command_buffer[4] == 't' && command_buffer[5] == 'e' &&
        command_buffer[6] == ' ') {
        // create command
        char* filename = &command_buffer[7];
        if (create_file(filename)) {
            print_string("File created: ");
            print_string(filename);
            print_string("\n");
        } else {
            print_string("Error: Could not create file\n");
        }
    }
    else if (command_buffer[0] == 'l' && command_buffer[1] == 's' &&
             command_length == 2) {
        // ls command
        list_files();
    }
    else if (command_buffer[0] == 'w' && command_buffer[1] == 'r' &&
             command_buffer[2] == 'i' && command_buffer[3] == 't' &&
             command_buffer[4] == 'e' && command_buffer[5] == ' ') {
        // write command
        char* space = &command_buffer[6];
        while (*space && *space != ' ') space++;
        if (*space == ' ') {
            *space = '\0';
            char* filename = &command_buffer[6];
            char* data = space + 1;
            if (write_file(filename, data, command_length - (data - command_buffer))) {
                print_string("Data written to file: ");
                print_string(filename);
                print_string("\n");
            } else {
                print_string("Error: Could not write to file\n");
            }
        }
    }
    else if (command_buffer[0] == 'r' && command_buffer[1] == 'e' &&
             command_buffer[2] == 'a' && command_buffer[3] == 'd' &&
             command_buffer[4] == ' ') {
        // read command
        char* filename = &command_buffer[5];
        char buffer[MAX_FILE_SIZE + 1];
        uint32_t size;
        if (read_file(filename, buffer, &size)) {
            buffer[size] = '\0';
            print_string("File contents: ");
            print_string(buffer);
            print_string("\n");
        } else {
            print_string("Error: Could not read file\n");
        }
    }
    else if (command_buffer[0] == 'h' && command_buffer[1] == 'e' &&
             command_buffer[2] == 'l' && command_buffer[3] == 'p' &&
             command_length == 4) {
        // help command
        print_string("Available commands:\n");
        print_string("  create <filename> - Create a new file\n");
        print_string("  write <filename> <content> - Write content to a file\n");
        print_string("  read <filename> - Read content from a file\n");
        print_string("  ls - List all files\n");
        print_string("  help - Show this help message\n");
    }
    else {
        print_string("Unknown command. Type 'help' for available commands.\n");
    }

    // Reset command buffer
    command_length = 0;
    show_prompt();
}

// List all files
void list_files(void) {
    int found = 0;
    print_string("Files:\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used) {
            print_string("  ");
            print_string(files[i].name);
            print_string(" (");
            // Convert size to string
            char size_str[10];
            int size = files[i].size;
            int idx = 0;
            if (size == 0) {
                size_str[idx++] = '0';
            } else {
                while (size > 0) {
                    size_str[idx++] = '0' + (size % 10);
                    size /= 10;
                }
            }
            size_str[idx] = '\0';
            // Print size in reverse (since we built it backwards)
            for (int j = idx - 1; j >= 0; j--) {
                print_char(size_str[j]);
            }
            print_string(" bytes)\n");
            found = 1;
        }
    }
    if (!found) {
        print_string("  No files found\n");
    }
}

// Clear the screen
void clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (VGA_COLOR_BLACK << 12) | (VGA_COLOR_WHITE << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

// Print a character to the screen
void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = 0;
        }
        return;
    }

    vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (VGA_COLOR_BLACK << 12) | (VGA_COLOR_WHITE << 8) | c;
    cursor_x++;
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = 0;
        }
    }
}

// Print a string to the screen
void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

// Handle keyboard input
void handle_keyboard(void) {
    if (inb(KEYBOARD_STATUS_PORT) & 1) {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);
        char c = scancode_to_ascii(scancode);
        if (c) {
            if (c == '\n') {
                print_char('\n');
                process_command();
            } else if (c == '\b' && command_length > 0) {
                command_length--;
                if (cursor_x > 0) {
                    cursor_x--;
                } else if (cursor_y > 0) {
                    cursor_x = VGA_WIDTH - 1;
                    cursor_y--;
                }
                vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (VGA_COLOR_BLACK << 12) | (VGA_COLOR_WHITE << 8) | ' ';
            } else if (c != '\b' && command_length < MAX_COMMAND_LENGTH - 1) {
                command_buffer[command_length++] = c;
                print_char(c);
            }
        }
    }
}

// Initialize the file system
void init_file_system(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].is_used = 0;
    }
}

// Create a new file
int create_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].is_used) {
            int j;
            for (j = 0; name[j] && j < MAX_FILENAME - 1; j++) {
                files[i].name[j] = name[j];
            }
            files[i].name[j] = '\0';
            files[i].size = 0;
            files[i].is_used = 1;
            return 1;
        }
    }
    return 0;
}

// Write data to a file
int write_file(const char* name, const char* data, uint32_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used) {
            int j;
            for (j = 0; name[j] && files[i].name[j] && name[j] == files[i].name[j]; j++);
            if (!name[j] && !files[i].name[j]) {
                if (size > MAX_FILE_SIZE) return 0;
                for (uint32_t k = 0; k < size; k++) {
                    files[i].data[k] = data[k];
                }
                files[i].size = size;
                return 1;
            }
        }
    }
    return 0;
}

// Read data from a file
int read_file(const char* name, char* buffer, uint32_t* size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used) {
            int j;
            for (j = 0; name[j] && files[i].name[j] && name[j] == files[i].name[j]; j++);
            if (!name[j] && !files[i].name[j]) {
                for (uint32_t k = 0; k < files[i].size; k++) {
                    buffer[k] = files[i].data[k];
                }
                *size = files[i].size;
                return 1;
            }
        }
    }
    return 0;
}

// Helper functions
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Simple scancode to ASCII conversion
static char scancode_to_ascii(uint8_t scancode) {
    static const char scancode_table[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_table)) {
        return scancode_table[scancode];
    }
    return 0;
}
