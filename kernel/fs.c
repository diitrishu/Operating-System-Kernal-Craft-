#include "fs.h"
#include "string.h"

static file_t files[FS_MAX_FILES];

void fs_init(void) {
    memset(files, 0, sizeof(files));
}

static file_t *find(const char *name) {
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (files[i].used && strcmp(files[i].name, name) == 0)
            return &files[i];
    return NULL;
}

int fs_create(const char *name) {
    size_t len = strlen(name);
    if (len == 0 || len >= FS_MAX_NAME) return FS_ERR_NAME;
    if (find(name)) return FS_ERR_EXISTS;

    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!files[i].used) {
            strlcpy(files[i].name, name, FS_MAX_NAME);
            files[i].size = 0;
            files[i].used = 1;
            return FS_OK;
        }
    }
    return FS_ERR_FULL;
}

int fs_write(const char *name, const char *data, uint32_t len, int append) {
    file_t *f = find(name);
    if (!f) return FS_ERR_NOENT;

    uint32_t offset = append ? f->size : 0;
    if (offset + len > FS_MAX_SIZE) return FS_ERR_SIZE;

    memcpy(&f->data[offset], data, len);
    f->size = offset + len;
    return FS_OK;
}

int fs_read(const char *name, const uint8_t **data, uint32_t *size) {
    file_t *f = find(name);
    if (!f) return FS_ERR_NOENT;
    *data = f->data;
    *size = f->size;
    return FS_OK;
}

int fs_delete(const char *name) {
    file_t *f = find(name);
    if (!f) return FS_ERR_NOENT;
    f->used = 0;
    f->size = 0;
    return FS_OK;
}

int fs_count(void) {
    int n = 0;
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (files[i].used) n++;
    return n;
}

const file_t *fs_entry(int i) {
    if (i < 0 || i >= FS_MAX_FILES || !files[i].used) return NULL;
    return &files[i];
}
