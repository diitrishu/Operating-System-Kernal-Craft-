#ifndef FS_H
#define FS_H

#include "types.h"

#define FS_MAX_FILES 100
#define FS_MAX_NAME  32       /* including the terminating NUL */
#define FS_MAX_SIZE  1024

typedef struct {
    char     name[FS_MAX_NAME];
    uint8_t  data[FS_MAX_SIZE];
    uint32_t size;
    int      used;
} file_t;

/* Error codes returned by the fs_* functions. */
#define FS_OK         0
#define FS_ERR_NAME  -1       /* empty or too long */
#define FS_ERR_EXISTS -2
#define FS_ERR_FULL  -3       /* no free file slot */
#define FS_ERR_NOENT -4       /* no such file */
#define FS_ERR_SIZE  -5       /* would exceed FS_MAX_SIZE */

void   fs_init(void);
int    fs_create(const char *name);
int    fs_write(const char *name, const char *data, uint32_t len, int append);
int    fs_read(const char *name, const uint8_t **data, uint32_t *size);
int    fs_delete(const char *name);
int    fs_count(void);
/* Slot i (0..FS_MAX_FILES-1), or NULL when the slot is unused. */
const file_t *fs_entry(int i);

#endif
